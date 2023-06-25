run: client server

client: socket.o mensagem.o entrada.o arquivo.o client.c
	gcc -Wall socket.o mensagem.o entrada.o arquivo.o client.c -o client

server: socket.o mensagem.o arquivo.o server.c
	gcc -Wall socket.o mensagem.o entrada.o arquivo.o server.c -o server

entrada.o: entrada.c
	gcc -Wall -c entrada.c -o entrada.o

socket.o: socket.c
	gcc -Wall -c socket.c -o socket.o

mensagem.o: mensagem.c
	gcc -Wall -c mensagem.c

arquivo.o: arquivo.c
	gcc -Wall -c arquivo.c

clean:
	rm -rf *.o

purge: clean
	rm -rf client server teste
