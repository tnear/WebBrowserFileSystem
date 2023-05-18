#ifndef SQLITE_FUSE_H
#define SQLITE_FUSE_H

#include <sqlite3.h>

sqlite3* createDatabase();
int _createWebsiteTable(sqlite3 *db);
int insertRow(sqlite3 *db);
//char* getFileData();

#endif
