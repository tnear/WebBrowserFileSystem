#ifndef SQLITE_FUSE_H
#define SQLITE_FUSE_H

#include <sqlite3.h>
#include <stdbool.h>

struct Website;
struct Node;

#define DB_FILENAME "websites.db"

sqlite3* createDatabase();
void closeDatabase(sqlite3 *db);
int _createWebsiteTable(sqlite3 *db);
int insertWebsite(sqlite3 *db, struct Website *website);
struct Website *lookupWebsiteByUrl(sqlite3 *db, const char *url);
struct Website *lookupWebsiteByFilename(sqlite3 *db, const char *filename);

bool lookupURL(sqlite3 *db, const char *url);
void deleteURL(sqlite3 *db, const char *url);

int getWebsiteCount(sqlite3 *db);
struct Node* getFileNames(sqlite3 *db);

#endif
