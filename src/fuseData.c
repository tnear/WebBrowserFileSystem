#include "fuseData.h"
#include "sqlite.h"
#include <stdlib.h>

FuseData* initFuseData()
{
    FuseData *fuseData = malloc(sizeof(FuseData));
    fuseData->db = createDatabase();
    fuseData->useMmap = false;

    return fuseData;
}

void deleteFuseData(FuseData *fuseData)
{
    // close and delete database
    closeDatabase(fuseData->db);

    // free memory
    free(fuseData);
}
