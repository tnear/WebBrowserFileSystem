CC = gcc
CFLAGS = -g
# lcurl needed for curl, `...fuse` needed for fuse
LDFLAGS = -lcurl `pkg-config --libs --cflags fuse` -lsqlite3

# C-files
SRCS = fuse-main.c util.c linkedList.c operations.c sqlite.c website.c fuseData.c

# Mapping of .c -> .o files
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: fuse-main

fuse-main: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

# Build C-files with the same flags
%.o: %.c
	$(CC) -c $< $(CFLAGS) $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -f fuse-main $(OBJS) *.core
