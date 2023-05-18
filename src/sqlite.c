#include "sqlite.h"
#include <assert.h>
#include <stdio.h>

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
    //"ID  INTEGER PRIMARY KEY  AUTOINCREMENT,"

    const char* createTableQuery = "CREATE TABLE IF NOT EXISTS Website ("\
        "URL   TEXT PRIMARY KEY," \
        "PATH  TEXT NOT NULL," \
        "HTML  TEXT NOT NULL);";

    char* errMsg = 0;
    int rc = sqlite3_exec(db, createTableQuery, 0, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 1;
    }

    return SQLITE_OK;
}

int insertRow(sqlite3 *db)
{
    const char *insertQuery = "INSERT INTO Website (URL, PATH, HTML) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, insertQuery, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    const char *url = "www.example.com";
    const char *path = "example";
    const char *html = "<HTML>";

    // Bind values to the prepared statement
    sqlite3_bind_text(stmt, 1, url, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, path, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, html, -1, SQLITE_STATIC);

    // Execute the prepared statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // finalize the statement and close the database
    sqlite3_finalize(stmt);
    //sqlite3_close(db);

    return SQLITE_OK;
}

// https://stackoverflow.com/questions/31146713/sqlite3-exec-callback-function-clarification