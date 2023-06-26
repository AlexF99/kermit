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
#include <openssl/md5.h>

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

    for (;;)
    {
        memset(buffer_out, 0, 67);
        memset(buffer_in, 0, 67);
        entrada_t *entrada = get_entrada();
        if (entrada == NULL)
            continue;

        if (entrada->comando == BACKUP)
        {
            for (int i = 0; i < entrada->num_params; i++)
            {
                memset(nome_arquivo, 0, 100);
                strcpy(nome_arquivo, entrada->params[i]);
                printf("nome arquivo: \"%s\"\n", entrada->params[i]);
                FILE *arq = fopen(nome_arquivo, "r");
                printf("%s\n", nome_arquivo);

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

                msg_out = cria_mensagem(strlen(nome_arquivo), 0, RECUPERA_ARQUIVO, (unsigned char *)nome_arquivo);
                envia_mensagem(msg_out, buffer_out, socket);

                do                
                {
                    recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
                    desempacota_mensagem(buffer_in, &msg_in);
                } while (msg_in->tipo != ERRO && msg_in->tipo != DADOS);

                if (msg_in->tipo == ERRO)
                {
                    char tipo_erro = (msg_in->dados[0]);
                    char * nome_arq_erro = (char *)(msg_in->dados)+2;

                    if (strcmp(nome_arquivo, nome_arq_erro) == 0)
                    {
                        switch (tipo_erro)
                        {
                        case DISCO_CHEIO:
                            printf("ERRO: Servidor sem espaço de armazenamento disponivel.");
                            break;

                        case SEM_PERMISSAO:
                            printf("ERRO: Sem permissão em: %s.\n", nome_arq_erro);
                            break;

                        case ARQ_NAO_EXISTE:
                            printf("ERRO: Arquivo %s não encontrado.\n", nome_arq_erro);
                            break;

                        default:
                            printf("ERRO: Algo inesperado aconteceu.\n");
                            break;
                        }

                        continue;
                    }
                }

                char bkp_str[100] = "recupera_";
                strcat(bkp_str, nome_arquivo);
                FILE *arq = fopen(bkp_str, "w+");

                if (!arq)
                {
                    printf("Erro ao recuperar backup (client)\n");
                    continue;
                }

                recebe_arquivo(arq, buffer_out, buffer_in, socket);
            }
        }
        else if (entrada->comando == CD)
            continue;
        else if (entrada->comando == BKPDIR)
        {
            memset(nome_arquivo, 0, 100);
            strcpy(nome_arquivo, entrada->params[0]);
            msg_out = cria_mensagem(strlen(nome_arquivo), 0, SERVER_DIR, (unsigned char *)nome_arquivo);
            envia_mensagem(msg_out, buffer_out, socket);
        }
        else if (entrada->comando == MD5_E)
        {
            strcpy(nome_arquivo, entrada->params[1]);
            printf("nome arquivo: \"%s\"\n", entrada->params[0]);
            FILE *arq = fopen(nome_arquivo, "rb");
            int seq = 0;

            if (!arq)
            {
                printf("Erro ao abrir arquivo %s\n", nome_arquivo);
                continue;
            }

            msg_out = cria_mensagem(strlen(nome_arquivo), seq++, MD5_T, (unsigned char *)nome_arquivo);
            envia_mensagem(msg_out, buffer_out, socket);

            do
            {
                recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
                desempacota_mensagem(buffer_in, &msg_in);
            } while (msg_in->tipo != ACK);

            unsigned char hash[MD5_DIGEST_LENGTH];
            unsigned char data[1024];
            int bytes;

            MD5_CTX mdContext;
            MD5_Init(&mdContext);

            while ((bytes = fread(data, 1, 1024, arq)) != 0)
                MD5_Update(&mdContext, data, bytes);
            MD5_Final(hash, &mdContext);

            msg_out = cria_mensagem(strlen((char*)hash), seq++, DADOS, hash);
            envia_mensagem(msg_out, buffer_out, socket);

            do
            {
                recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
                desempacota_mensagem(buffer_in, &msg_in);
            } while ((msg_in->tipo != DADOS || msg_in->tipo != ERRO) && msg_in->sequencia != seq);

            printf("%s\n", (char *)msg_in->dados);

            fclose(arq);
        }
    }

    close(socket);
    return 0;
}
