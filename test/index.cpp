#include "tds-client.h"
#include <map>

using namespace std;

auto getFile = [](const string& fileName) -> string {
  ifstream doc(fileName);
  string xstr;
  xstr.assign(istreambuf_iterator<char>(doc), istreambuf_iterator<char>());
  return move(xstr);
};

int spec_0(map<string, string> sqlconf, const string& script) {
  auto db = tds::TDSClient();
  int rc;

  rc = db.connect(sqlconf["host"], sqlconf["user"], sqlconf["pass"]);
  
  if (rc)
		return rc;

  rc = db.useDatabase(sqlconf["database"]);
	if (rc)
		return rc;

  auto script2 = getFile("script.sql");
cout << script2 << endl;
  db.sql(script2);

	rc = db.execute();

  for (const auto& row : db.fieldValues) {
    for (const auto& col: row) {
      cout << col << endl;
    }
  }

	if (rc) {
		return rc;
	}

	return 0;  
}

int main(int argc, char *argv[]) {

  int rc = 0;
cout << "Started..." << endl;
  {
    rc = spec_0({{"host", "localhost"}, {"user", "admin"}, {"pass", "12345678"}, {"database", "master"}}, "select current_timestamp;");
    if (rc != 0) {
      throw ("Spec 0 failed.");
    }
  }

  return rc;
}
