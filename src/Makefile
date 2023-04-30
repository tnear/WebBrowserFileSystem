CC = gcc
CFLAGS = -g 
FUSE_FLAGS = `pkg-config --libs --cflags fuse`

.PHONY: all
all: fuse-main

fuse-main: fuse-main.c
	$(CC) fuse-main.c $(CFLAGS) $(FUSE_FLAGS) -o $@ 

#%.o: %.c
#	$(CC) -c $< -o $@

.PHONY: clean
clean:
	rm -f fuse-main *.o
