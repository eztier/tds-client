#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <sybfront.h>	/* sybfront.h always comes first */
#include <sybdb.h>	/* sybdb.h is the only other file you need */

#define TDSCLIENT_VERSION "0.1.1"
#define NULL_BUFFER "NULL"

using namespace std;

namespace tds {
  static string loggerName = "tds::tdsclient::logger";  

  class TDSClient{
  public:
    struct COL
    {
      char* name;
      char* buffer;
      int type, size, status;
    } *columns, *pcol;

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
    TDSClient(const string& _host, const string& _user, const string& _pass) : host(_host), user(_user), pass(_pass) {}
    ~TDSClient() {}
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

    char* nullBuffer = "NULL";    
  };
}
