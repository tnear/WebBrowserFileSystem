# fuse

Run (temporary):
$ umount mnt 2> /dev/null; ./build.sh && ./fuse-main.o -s -f  mnt

gdb with fuse:
// https://www.cs.hmc.edu/~geoff/classes/hmc.cs135.201109/homework/fuse/fuse_doc.html
$ gdb <prog>
(gdb) <set breakpoints>
(gdb) run -s -d <mnt_dir>
// -s = single-threaded (aids gdb)
// -d = debug (aids gdb)
// -f = run in foreground (required for printf statements)

Install:
fuse
sqlite
any others?

Supported protocols:
HTTP/S
FTP
DICT

Deleting files

Large files:
- set artificial cap then show preview for sites over that

Challenges:
- slash character
- trying to save inside mounted directory
- file attributes (permissions, name, timestamp)
- memory leaks (C++ has smart pointers which can wrap C apis, RAII)

mmap
- no native support, though still functional
- fuse has its own buffer
- mmap does work, uses extra copy
- offset

word count of .h and .c:
$ find . -name '*.[chS]' | xargs wc
 1288  3314 32136 ./test/test.c
   18    26   247 ./src/fuseData.h
   37   147  1337 ./src/operations.h
   22    68   572 ./src/linkedList.h
   21    37   384 ./src/fuseData.c
   38    79   778 ./src/website.c
   98   233  1973 ./src/linkedList.c
   55   152  1320 ./src/fuse-main.c
   25    63   626 ./src/sqlite.h
   14    30   240 ./src/website.h
  249   781  6126 ./src/sqlite.c
  352  1135  9902 ./src/operations.c
   28    91   803 ./src/util.h
  304   882  7525 ./src/util.c
 2549  7038 63969 total


swapon:
FUSE allows you to create a custom filesystem that operates in user space, providing a way to implement file system behaviors using user-level code. It allows you to create virtual filesystems that can be mounted and accessed like regular filesystems.

On the other hand, swapon is a command used to activate swap space in Linux. Swap space is a dedicated partition or file used as virtual memory extension when the system's physical memory (RAM) is fully utilized. It is managed by the kernel and used for swapping out inactive memory pages.

FUSE and swapon serve different purposes and operate at different levels of the system. FUSE is focused on implementing custom filesystems with user-level code, while swapon is used for managing swap space at the kernel level.

---