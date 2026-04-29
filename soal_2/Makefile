CC = gcc
CFLAGS = -lpthread -lrt

all: orion eternal

orion: orion.c arena.h
	$(CC) orion.c -o orion $(CFLAGS)

eternal: eternal.c arena.h
	$(CC) eternal.c -o eternal $(CFLAGS)

clean:
	rm -f orion eternal history.log

clear_ipc:
	ipcs -m | grep `whoami` | awk '{print $$2}' | xargs -r ipcrm -m
	ipcs -s | grep `whoami` | awk '{print $$2}' | xargs -r ipcrm -s
	ipcs -q | grep `whoami` | awk '{print $$2}' | xargs -r ipcrm -q
