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
#include "entrada.h"

void envia_arquivo(char *nome_arquivo, unsigned char *buffer_out, unsigned char *buffer_in, int socket)
{
    printf("nome arquivo: \"%s\"\n", nome_arquivo);

    FILE *arq = fopen(nome_arquivo, "r");
    if (!arq)
    {
        fprintf(stderr, "Erro ao abrir arquivo %s\n", nome_arquivo);
        return;
    }

    size_t bytes_lidos;
    char dados[64];
    int n_mensagens = 0;

    // Inicio da transmissão
    mensagem_t *msg = cria_mensagem(strlen(nome_arquivo), n_mensagens++, BACKUP_ARQUIVO, 0, (unsigned char *)nome_arquivo);
    envia_mensagem(msg, buffer_out, socket);

    recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
    msg = desempacota_mensagem(buffer_in);

    while (msg->tipo != ACK)
    {
        memset(buffer_in, 0, 67);
        recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
        msg = desempacota_mensagem(buffer_in);
    }

    while (!feof(arq))
    {
        if (n_mensagens >= 64)
            n_mensagens = 1;

        bytes_lidos = fread(dados, 1, 63, arq);

        msg = cria_mensagem((unsigned char)bytes_lidos, n_mensagens++, DADOS, 0, (unsigned char *)dados);
        memset(dados, 0, 64);

        envia_mensagem(msg, buffer_out, socket);
        recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
        msg = desempacota_mensagem(buffer_in);

        while (msg->tipo != ACK)
        {
            memset(buffer_in, 0, 67);
            recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
            msg = desempacota_mensagem(buffer_in);
        }

        // imprime_mensagem(msg);
    }

    // recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
    // msg = desempacota_mensagem(buffer_in);

    msg = cria_mensagem(0, n_mensagens, FIM_ARQUIVO, 0, NULL);
    envia_mensagem(msg, buffer_out, socket);

    // printf("Enviando mensagem: (Client)\n");
    // imprime_mensagem(msg);

    fclose(arq);
}

int main(int argc, char const *argv[])
{
    int socket = ConexaoRawSocket("lo");
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
                envia_arquivo(entrada->params[i], buffer_out, buffer_in, socket);
        }
    }
    return 0;
}
