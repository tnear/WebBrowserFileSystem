CC = gcc
#CC = clang
CFLAGS = -g
LDFLAGS = -lcurl -lsqlite3

# C-files
SRCS = test.c ../src/util.c ../src/linkedList.c ../src/operations.c ../src/sqlite.c ../src/website.c ../src/fuseData.c

# Mapping of .c -> .o files
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: test

test: $(OBJS)
# Build src first before tests
	(cd ../src; make all)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

# Build C-files with the same flags
%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

.PHONY: clean
clean:
	rm -f test $(OBJS) *.core
