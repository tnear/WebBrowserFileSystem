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
