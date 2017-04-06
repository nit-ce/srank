CC = cc
CFLAGS = -Wall -O2
LDFLAGS =

all: srank
%.o: %.c
	$(CC) -c $(CFLAGS) $<
srank: srank.o sbuf.o sidx.o
	$(CC) -o $@ $^ $(LDFLAGS)
clean:
	rm -f *.o srank
