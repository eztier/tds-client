#pragma once

#include <cstring>
#include <cassert>
#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <sstream>
#include <mutex>
#include <thread>
#include <sybfront.h>	/* sybfront.h always comes first */
#include <sybdb.h>	/* sybdb.h is the only other file you need */

#define TDSCLIENT_VERSION "0.1.7"
#define NULL_BUFFER "NULL"
#define TINYINT_MAX 32767

using namespace std;

namespace tds {
  static string loggerName = "tds::tdsclient::logger";
  static string logName = "tdsclient";

  std::mutex log_mutex;

  struct TDSException : public std::runtime_error { 
  public:
    TDSException(char const* const message) throw();
  };

  class TDSClient{
  public:
    struct COL
    {
      char* name;
      char* buffer;
      int type, size, status;
    } *columns = NULL, *pcol = NULL;

    int init();
    int connect();
    int connect(const string& _host, const string& _user, const string& _pass);
    int useDatabase(const string& _db);
    void sql(const string& _script);
    int execute();
    int getMetadata();
    int fetchData();
    vector<string> fieldNames;
    vector<vector<string>> fieldValues;
    TDSClient() {};
    TDSClient(const string& _host, const string& _user, const string& _pass) : host(_host), user(_user), pass(_pass) { TDSClient(); };
    ~TDSClient() {
      /* free metadata and data buffers */
      if (columns != NULL) {
        for (pcol = columns; pcol - columns < ncols; pcol++) {
          if (pcol->buffer != NULL)
            free(pcol->buffer);
        }

        free(columns);
      }

      dbclose(dbproc);
      dbexit();
    }
  private:
    LOGINREC* login;
    DBPROCESS* dbproc;
    RETCODE erc;
    string host;
    string user;
    string pass;
    string script;
    int ncols;
    int row_code;
  };

  int quick(map<string, string>& sqlconf, const istream& input, vector<string>& fieldNames, vector<vector<string>>& fieldValues) {
    if (sqlconf.count("host") == 0 || sqlconf.count("user") == 0 || sqlconf.count("pass") == 0 || sqlconf.count("database") == 0)
      return 1;

    int rc = 0;

    {
      auto db = tds::TDSClient();
      
      rc = db.connect(sqlconf["host"], sqlconf["user"], sqlconf["pass"]);
      
      if (rc) {
        cout << "No connection" << endl;
        return rc;
      }

      rc = db.useDatabase(sqlconf["database"]);
      if (rc) {
        cout << "Cannot switch database" << endl;
        return rc;
      }

      ostringstream oss;
      oss << input.rdbuf();

      db.sql(oss.str());

      rc = db.execute();

      // No errors
      if (!rc) {
        fieldNames = std::move(db.fieldNames);
        fieldValues = std::move(db.fieldValues);
      }
    }

    return rc;
  }
}
