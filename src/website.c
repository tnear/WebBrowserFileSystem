#include "website.h"
#include <stdlib.h>
#include <string.h>

Website* initWebsite(const char *url, const char *path, const char *html, time_t time)
{
    Website *website = malloc(sizeof(Website));

    size_t len = strlen(url);
    website->url = malloc(len + 1);
    website->url[len] = '\0';
    strcpy(website->url, url);

    len = strlen(path);
    website->path = malloc(len + 1);
    website->path[len] = '\0';
    strcpy(website->path, path);

    len = strlen(html);
    website->html = malloc(len + 1);
    website->html[len] = '\0';
    strcpy(website->html, html);

    website->htmlLen = len;
    website->time = time;

    return website;
}

void freeWebsite(Website *website)
{
    if (!website)
        return;

    free(website->url);
    free(website->path);
    free(website->html);
    free(website);

    website = NULL;
}
