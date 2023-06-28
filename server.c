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
#include <openssl/md5.h>

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
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000;
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
            arq = fopen((char *) msg_in->dados, "w+");

            if (!arq)
                envia_erro_arq(errno, (char *) msg_in->dados, buffer_out, socket);
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
                envia_erro_arq(errno, nome_arquivo, buffer_out, socket);
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
        else if (msg_in && msg_in->tipo == VERIF)
        {
            FILE * arq = fopen((char *) msg_in->dados, "rb");

            if (arq)
            {
                unsigned char hash[MD5_DIGEST_LENGTH];
                unsigned char data[1024];
                int bytes;

                MD5_CTX mdContext;
                MD5_Init(&mdContext);

                while ((bytes = fread(data, 1, 1024, arq)) != 0)
                    MD5_Update(&mdContext, data, bytes);
                MD5_Final(hash, &mdContext);

                msg_out = cria_mensagem(strlen((char *) hash), 0, DADOS, hash);
                envia_mensagem(msg_out, buffer_out, socket);

                fclose(arq);
            }
            else
                envia_erro_arq(errno, (char *) msg_in->dados, buffer_out, socket);
        }
    }
    return 0;
}
