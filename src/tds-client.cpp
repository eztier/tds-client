#include "tds-client.h"

using namespace tds;

int err_handler_tdsclient(DBPROCESS* dbproc, int severity, int dberr, int oserr, char* dberrstr, char* oserrstr) {
  if ((dbproc == NULL) || (DBDEAD(dbproc))) {
    spdlog::get(loggerName)->error("dbproc is NULL error: {}", dberrstr);
    return(INT_CANCEL);
  }
  else {
    spdlog::get(loggerName)->error("DB-Library error: {} {}", dberr, dberrstr);

    if (oserr != DBNOERR) {
      spdlog::get(loggerName)->error("Operating-system error: {}", oserrstr);
    }
    // DO NOT CALL dbclose(dbproc)!; tds needs to clean itself up!
    dbexit();

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

  spdlog::get(loggerName)->warn("msgno: {} severity: {} msgstate: {}", msgno, severity, msgstate);

  if (strlen(srvname) > 0)
    spdlog::get(loggerName)->warn("Server: {}", srvname);

  if (strlen(procname) > 0)
    spdlog::get(loggerName)->warn("Procedure: {}", procname);

  if (line > 0)
    spdlog::get(loggerName)->warn("line: {}", line);

  spdlog::get(loggerName)->warn("msgtext: {}", msgtext);

  return(0);
}

int tds::TDSClient::connect(const string& _host, const string& _user, const string& _pass){
  host = _host;
  user = _user;
  pass = _pass;
  return connect();
};

int tds::TDSClient::init() {
  if (dbinit() == FAIL) {
    spdlog::get(loggerName)->error("dbinit() failed");
    return 1;
  }
  return 0;
};

int tds::TDSClient::connect() {
  if (dbinit() == FAIL) {
    spdlog::get(loggerName)->error("dbinit() failed");
    return 1;
  }

  //handle server/network errors
  dberrhandle((EHANDLEFUNC)err_handler_tdsclient);
  
  // Get a LOGINREC.
  if ((login = dblogin()) == NULL) {
    spdlog::get(loggerName)->error("connect() unable to allocate login structure");
    return 1;
  }

  DBSETLUSER(login, user.c_str());
  DBSETLPWD(login, pass.c_str());

  // Connect to server
  if ((dbproc = dbopen(login, host.c_str())) == NULL) {
    spdlog::get(loggerName)->error("connect() unable to connect to {}", host);
    return 1;
  }

  return 0;
};

int tds::TDSClient::useDatabase(const string& db){
  if ((erc = dbuse(dbproc, db.c_str())) == FAIL) {
    spdlog::get(loggerName)->error("useDatabase() unable to use database {}", db);
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
cout << "type: " << pcol->type << endl;    
cout << "size: " << pcol->size << endl;
    if (pcol->size == INT_MAX) {
      pcol->size = TINYINT_MAX;
    } else {
      pcol->size = 255;
    }

    fieldNames.push_back(move(string(pcol->name)));

    // void *full_msg = calloc(full_msg_size, sizeof(char));
    if ((pcol->buffer = (char*)calloc(1, pcol->size + 1)) == NULL){
      perror(NULL);
      return 1;
    }

    erc = dbbind(dbproc, c, NTBSTRINGBIND, 0, (BYTE*)pcol->buffer);

    if (erc == FAIL) {
      spdlog::get(loggerName)->error("dbnullbind {} failed", c);
      return 1;
    }
    erc = dbnullbind(dbproc, c, &pcol->status);
    if (erc == FAIL) {
      spdlog::get(loggerName)->error("dbnullbind {} failed", c);
      return 1;
    }
  }
  
  return 0;
};

int tds::TDSClient::fetchData() {
  while ((row_code = dbnextrow(dbproc)) != NO_MORE_ROWS) {
    vector<string> row;

    switch (row_code) {
    case REG_ROW:
      for (pcol = columns; pcol - columns < ncols; pcol++) {
        char *buffer = pcol->status == -1 ? nullBuffer : pcol->buffer;

        row.push_back(move(string(buffer)));
      }
      fieldValues.push_back(row);
      break;

    case BUF_FULL:
      assert(row_code != BUF_FULL);
      break;

    case FAIL:
      spdlog::get(loggerName)->error("dbresults failed");
      return 1;

    default:
      spdlog::get(loggerName)->info("Data for computeid {} ignored", row_code);
    }

  }

  /* free metadata and data buffers */
  for (pcol = columns; pcol - columns < ncols; pcol++) {
    free(pcol->buffer);
  }
  free(columns);

  return 0;
};

int tds::TDSClient::execute() {
  auto status = dbsqlexec(dbproc);

  if (status == FAIL) {
    spdlog::get(loggerName)->error("execute() dbsqlexec failed");
    return 1;
  }

  while ((erc = dbresults(dbproc)) != NO_MORE_RESULTS) {
    if (erc == FAIL) {
      spdlog::get(loggerName)->error("execute() no results");
      return 1;
    }

    getMetadata();

    fetchData();
  }

  dbclose(dbproc);
  dbexit();

  return 0;
};
