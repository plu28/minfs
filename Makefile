CC = gcc

CFLAGS = -Wall -g

TARGETS = minls minget

.PHONY: all clean

all: $(TARGETS)

minls: minfs.c minls.c
	$(CC) $(CFLAGS) -o $@ $^

minget: minfs.c minget.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJS) $(TARGETS)

