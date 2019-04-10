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
// #include <boost/filesystem.hpp>
// #include "spdlog/spdlog.h"

#define TDSCLIENT_VERSION "0.1.5"
#define NULL_BUFFER "NULL"
// #define INT_MAX 2147483647 // Just use __INT_MAX__
#define TINYINT_MAX 32767

using namespace std;

namespace tds {
  static string loggerName = "tds::tdsclient::logger";
  static string logName = "tdsclient";

  std::mutex log_mutex;

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

    char nullBuffer[1] = "";
    
    /*
    
    void setupLog() {
      if (boost::filesystem::create_directory("./log")){
        boost::filesystem::path full_path(boost::filesystem::current_path());
      }
  
      size_t q_size = 1048576; //queue size must be power of 2
      spdlog::set_async_mode(q_size);
  
      std::vector<spdlog::sink_ptr> sinks;
      sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink_mt>("log/" + logName, 0, 0));
      auto combined_logger = std::make_shared<spdlog::logger>(loggerName, begin(sinks), end(sinks));
      combined_logger->set_pattern("[%Y-%m-%d %H:%M:%S:%e] [%l] [thread %t] %v");
      spdlog::register_logger(combined_logger);
    }
    */
  };

  int quick(map<string, string>& sqlconf, const istream& input, vector<string>& fieldNames, vector<vector<string>>& fieldValues) {
    if (sqlconf.count("host") == 0 || sqlconf.count("user") == 0 || sqlconf.count("pass") == 0 || sqlconf.count("database") == 0)
      return 1;

    auto db = tds::TDSClient();
    int rc = 0;

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

    if (rc) {
      return rc;
    }

    // No errors
    fieldNames = std::move(db.fieldNames);
    fieldValues = std::move(db.fieldValues);

    return rc;
  }
}
