#ifndef SQLITE_FUSE_H
#define SQLITE_FUSE_H

#include <sqlite3.h>

struct Website;

sqlite3* createDatabase();
int _createWebsiteTable(sqlite3 *db);
int insertRow(sqlite3 *db, struct Website *website);
char* getHtmlData(sqlite3 *db, const char *url);

#endif
