#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <sybfront.h>	/* sybfront.h always comes first */
#include <sybdb.h>	/* sybdb.h is the only other file you need */
#include <boost/filesystem.hpp>
// #include "spdlog/spdlog.h"

#define TDSCLIENT_VERSION "0.1.3"
#define NULL_BUFFER "NULL"
#define INT_MAX 2147483647
#define TINYINT_MAX 32767

using namespace std;

namespace tds {
  static string loggerName = "tds::tdsclient::logger";
  static string logName = "tdsclient";

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

    char* nullBuffer = "";
    
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
}
