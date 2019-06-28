CFLAGS = -D_GNU_SOURCE -std=c11 -pedantic -Wall -Wvla -Werror

all: maint gstat server client

client: utils.o sock.o client.c
	cc $(CFLAGS) -o client client.c utils.o sock.o

server: utils.o sem.o sharemem.o sock.o server.c
	cc $(CFLAGS) -o server server.c utils.o sem.o sharemem.o sock.o

gstat: utils.o sem.o sharemem.o gstat.c
	cc $(CFLAGS) -o gstat gstat.c utils.o sem.o sharemem.o

maint: utils.o sem.o sharemem.o maint.c
	cc $(CFLAGS) -o maint maint.c utils.o sem.o sharemem.o
	
sem.o: sem.h sem.c
	cc $(CFLAGS) -c sem.c

sharemem.o: sharemem.h sharemem.c
	cc $(CFLAGS) -c sharemem.c

sock.o : sock.h sock.c
	cc $(CFLAGS) -c sock.c

utils.o : utils.h utils.c
	cc $(CFLAGS) -c utils.c

clean:
	rm *.o
	rm gstat
	rm maint
	rm client
	rm server
	rm ./code_repository/*
	rm compiler_output/*.txt
	rm outputs/*.txt
