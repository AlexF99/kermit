run: mensagem.o
	gcc -Wall -c socket.c -o socket.o
	gcc -Wall -c entrada.c -o entrada.o
	gcc -Wall socket.o mensagem.o server.c -o server
	gcc -Wall socket.o mensagem.o entrada.o client.c -o client

mensagem.o: mensagem.c
	gcc -Wall -c mensagem.c

clean:
	rm -rf *.o

purge: clean
	rm -rf client server teste
