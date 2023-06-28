#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include "mensagem.h"
#include "arquivo.h"
#include "socket.h"

int envia_arquivo(FILE *arq, unsigned char *buffer_out, unsigned char *buffer_in, int socket)
{
    mensagem_t *msg_in = NULL;
    mensagem_t *msg_out = NULL;
    size_t bytes_lidos;
    char dados[64];
    int sequencia_envio = 1;

    while (!feof(arq))
    {
        if (sequencia_envio >= 64)
            sequencia_envio = 1;

        bytes_lidos = fread(dados, 1, 63, arq);

        msg_out = cria_mensagem((unsigned char)bytes_lidos, sequencia_envio, DADOS, (unsigned char *)dados);
        envia_mensagem(msg_out, buffer_out, socket);
        do
        {
            while (recv(socket, buffer_in, sizeof(unsigned char) * 67, 0) == -1)
            {
                printf("deu timeout no envio do arquivo\nenviando msg %d novamente\n", sequencia_envio);
                msg_out = cria_mensagem((unsigned char)bytes_lidos, sequencia_envio - 1, DADOS, (unsigned char *)dados);
                envia_mensagem(msg_out, buffer_out, socket);
            }
            memset(dados, 0, 64);
            destroi_mensagem(msg_in);
            desempacota_mensagem(buffer_in, &msg_in);
            if (msg_in && msg_in->tipo == ACK)
                sequencia_envio++;
            if (msg_in && msg_in->tipo == NACK)
            {
                printf("RECEBI UM NACK, reenviando mensagem...\n");
                envia_mensagem(msg_out, buffer_out, socket);
                continue;
            }
        } while (msg_in && (msg_in->tipo != ACK || (msg_in->tipo == ACK && msg_in->sequencia != msg_out->sequencia)));

        destroi_mensagem(msg_out);
    }

    msg_out = cria_mensagem(0, sequencia_envio, FIM_ARQUIVO, 0);
    envia_mensagem(msg_out, buffer_out, socket);

    fclose(arq);
    return 0;
}

int recebe_arquivo(FILE *arq, unsigned char *buffer_out, unsigned char *buffer_in, int socket)
{
    mensagem_t *msg_in = cria_mensagem(0, 0, 0, NULL);
    mensagem_t *msg_out;
    int last_msg = -1;
    int sequencia_recibo = 0;

    do
    {
        recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);

        if (desempacota_mensagem(buffer_in, &msg_in) == -1)
        {
            printf("ENVIANDO NACK...\n");
            msg_out = cria_mensagem(0, 0, NACK, NULL);
            envia_mensagem(msg_out, buffer_out, socket);
        }
    } while (msg_in->tipo != DADOS);

    while (msg_in->tipo != FIM_ARQUIVO)
    {
        if (sequencia_recibo >= 64)
            sequencia_recibo = 1;

        if (msg_in->tipo == DADOS && msg_in->sequencia != last_msg)
        {
            fwrite((char *)msg_in->dados, 1, msg_in->tamanho, arq);
            last_msg = msg_in->sequencia;
        }

        msg_out = cria_mensagem(63, msg_in->sequencia, ACK, NULL);
        envia_mensagem(msg_out, buffer_out, socket);
        sequencia_recibo = msg_in->sequencia + 1 >= 64 ? 1 : msg_in->sequencia + 1;

        int is_nack = 0;
        int num_nacks = 0;
        do
        {
            recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
            destroi_mensagem(msg_in);
            if (desempacota_mensagem(buffer_in, &msg_in) == -1)
            {
                printf("ENVIANDO NACK...\n");
                msg_out = cria_mensagem(0, 0, NACK, NULL);
                envia_mensagem(msg_out, buffer_out, socket);
                num_nacks++;
            }
        } while ((is_nack && num_nacks < 30) || (msg_in->tipo != DADOS && msg_in->tipo != FIM_ARQUIVO));

        destroi_mensagem(msg_out);
    }
    fclose(arq);
    return 0;
}

int envia_erro_arq(int error, char * nome_arquivo, unsigned char * buffer_out, int socket)
{
    mensagem_t * msg_out;
    char error_msg[63];

    switch (error)
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
    msg_out = cria_mensagem(sizeof(error_msg), 1, ERRO, (unsigned char *)error_msg);
    envia_mensagem(msg_out, buffer_out, socket);
    destroi_mensagem(msg_out);

    return 0;
}

int recebe_erro_arq(int tipo_erro, char * nome_arq_erro)
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

    return 0;
}
