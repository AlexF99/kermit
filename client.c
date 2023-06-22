#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "socket.h"
#include "mensagem.h"
#include "entrada.h"
#include "arquivo.h"

int main(int argc, char const *argv[])
{
    int socket = ConexaoRawSocket("lo");

    mensagem_t *msg_in = cria_mensagem(0, 0, 0, NULL);
    mensagem_t *msg_out;
    char nome_arquivo[100];

    // timeout config
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0)
    {
        fprintf(stderr, "Erro ao definir o timeout de recv: %s\n", strerror(errno));
        exit(1);
    }

    unsigned char *buffer_out = (unsigned char *)malloc(67);
    unsigned char *buffer_in = (unsigned char *)malloc(67);
    memset(buffer_out, 0, 67);
    memset(buffer_in, 0, 67);

    for (;;)
    {
        entrada_t *entrada = get_entrada();
        if (entrada->comando == BACKUP)
        {
            for (int i = 0; i < entrada->num_params; i++)
            {
                memset(nome_arquivo, 0, 100);
                strcpy(nome_arquivo, entrada->params[i]);
                printf("nome arquivo: \"%s\"\n", entrada->params[i]);
                FILE *arq = fopen(nome_arquivo, "r");

                if (arq)
                {
                    msg_out = cria_mensagem(strlen(nome_arquivo), 0, BACKUP_ARQUIVO, (unsigned char *)nome_arquivo);
                    envia_mensagem(msg_out, buffer_out, socket);

                    do
                    {
                        while (recv(socket, buffer_in, sizeof(unsigned char) * 67, 0) == -1)
                        {
                            printf("deu timeout com recv(nome arquivo)\n");
                            envia_mensagem(msg_out, buffer_out, socket);
                        }
                        if (desempacota_mensagem(buffer_in, &msg_in) == -1)
                        {
                            printf("ENVIANDO NACK...\n");
                            msg_out = cria_mensagem(0, 0, NACK, NULL);
                            envia_mensagem(msg_out, buffer_out, socket);
                        }
                    } while (msg_in && msg_in->tipo != OK);
                    destroi_mensagem(msg_out);
                    envia_arquivo(arq, buffer_out, buffer_in, socket);
                }
                else
                    fprintf(stderr, "Erro ao abrir arquivo %s\n", nome_arquivo);
            }
        }
        else if (entrada->comando == RECUPERA)
        {
            for (int i = 0; i < entrada->num_params; i++)
            {
                memset(nome_arquivo, 0, 100);
                strcpy(nome_arquivo, entrada->params[i]);

                char bkp_str[100] = "recupera_";
                strcat(bkp_str, nome_arquivo);
                FILE *arq = fopen(bkp_str, "w+");

                if (!arq)
                {
                    printf("Erro ao recuperar backup (client)\n");
                    return 1;
                }

                msg_out = cria_mensagem(strlen(nome_arquivo), 0, RECUPERA_ARQUIVO, (unsigned char *)nome_arquivo);
                envia_mensagem(msg_out, buffer_out, socket);

                recebe_arquivo(arq, buffer_out, buffer_in, socket);
            }
        }
        else if (entrada->comando == CD)
            continue;
    }

    close(socket);
    return 0;
}
