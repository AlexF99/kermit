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
    int n_mensagens = 1;
    char dados[64];
    char nome_arquivo[100];

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
        msg = cria_mensagem(0, 0, BACKUP_ARQUIVO, 0);
        pacote = empacota_mensagem(msg);
        memcpy(buffer, pacote, sizeof(unsigned char) * 68);
        send(socket, buffer, sizeof(unsigned char) * 68, 0);

        while (!feof(arq))
        {
            //sleep(1);
            int i = 0;
            while (i < 63 && !feof(arq))
            {
                a = getc(arq);
                dados[i] = a;
                ++i;
            }

            msg = cria_mensagem((unsigned char)i, (unsigned char)n_mensagens, DADOS, 0);
            memcpy(msg->dados, dados, i);
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
