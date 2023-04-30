#define FUSE_USE_VERSION 31

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>
#include <sys/stat.h>

// https://www.youtube.com/watch?v=LZCILvr5tUk

typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
                const struct stat *stbuf, off_t off);

char MY_FILE[] = "my_file";
char myText[] = "Hello world3!\n";
char HTML_FILE[] = "file.html";

/*
void downloadURL(char *url)
{
    // open file for writing
    FILE *file = fopen(HTML_FILE, "w");
    assert(file);

    // initialize curl
    CURL *curl = curl_easy_init();
    assert(curl);

    // set URL to download
    curl_easy_setopt(curl, CURLOPT_URL, url);

    // set file handle for output
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    // perform the download
    CURLcode res = curl_easy_perform(curl);

    // check for errors
    if (res != CURLE_OK)
    {
        printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // cleanup curl
    curl_easy_cleanup(curl);

    // close file handle
    fclose(file);
}
*/

static struct stat regular_file = {
    .st_mode = S_IFREG | 0400
};

int myfs_getattr(const char *path, struct stat *stbuf)
{
    if (strcmp(path, "/") == 0)
    {
        // Root path
        stbuf->st_mode = S_IFDIR | 0400;
    }
    else if (strcmp(path, "/my_file") == 0)
    {
        stbuf->st_mode = regular_file.st_mode;
        stbuf->st_size = strlen(myText);
    }


    return 0;
}

int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
    if (strcmp(path, "/") == 0)
    {
        // Root
        filler(buf, MY_FILE, &regular_file, 0);
    }

    return 0;
}

static int myfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    if (strcmp(path, "/my_file") == 0)
    {
        printf("read for my_file\n");
        //downloadURL("http://example.com");

        //FILE *fp = fopen(HTML_FILE, "r");
        //char buf[255] = {};
        //fread(buf, 255, 1, fp);

        //// Verify substring
        //assert(strstr(buf, "doctype") != 0);


        strcpy(buf, myText);

        return strlen(myText);
    }

    return -ENOENT;
}

struct fuse_operations myOperations = {
    .getattr = myfs_getattr,
    .read    = myfs_read,
    .readdir = myfs_readdir
};

int main(int argc, char* argv[])
{
    printf("main9\n");
    return fuse_main(argc, argv, &myOperations, NULL);
}
