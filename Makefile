CC = gcc
CFLAGS = -Wall -pthread

myshell:
	$(CC) $(CFLAGS) -o myshell myshell.c

clean:
	rm myshell *.o *~