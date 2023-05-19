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

int insertRow(sqlite3 *db, struct Website *website)
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

// note: caller must free(buffer)
char* getHtmlData(sqlite3 *db, const char *url)
{
    char *buffer = NULL;
    const char sql[] = "select HTML from " WEBSITE " where URL = ?;";
    
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    assert(rc == SQLITE_OK);
    
    sqlite3_bind_text(stmt, 1, url, -1, SQLITE_STATIC);

    int count = 0;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        ++count;
        const char *contents = sqlite3_column_text(stmt, 0);
        int len = strlen(contents);
        buffer = malloc(len + 1);
        buffer[len] = '\0';
        strcpy(buffer, contents);
    }
    
    assert(count <= 1);
    assert(rc == SQLITE_DONE);
    sqlite3_finalize(stmt);

    return buffer;
}
