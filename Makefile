
# Banking Simulation, Makefile
# Fall 2014: CS 304 (Computer Organization)
# Professor: Gongbing Hong
# Made by: Divya Bathey

CC = gcc
CFLAGS = -g -Wall -I

banksim: bonus.c queue.c queue.h
	$(CC) -Wall -o banksim bonus.c queue.c queue.h -g -lpthread -I.

clean:
	rm -f *~ *.o banksim
