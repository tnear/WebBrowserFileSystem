#include "sqlite.h"
#include "linkedList.h"
#include "website.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Website table for storing url, path, and html data
#define WEBSITE "Website"

sqlite3* createDatabase()
{
    sqlite3 *db = NULL;

    // create in-memory database
    int rc = sqlite3_open(":memory:", &db);
    assert(rc == SQLITE_OK);

    rc = _createWebsiteTable(db);
    assert(rc == SQLITE_OK);

    return db;
}

int _createWebsiteTable(sqlite3 *db)
{
    const char* createTableQuery = "create table if not exists " WEBSITE " ("\
        "URL   text primary key," \
        "PATH  text not null," \
        "HTML  text not null);";

    int rc = sqlite3_exec(db, createTableQuery, 0, 0, NULL);
    assert(rc == SQLITE_OK);
    return rc;
}

int insertWebsite(sqlite3 *db, Website *website)
{
    const char *insertQuery = "insert into " WEBSITE " (URL, PATH, HTML) values (?, ?, ?);";
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, insertQuery, -1, &stmt, 0);
    assert(rc == SQLITE_OK);

    // Bind values to the prepared statement
    int idx = 1;
    sqlite3_bind_text(stmt, idx++, website->url,  -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, idx++, website->path, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, idx++, website->html, -1, SQLITE_STATIC);

    // Execute the prepared statement
    rc = sqlite3_step(stmt);
    assert(rc == SQLITE_DONE);

    // finalize the statement
    sqlite3_finalize(stmt);

    return SQLITE_OK;
}

// note: caller must freeWebsite(website)
Website* lookupWebsiteByUrl(sqlite3 *db, const char *url)
{
    Website *website = NULL;
    const char sql[] = "select PATH,HTML from " WEBSITE " where URL = ?;";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    assert(rc == SQLITE_OK);

    // bind values to prepared statement
    int idx = 1;
    sqlite3_bind_text(stmt, idx++, url, -1, SQLITE_STATIC);

    int count = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        ++count;
        const char *path = sqlite3_column_text(stmt, 0);
        const char *html = sqlite3_column_text(stmt, 1);

        website = initWebsite(url, path, html);
    }

    assert(rc == SQLITE_DONE);
    assert(count <= 1);

    // a website should be created only if count == 1
    assert( (count == 0 && !website) || (count == 1 && website) );

    sqlite3_finalize(stmt);

    return website;
}

Website *lookupWebsiteByFilename(sqlite3 *db, const char *filename)
{
    Website *website = NULL;
    const char sql[] = "select URL,HTML from " WEBSITE " where PATH = ?;";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    assert(rc == SQLITE_OK);

    // bind values to prepared statement
    int idx = 1;
    sqlite3_bind_text(stmt, idx++, filename, -1, SQLITE_STATIC);

    int count = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        ++count;
        const char *url = sqlite3_column_text(stmt, 0);
        const char *html = sqlite3_column_text(stmt, 1);

        website = initWebsite(url, filename, html);
    }

    assert(rc == SQLITE_DONE);
    assert(count <= 1);

    // a website should be created only if count == 1
    assert( (count == 0 && !website) || (count == 1 && website) );

    sqlite3_finalize(stmt);

    return website;
}

bool lookupURL(sqlite3 *db, const char *url)
{
    char *path = NULL;
    char *html = NULL;
    const char sql[] = "select count(*) from " WEBSITE " where URL = ?;";
    
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    assert(rc == SQLITE_OK);
    
    // bind values to prepared statement
    int idx = 1;
    sqlite3_bind_text(stmt, idx++, url, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    assert(rc == SQLITE_ROW);

    // get count
    int count = sqlite3_column_int(stmt, 0);
    assert(count <= 1);

    sqlite3_finalize(stmt);

    // return true when finding the URL
    return count > 0;
}

int getWebsiteCount(sqlite3 *db)
{
    const char sql[] = "select count(*) from " WEBSITE ";";
    
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    assert(rc == SQLITE_OK);
    
    rc = sqlite3_step(stmt);
    assert(rc == SQLITE_ROW);

    // get count
    int count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return count;
}

// returns linked list of all file names in database
// intended for readdir() which only needs file names
Node* getFileNames(sqlite3 *db)
{
    Node *llHead = NULL;

    char *path = NULL;
    const char sql[] = "select PATH from " WEBSITE ";";
    
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    assert(rc == SQLITE_OK);
    
    // bind values to prepared statement
    int idx = 1;
    sqlite3_bind_text(stmt, idx++, path, -1, SQLITE_STATIC);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const char *tempPath = sqlite3_column_text(stmt, 0);
        char url[] = ""; // not needed for file names
        llInsertNode(&llHead, tempPath, url);
    }
    
    sqlite3_finalize(stmt);
    return llHead;
}
