#ifndef FUSE_DATA_H
#define FUSE_DATA_H

#include <sqlite3.h>
#include <stdbool.h>

struct Node;

typedef struct FuseData
{
    sqlite3 *db;
    bool useMmap;
} FuseData;

FuseData* initFuseData();
void deleteFuseData(FuseData *fuseData);

#endif
