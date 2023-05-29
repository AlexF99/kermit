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

    int counter;

    for (;;)
    {
        recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
        msg_in = desempacota_mensagem(buffer_in);

        if (msg_in && msg_in->tipo == BACKUP_ARQUIVO)
        {
            printf("dados: %s\n", msg_in->dados);
            printf("Começando transmissão\n");
            counter = 1;

            char bkp_str[100] = "backup_";

            strcat(bkp_str, (char *)msg_in->dados);
            arq = fopen(bkp_str, "w+");

            msg_out = cria_mensagem(0, counter, ACK, 0, NULL);
            imprime_mensagem(msg_out);
            envia_mensagem(msg_out, buffer_out, socket);

            if (!arq)
            {
                perror("Erro ao criar backup");
                exit(1);
            }

            while (msg_in->tipo != FIM_ARQUIVO)
            {
                
                if (counter >= 64)
                    counter = 1;

                recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
                msg_in = desempacota_mensagem(buffer_in);

                if (msg_in->sequencia == counter && msg_in->tipo == DADOS)
                {
                    fwrite((char *)msg_in->dados, 1, msg_in->tamanho, arq);
                    
                    msg_out = cria_mensagem(0, counter, ACK, 0, NULL);
                    envia_mensagem(msg_out, buffer_out, socket);
                    
                    counter++;
                }
            }
            printf("Backup realizado com sucesso!\n");
            fclose(arq);
        }
    }
    return 0;
}
