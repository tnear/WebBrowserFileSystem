#include "website.h"
#include <stdlib.h>
#include <string.h>

struct Website* createWebsite(const char *url, const char *path, const char *html)
{
    struct Website *website = malloc(sizeof(struct Website));
    website->url = malloc(strlen(url));
    strcpy(website->url, url);
    website->path = malloc(strlen(path));
    strcpy(website->path, path);
    website->html = malloc(strlen(html));
    strcpy(website->html, html);

    return website;
}

void deleteWebsite(struct Website *website)
{
    free(website->url);
    free(website->path);
    free(website->html);
    free(website);

    website = NULL;
}
