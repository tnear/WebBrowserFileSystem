#include "sqlite.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "website.h"

#define WEBSITE "Website"

sqlite3* createDatabase()
{
    sqlite3 *db = NULL;

    // create in-memory database
    int rc = sqlite3_open(":memory:", &db);
    assert(rc == SQLITE_OK);

    int ret = _createWebsiteTable(db);
    assert(ret == SQLITE_OK);

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

int insertRow(sqlite3 *db, Website *website)
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

// note: caller must deleteWebsite(website)
Website* lookupWebsite(sqlite3 *db, const char *url)
{
    Website *website = NULL;
    char *path = NULL;
    char *html = NULL;
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
        const char *tempPath = sqlite3_column_text(stmt, 0);
        int len = strlen(tempPath);
        path = malloc(len + 1);
        path[len] = '\0';
        strcpy(path, tempPath);

        const char *tempHtml = sqlite3_column_text(stmt, 1);
        len = strlen(tempHtml);
        html = malloc(len + 1);
        html[len] = '\0';
        strcpy(html, tempHtml);
    }
    
    assert(rc == SQLITE_DONE);
    assert(count <= 1);

    if (count == 1)
    {
        website = createWebsite(url, path, html);
    }
    
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
