#include "tds-client.h"

using namespace tds;

int err_handler_tdsclient(DBPROCESS* dbproc, int severity, int dberr, int oserr, char* dberrstr, char* oserrstr) {
  if ((dbproc == NULL) || (DBDEAD(dbproc))) {
    string err = "dbproc is NULL error: ";
    err.append(dberrstr);
    
    throw tds::TDSException(err.c_str());
    return(INT_CANCEL);
  }
  else {
    cerr << "DB-Library error: {} {}" << dberr << " " << dberrstr << endl;

    if (oserr != DBNOERR) {
      cerr << "Operating-system error: {}" << oserrstr << endl;
    }
    // DO NOT CALL dbclose(dbproc)! or dbexit(); tds needs to clean itself up!
    return(INT_CANCEL);
  }
}

int msg_handler_tdsclient(DBPROCESS* dbproc, DBINT msgno, int msgstate, int severity, char* msgtext, char* srvname, char* procname, int line) {
  /*
  ** If it's a database change message, we'll ignore it.
  ** Also ignore language change message.
  */
  if (msgno == 5701 || msgno == 5703)
    return(0);

  cout << "msgno: {} severity: {} msgstate: {}" << msgno << severity << msgstate << endl;

  return(0);
}

TDSException::TDSException(char const* const message) throw() : std::runtime_error(message) {}

int tds::TDSClient::connect(const string& _host, const string& _user, const string& _pass){
  host = _host;
  user = _user;
  pass = _pass;
  return connect();
};

int tds::TDSClient::init() {
  if (dbinit() == FAIL) {
    cerr << "dbinit() failed" << endl;
    return 1;
  }
  return 0;
};

int tds::TDSClient::connect() {
  if (dbinit() == FAIL) {
    cerr << "dbinit() failed" << endl;
    return 1;
  }

  //handle server/network errors
  dberrhandle((EHANDLEFUNC)err_handler_tdsclient);

  dbmsghandle((MHANDLEFUNC)msg_handler_tdsclient);
  
  // Get a LOGINREC.
  if ((login = dblogin()) == NULL) {
    cerr << "connect() unable to allocate login structure" << endl;
    return 1;
  }

  DBSETLUSER(login, user.c_str());
  DBSETLPWD(login, pass.c_str());
  
  dbsetlname(login, "UTF-8", DBSETCHARSET);
  
  // UTF16
  dbsetlbool(login, 1, DBSETUTF16);
  
  dbsetlversion(login, DBVERSION_74);

  // Connect to server
  if ((dbproc = dbopen(login, host.c_str())) == NULL) {
    string err = "connect() unable to connect to " + host;
    throw tds::TDSException(err.c_str());
    return 1;
  }

  return 0;
};

int tds::TDSClient::useDatabase(const string& db){
  if ((erc = dbuse(dbproc, db.c_str())) == FAIL) {
    cerr << "useDatabase() unable to use database {}" << db << endl;
    return 1;
  }
  return 0;
};

void tds::TDSClient::sql(const string& _script){
  script = _script;
  dbcmd(dbproc, script.c_str());
};

int tds::TDSClient::getMetadata() {
  ncols = dbnumcols(dbproc);
  if ((columns = (COL*)calloc(ncols, sizeof(struct COL))) == NULL) {
    perror(NULL);
    return 1;
  }

  /*
  * Read metadata and bind.
  */
  for (pcol = columns; pcol - columns < ncols; pcol++) {
    int c = pcol - columns + 1;

    pcol->name = dbcolname(dbproc, c);
    pcol->type = dbcoltype(dbproc, c); //xml 241, 
    pcol->size = dbcollen(dbproc, c);
    if (pcol->size == __INT_MAX__) {
      // About 1 MB.
      pcol->size = TINYINT_MAX * 32;
    } else {
      pcol->size = pcol->size > 255 ? pcol->size * 2 : 255;
    }

    fieldNames.push_back(move(string(pcol->name)));

    if ((pcol->buffer = (char*)calloc(1, pcol->size + 1)) == NULL){
      cerr << "Cannot allocate buffer." << endl;
      continue;
    }
    
    // SYBMSDATETIME2 SYBMSDATETIMEOFFSET SYBMSTIME -> DATETIME2BIND(since 2014) 
    // SYBMSXML -> SYBTEXT
    // http://infocenter.sybase.com/help/index.jsp?topic=/com.sybase.help.ocs_12.5.1.dblib/html/dblib/X14206.htm
    switch (pcol->type) {
      case 35:
      case 241: // xml
        erc = dbbind(dbproc, c, BINARYBIND, 0, (BYTE*)pcol->buffer);
        break;
      default:
        erc = dbbind(dbproc, c, NTBSTRINGBIND, 0, (BYTE*)pcol->buffer);
        break;
    }
    
    if (erc == FAIL)
      cerr << "dbbind {} failed " << c << endl; 
    
    erc = dbnullbind(dbproc, c, &pcol->status);
    if (erc == FAIL)
      cerr << "dbnullbind {} failed " << c << endl;
  }
  
  return 0;
};

int tds::TDSClient::fetchData() {
  while ((row_code = dbnextrow(dbproc)) != NO_MORE_ROWS) {
    vector<string> row;
    
    switch (row_code) {
    case REG_ROW:
      for (pcol = columns; pcol - columns < ncols; pcol++) {
        //    From Microsoft documentation:
        //    "A NULL BYTE pointer is returned if there is no such column or if the
        //    data has a null value. To make sure that the data is really a null
        //    value, check for a return of 0 from *dbdatlen*."
        
        if (pcol->buffer != NULL) {
          row.push_back(string(pcol->buffer));
        } else {
          row.push_back("");
        }
      }
      fieldValues.push_back(row);
      break;

    case BUF_FULL:
      assert(row_code != BUF_FULL);
      break;

    case FAIL:
      cerr << "dbresults failed" << endl;
      break;

    default:
      cerr << "Data for computeid {} ignored" << row_code << endl;
    }

  }

  return 0;
};

int tds::TDSClient::execute() {
  auto status = dbsqlexec(dbproc);

  if (status == FAIL) {
    cerr << "execute() dbsqlexec failed" << endl;
    return 1;
  }

  while ((erc = dbresults(dbproc)) != NO_MORE_RESULTS) {
    if (erc == FAIL) {
      cerr << "execute() no results" << endl;
      return 1;
    }

    getMetadata();

    fetchData();
  }

  return 0;
};

