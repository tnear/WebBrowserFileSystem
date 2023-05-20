#ifndef SQLITE_FUSE_H
#define SQLITE_FUSE_H

#include <sqlite3.h>
#include <stdbool.h>

struct Website;
struct Node;

sqlite3* createDatabase();
int _createWebsiteTable(sqlite3 *db);
int insertWebsite(sqlite3 *db, struct Website *website);
struct Website *lookupWebsite(sqlite3 *db, const char *url);
bool lookupURL(sqlite3 *db, const char *url);
int getWebsiteCount(sqlite3 *db);
struct Node* getFileNames(sqlite3 *db);

#endif
