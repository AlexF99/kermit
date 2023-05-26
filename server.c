#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "socket.h"
#include <net/ethernet.h>
#include "mensagem.h"

int main(int argc, char const *argv[])
{
    printf("  \n");
    int socket = ConexaoRawSocket("lo");
    mensagem_t * msg;
    FILE * arq;

    unsigned char *buffer = (unsigned char *)malloc(68); // to receive data
    memset(buffer, 0, 68);

    int counter;

    for (;;)
    {
        recv(socket, buffer, sizeof(unsigned char) * 68, 0);
        msg = desempacota_mensagem(buffer);

        if (msg && msg->tipo == BACKUP_ARQUIVO)
        {
            printf("Começando transmissão\n");
            counter = 1;

            arq = fopen("backup", "w+");

            if (!arq)
            {
                perror("Erro ao criar backup");
                exit(1);
            }

            while (msg->tipo != FIM_ARQUIVO)
            {
                recv(socket, buffer, sizeof(unsigned char) * 68, 0);
                msg = desempacota_mensagem(buffer);

                if (msg->sequencia == counter)
                {
                    if (msg->tipo == DADOS)
                    {
                        fprintf(arq, "%s", (char *) msg->dados);
                    }

                    counter++;
                }
            }
            printf("Backup realizado com sucesso!\n");
            fclose(arq);
        }
    }
    return 0;
}
