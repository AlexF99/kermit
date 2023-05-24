#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "socket.h"
#include "mensagem.h"

int main(int argc, char const *argv[])
{
    FILE * arq;
    char a;

    arq = fopen("arquivo_client.txt", "r");

    if (!arq)
    {
        perror("Erro ao abrir arquivo.");
        exit(1);
    }

    while (!feof(arq))
    {
        a = getc(arq);
        printf("%c\n", a);
    }

    int socket = ConexaoRawSocket("lo");
    ssize_t message_size;
    unsigned char *buffer = (unsigned char *)malloc(68);
    memset(buffer, 0, 68);

    mensagem_t * msg = cria_mensagem(0b00100111, 0b00101011, 0b00001111, 0b10101010);
    unsigned char * pacote = empacota_mensagem(msg);
    memcpy(buffer, pacote, sizeof(unsigned char) * 43);

    for (;;)
    {
        sleep(1);
        printf("Enviando mensagem: (Client)\n");
        imprime_mensagem(msg);
        message_size = send(socket, buffer, sizeof(unsigned char) * 68, 0);
    }
    return 0;
}
