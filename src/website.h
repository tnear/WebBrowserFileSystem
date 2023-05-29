#ifndef WEBSITE_H
#define WEBSITE_H

#include <time.h>

typedef struct Website
{
    char *url;
    char *path;
    char *html;
    int htmlLen;
    time_t time;
} Website;

Website* initWebsite(const char *url, const char *path, const char *html, time_t time);
void freeWebsite(Website *website);

#endif
