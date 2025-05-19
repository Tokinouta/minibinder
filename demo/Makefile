CC = gcc
CFLAGS = -Wall -Wextra -O2 -I../lib -static

BINARIES = sender receiver ioctl

all: $(BINARIES)

sender: sender.c ../lib/minibinder.c ../lib/minibinder.h
	$(CC) $(CFLAGS) -o sender sender.c ../lib/minibinder.c

receiver: receiver.c ../lib/minibinder.c ../lib/minibinder.h
	$(CC) $(CFLAGS) -o receiver receiver.c ../lib/minibinder.c

ioctl: ioctl.c ../lib/minibinder.c ../lib/minibinder.h
	$(CC) $(CFLAGS) -o ioctl ioctl.c ../lib/minibinder.c

clean:
	rm -f $(BINARIES)