CC = gcc
CFLAGS = -g
LDFLAGS = -lcurl

# C-files
SRCS = test.c ../src/util.c ../src/linkedList.c

# Mapping of .c -> .o files
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: test

test: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

# Build C-files with the same flags
%.o: %.c
	$(CC) -c $< $(CFLAGS) $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -f test $(OBJS)
