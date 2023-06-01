# WebBrowserFileSystem
WebBrowserFileSystem (todo)

## Supported protocols
* HTTP/S
* FTP
* DICT
* Amazon S3 buckets (public)

## Installation
```bash
$ git clone <repo>
# install meson & ninja to build fuse
$ sudo apt install meson
$ sudo apt install ninja-build

# install fuse, directions here
https://github.com/libfuse/libfuse
Download tar
$ tar -xf <tar>.xz

# install fuse development tools
$ sudo apt install libfuse-dev

# install sqlite
$ sudo apt install libsqlite3-dev 

# install curl
$ sudo apt install libcurl4-openssl-dev

# Build
$ cd src
$ make

# Build and run tests (optional)
$ cd test
$ make
$ ./test

```
## Setup
```bash
$ cd src
# create mount directory (if doesn't already exist)
$ mkdir -p mnt
# start WebBrowserFileSystem executable using 'mnt' as the special directory
# (-f = run in foreground, required for print statements)
# (-s = single-threaded, aids debugging)
$ ./fuse-main.o -s -f mnt/

# open separate terminal/screen, then change to the mounted directory
$ cd src/mnt
# 
```

## Downloading files
```bash
# HTTP
$ cat example.com


```




Deleting files

Challenges:
- slash character
- trying to save inside mounted directory
- file attributes (permissions, name, timestamp)

mmap
- no native support, though still functional
- fuse has its own buffer
- mmap does work, uses extra copy
- offset

swapon:
FUSE allows you to create a custom filesystem that operates in user space, providing a way to implement file system behaviors using user-level code. It allows you to create virtual filesystems that can be mounted and accessed like regular filesystems.

On the other hand, swapon is a command used to activate swap space in Linux. Swap space is a dedicated partition or file used as virtual memory extension when the system's physical memory (RAM) is fully utilized. It is managed by the kernel and used for swapping out inactive memory pages.

FUSE and swapon serve different purposes and operate at different levels of the system. FUSE is focused on implementing custom filesystems with user-level code, while swapon is used for managing swap space at the kernel level.

---
