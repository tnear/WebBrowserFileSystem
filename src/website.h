#ifndef WEBSITE_H
#define WEBSITE_H

struct Website
{
    char *url;
    char *path;
    char *html;
};

struct Website* createWebsite(const char *url, const char *path, const char *html);
void deleteWebsite(struct Website *website);

#endif
