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
    mensagem_t * mensagens[100];
    int n_mensagens = 0;
    char dados[64];

    arq = fopen("arquivo_client.txt", "r");

    if (!arq)
    {
        perror("Erro ao abrir arquivo.");
        exit(1);
    }

    while (!feof(arq))
    {
        int i = 0;
        while(i < 63 && !feof(arq))
        {
            a = getc(arq);
            dados[i] = a;
            ++i;
        }
        mensagens[n_mensagens] = cria_mensagem((unsigned char) i, (unsigned char) n_mensagens, 0b00001111, 0b10101010);
        memcpy((mensagens[n_mensagens])->dados, dados, i);
        memset(dados, 0, 64);
        n_mensagens++;
        // printf("%s\n", dados);
    }

    int socket = ConexaoRawSocket("lo");
    ssize_t message_size;
    unsigned char *buffer = (unsigned char *)malloc(68);
    memset(buffer, 0, 68);

    mensagem_t * msg;

    int j = 0;
    for (;;)
    {
        if (j == n_mensagens)
            j = 0;

        msg = mensagens[j];
        unsigned char * pacote = empacota_mensagem(msg);
        memcpy(buffer, pacote, sizeof(unsigned char) * 68);
        j++;

        printf("Enviando mensagem: (Client)\n");
        imprime_mensagem(msg);
        message_size = send(socket, buffer, sizeof(unsigned char) * 68, 0);

    }
    return 0;
}
