#ifndef WEBSITE_H
#define WEBSITE_H

typedef struct Website
{
    char *url;
    char *path;
    char *html;
} Website;

Website* createWebsite(const char *url, const char *path, const char *html);
void deleteWebsite(Website *website);

#endif
