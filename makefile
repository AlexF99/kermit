run:
	gcc -Wall -c socket.c -o socket.o
	gcc -Wall socket.o server.c -o server
	gcc -Wall socket.o client.c -o client