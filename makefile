run:
	gcc -Wall -c entrada.c -o entrada.o
	gcc -Wall -c socket.c -o socket.o
	gcc -Wall socket.o server.c -o server
	gcc -Wall socket.o entrada.o client.c -o client