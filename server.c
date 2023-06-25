#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <errno.h>

#include "socket.h"
#include "mensagem.h"
#include "entrada.h"
#include "arquivo.h"

int main(int argc, char const *argv[])
{
    printf("  \n");
    int socket = ConexaoRawSocket("lo");
    mensagem_t *msg_in = cria_mensagem(0, 0, 0, NULL);
    mensagem_t *msg_out;
    FILE *arq;

    // timeout config
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0)
    {
        fprintf(stderr, "Erro ao definir o timeout de recv: %s\n", strerror(errno));
        exit(1);
    }

    unsigned char *buffer_in = (unsigned char *)malloc(67);  // to receive data
    unsigned char *buffer_out = (unsigned char *)malloc(67); // to receive data

    for (;;)
    {
        memset(buffer_in, 0, 67);
        memset(buffer_out, 0, 67);
        recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
        if (desempacota_mensagem(buffer_in, &msg_in) == -1)
        {
            printf("ENVIANDO NACK...\n");
            msg_out = cria_mensagem(0, 0, NACK, NULL);
            envia_mensagem(msg_out, buffer_out, socket);
        }
        if (msg_in && msg_in->tipo == BACKUP_ARQUIVO)
        {
            char bkp_str[100] = "backup_";

            strcat(bkp_str, (char *)msg_in->dados);
            arq = fopen(bkp_str, "w+");

            if (!arq)
            {
                char error_msg[63];
                switch (errno)
                {
                case ENOMEM:
                    strcpy(error_msg, "0-"); 
                    break;

                case EACCES:
                    strcpy(error_msg, "1-");
                    break;

                case ENOENT:
                    strcpy(error_msg, "2-");
                    break;
                
                default:
                    strcpy(error_msg, "3-");
                    break;
                }

                strcat(error_msg, (char *)msg_in->dados);
                msg_out = cria_mensagem(sizeof(error_msg), 1, ERRO, (unsigned char *) error_msg);
                envia_mensagem(msg_out, buffer_out, socket);
            }
            else
            {
                msg_out = cria_mensagem(0, msg_in->sequencia, OK, 0);
                envia_mensagem(msg_out, buffer_out, socket);

                recebe_arquivo(arq, buffer_out, buffer_in, socket);
                printf("Backup realizado com sucesso!\n");
            }
        }

        else if (msg_in && msg_in->tipo == RECUPERA_ARQUIVO)
        {
            char nome_arquivo[100];
            strcpy(nome_arquivo, (char *)msg_in->dados);

            FILE *arq = fopen(nome_arquivo, "r");

            if (!arq)
            {
                char error_msg[63];
                switch (errno)
                {
                case ENOMEM:
                    strcpy(error_msg, "0-"); 
                    break;

                case EACCES:
                    strcpy(error_msg, "1-");
                    break;

                case ENOENT:
                    strcpy(error_msg, "2-");
                    break;
                
                default:
                    strcpy(error_msg, "3-");
                    break;
                }

                strcat(error_msg, nome_arquivo);
                msg_out = cria_mensagem(sizeof(error_msg), 1, ERRO, (unsigned char *) error_msg);
                envia_mensagem(msg_out, buffer_out, socket);
            }
            else
            {
                if (envia_arquivo(arq, buffer_out, buffer_in, socket) != -1)
                    printf("Backup recuperado com sucesso!\n");
            }
        }
        else if (msg_in && msg_in->tipo == SERVER_DIR)
        {
            char diretorio[100];
            strcpy(diretorio, (char *)msg_in->dados);
            cd_local(diretorio);
        }
    }
    return 0;
}
