#include "fuseData.h"
#include "sqlite.h"
#include <stdlib.h>

FuseData* initFuseData()
{
    FuseData *fuseData = malloc(sizeof(FuseData));
    fuseData->llHead = NULL;
    fuseData->db = createDatabase();

    return fuseData;
}

void deleteFuseData(FuseData *fuseData)
{
    // close database
    sqlite3_close(fuseData->db);

    // free memory
    free(fuseData);
}
