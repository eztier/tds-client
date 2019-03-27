#include "tds-client.h"
#include <thread> 

using namespace std;

int spec_0 (map<string, string>& sqlconf, const istream& input) {
  vector<string> fieldNames;
  vector<vector<string>> fieldValues;
  
  auto rc = tds::quick(sqlconf, input, fieldNames, fieldValues);

  if (rc) return rc;

  cout << "Thread " << std::this_thread::get_id() << endl;

  int idx = 0, cnt = fieldNames.size() - 1;
  for (auto& fld : fieldNames) {
    cout << fld << (idx < cnt ? "\t" : "");
    idx++;
  }
  cout << endl;
  idx = 0;
  
  for (auto& row : fieldValues) {
    for (const auto& col: row) {
      cout << col << (idx < cnt ? "\t" : "");
    }
    cout << endl;
  }
  cout << endl;

	return 0;  
}

auto main(int argc, char *argv[]) -> int {

  int rc = 0;

  map<string, string> config{{"host", "localhost:1433"}, {"user", "admin"}, {"pass", "12345678"}, {"database", "master"}};

  {
    std::vector<std::thread> v;
    for(int n = 0; n < 1000; ++n) {
      v.emplace_back([&]{
        spec_0(config, istringstream("select current_timestamp [time]"));
      });
    }

    this_thread::sleep_for(chrono::seconds(1));

    for(auto& t : v) {
      t.join();
    }

  }

  switch ( argc ) {
    case 1: 
      rc = spec_0( config, istringstream("select current_timestamp") );
      break;
    case 2: 
      rc = spec_0( config, ifstream( argv[1] ));
      break;
    default: 
      exit( EXIT_FAILURE );
  }

  return rc;
}
