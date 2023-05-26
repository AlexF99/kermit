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
    int n_mensagens;
    char dados[64];
    char nome_arquivo[63];
    size_t bytes_lidos;

    int socket = ConexaoRawSocket("lo");
    unsigned char *buffer = (unsigned char *)malloc(68);
    memset(buffer, 0, 68);

    mensagem_t * msg;
    unsigned char *pacote;

    for (;;)
    {
        scanf("%s", nome_arquivo);
        arq = fopen(nome_arquivo, "r");

        if (!arq)
        {
            perror("Erro ao abrir arquivo.");
            exit(1);
        }
        
        // Inicio da transmissão
        msg = cria_mensagem(63, 0, BACKUP_ARQUIVO, 0);
        memcpy(msg->dados, nome_arquivo, strlen(nome_arquivo));
        n_mensagens = 0;
        pacote = empacota_mensagem(msg);
        memcpy(buffer, pacote, sizeof(unsigned char) * 68);
        send(socket, buffer, sizeof(unsigned char) * 68, 0);
        n_mensagens++;

        while (!feof(arq))
        {
            if (n_mensagens >= 64)
                n_mensagens = 1;

            bytes_lidos = fread(dados, 1, 63, arq);

            msg = cria_mensagem((unsigned char) bytes_lidos, (unsigned char)n_mensagens, DADOS, 0);
            memcpy(msg->dados, dados, bytes_lidos);
            memset(dados, 0, 64);
            ++n_mensagens;

            pacote = empacota_mensagem(msg);
            memcpy(buffer, pacote, sizeof(unsigned char) * 68);
            send(socket, buffer, sizeof(unsigned char) * 68, 0);

            printf("Enviando mensagem: (Client)\n");
            imprime_mensagem(msg);
        }

        msg = cria_mensagem(0, n_mensagens, FIM_ARQUIVO, 0);
        pacote = empacota_mensagem(msg);
        memcpy(buffer, pacote, sizeof(unsigned char) * 68);
        send(socket, buffer, sizeof(unsigned char) * 68, 0);

        printf("Enviando mensagem: (Client)\n");
        imprime_mensagem(msg);
    }
    return 0;
}
