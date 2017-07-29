CC = cc
CFLAGS = -Wall -O2
LDFLAGS =

all: srank sconv
%.o: %.c
	$(CC) -c $(CFLAGS) $<
srank: srank.o sbuf.o sidx.o
	$(CC) -o $@ $^ $(LDFLAGS)
sconv: sconv.o sbuf.o
	$(CC) -o $@ $^ $(LDFLAGS)
clean:
	rm -f *.o srank
