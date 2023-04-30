CC = gcc
CFLAGS = -g
# lcurl needed for curl, `...fuse` needed for fuse
LDFLAGS = -lcurl `pkg-config --libs --cflags fuse`

.PHONY: all
all: fuse-main

fuse-main: fuse-main.o util.o
	$(CC) -o $@ fuse-main.o util.o $(LDFLAGS)

# Build C-files
%.o: %.c
	$(CC) -c $< $(CFLAGS) $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -f fuse-main *.o
