CC = gcc
CFLAGS = -g
# lcurl needed for curl, `...fuse` needed for fuse
LDFLAGS = -lcurl `pkg-config --libs --cflags fuse`

# C-files
SRCS = fuse-main.c util.c linkedList.c

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
	rm -f fuse-main $(OBJS)
