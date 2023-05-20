#include "website.h"
#include <stdlib.h>
#include <string.h>

Website* initWebsite(const char *url, const char *path, const char *html)
{
    Website *website = malloc(sizeof(Website));
    website->url = malloc(strlen(url));
    strcpy(website->url, url);
    website->path = malloc(strlen(path));
    strcpy(website->path, path);
    website->html = malloc(strlen(html));
    strcpy(website->html, html);

    return website;
}

void freeWebsite(Website *website)
{
    free(website->url);
    free(website->path);
    free(website->html);
    free(website);

    website = NULL;
}
