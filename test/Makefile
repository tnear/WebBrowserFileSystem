CC = gcc
CFLAGS = -g
LDFLAGS = -lcurl

.PHONY: all
all: test

test: test.o ../src/util.o
	$(CC) -o $@ test.o ../src/util.o $(LDFLAGS)

# Build C-files
%.o: %.c
	$(CC) -c $< $(CFLAGS) $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -f test *.o
