#ifndef WEBSITE_H
#define WEBSITE_H

typedef struct Website
{
    char *url;
    char *path;
    char *html;
    int htmlLen;
} Website;

Website* initWebsite(const char *url, const char *path, const char *html);
void freeWebsite(Website *website);

#endif
