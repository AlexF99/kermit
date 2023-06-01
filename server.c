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
    mensagem_t *msg_in;
    mensagem_t *msg_out;
    FILE *arq;

    unsigned char *buffer_in = (unsigned char *)malloc(67); // to receive data
    unsigned char *buffer_out = (unsigned char *)malloc(67); // to receive data

    memset(buffer_in, 0, 67);
    memset(buffer_out, 0, 67);

    int sequencia_recibo = 0;

    for (;;)
    {
        recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
        msg_in = desempacota_mensagem(buffer_in);

        if (msg_in && msg_in->tipo == BACKUP_ARQUIVO)
        {
            char bkp_str[100] = "backup_";

            strcat(bkp_str, (char *)msg_in->dados);
            arq = fopen(bkp_str, "w+");

            if (!arq)
            {
                perror("Erro ao criar backup");
                exit(1);
            }

            msg_out = cria_mensagem(0, msg_in->sequencia, OK, 0, NULL);
            envia_mensagem(msg_out, buffer_out, socket);
            // printf("Enviei OK para mensagem: %d\n", msg_out->sequencia);

            do
            {
                recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
                msg_in = desempacota_mensagem(buffer_in);
            } while (msg_in->tipo != DADOS);

            destroi_mensagem(msg_out);

            while (msg_in->tipo != FIM_ARQUIVO)
            {
                // printf("Recebi a mensagem: %d\n", msg_in->sequencia);
                if (sequencia_recibo >= 64)
                    sequencia_recibo = 1;

                fwrite((char *)msg_in->dados, 1, msg_in->tamanho, arq);

                msg_out = cria_mensagem(63, msg_in->sequencia, ACK, 0, NULL);
                envia_mensagem(msg_out, buffer_out, socket);
                // printf("Enviei ACK para mensagem: %d\n", msg_out->sequencia);
                sequencia_recibo =  msg_in->sequencia + 1 >= 64 ? 1 : msg_in->sequencia + 1;
                // printf("%d\n", sequencia_recibo);

                do
                {
                    destroi_mensagem(msg_in);
                    recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
                    msg_in = desempacota_mensagem(buffer_in);
                } while ((msg_in->tipo != DADOS || msg_in->tipo != FIM_ARQUIVO) && msg_in->sequencia != sequencia_recibo);

                destroi_mensagem(msg_out);
            }
            printf("Backup realizado com sucesso!\n");
            fclose(arq);
        }
    }
    return 0;
}
