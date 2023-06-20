#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

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

        msg_out = cria_mensagem((unsigned char)bytes_lidos, sequencia_envio++, DADOS, 0, (unsigned char *)dados);
        memset(dados, 0, 64);
        envia_mensagem(msg_out, buffer_out, socket);

        do
        {
            destroi_mensagem(msg_in);
            if (recv(socket, buffer_in, sizeof(unsigned char) * 67, 0) == -1)
            {
                printf("deu timeout com recv (envio do arq)\n");
                return -1;
            }
            msg_in = desempacota_mensagem(buffer_in);
        } while (msg_in->tipo != ACK || (msg_in->tipo == ACK && msg_in->sequencia != msg_out->sequencia));

        destroi_mensagem(msg_out);
        // printf("Recebi ACK da mensagem: %d\n", msg_in->sequencia);
    }

    msg_out = cria_mensagem(0, sequencia_envio, FIM_ARQUIVO, 0, NULL);
    envia_mensagem(msg_out, buffer_out, socket);

    fclose(arq);
    return 0;
}

int recebe_arquivo(FILE *arq, unsigned char *buffer_out, unsigned char *buffer_in, int socket)
{
    mensagem_t *msg_in;
    mensagem_t *msg_out;

    int sequencia_recibo = 0;

    do
    {
        recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
        msg_in = desempacota_mensagem(buffer_in);
    } while (msg_in->tipo != DADOS);

    while (msg_in->tipo != FIM_ARQUIVO)
    {
        // printf("Recebi a mensagem: %d\n", msg_in->sequencia);
        if (sequencia_recibo >= 64)
            sequencia_recibo = 1;

        fwrite((char *)msg_in->dados, 1, msg_in->tamanho, arq);

        msg_out = cria_mensagem(63, msg_in->sequencia, ACK, 0, NULL);
        envia_mensagem(msg_out, buffer_out, socket);
        // printf("Enviei ACK para mensagem: %d\n", msg_out->sequencia);
        sequencia_recibo = msg_in->sequencia + 1 >= 64 ? 1 : msg_in->sequencia + 1;
        // printf("%d\n", sequencia_recibo);

        do
        {
            destroi_mensagem(msg_in);
            recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
            msg_in = desempacota_mensagem(buffer_in);
        } while ((msg_in->tipo != DADOS || msg_in->tipo != FIM_ARQUIVO) && msg_in->sequencia != sequencia_recibo);

        destroi_mensagem(msg_out);
    }
    fclose(arq);
    return 0;
}
