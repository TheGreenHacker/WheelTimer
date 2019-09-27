TARGET: main
CC = gcc
CFLAGS = -Wall -Wextra -g 

LinkedList.o: LinkedList.c
	$(CC) $(CFLAGS) -c LinkedList.c -o LinkedList.o

WheelTimer.o: WheelTimer.c
	$(CC) $(CFLAGS) -pthread -c WheelTimer.c -o WheelTimer.o

main: LinkedList.o WheelTimer.o
	$(CC) $(CFLAGS) main.c -o main LinkedList.o WheelTimer.o

clean:
	rm *.o main

