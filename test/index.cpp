#include "tds-client.h"
#include <map>

using namespace std;

int spec_0(map<string, string> sqlconf, const string& script) {
  auto db = tds::TDSClient();
  int rc;

  rc = db.connect(sqlconf["host"], sqlconf["user"], sqlconf["pass"]);
  
  if (rc)
		return rc;

  rc = db.useDatabase(sqlconf["database"]);
	if (rc)
		return rc;

  db.sql(script);

	rc = db.execute();

	if (rc) {
		return rc;
	}

	return 0;  
}

int main(int argc, char *argv[]) {

  int rc = 0;

  {
    rc = spec_0({{"host", "127.0.0.1"}, {"user", "guest"}, {"pass", "1234"}}, "select current_timestamp;");
    if (rc != 0) {
      throw ("Spec 0 failed.");
    }
  }

  return rc;
}
