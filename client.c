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
    int sequencia_envio = 0;

    mensagem_t * msg_in;
    mensagem_t * msg_out;

    // Inicio da transmissão
    msg_out = cria_mensagem(strlen(nome_arquivo), sequencia_envio++, BACKUP_ARQUIVO, 0, (unsigned char *)nome_arquivo);
    envia_mensagem(msg_out, buffer_out, socket);

    // printf("Enviei mensagem: %d\n", msg_out->sequencia);

    do
    {
        recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
        msg_in = desempacota_mensagem(buffer_in);
    } while (msg_in->tipo != OK);
    destroi_mensagem(msg_out);

    // printf("Recebi OK da mensagem: %d\n", msg_in->sequencia);
    
    while (!feof(arq))
    {
        if (sequencia_envio >= 64)
            sequencia_envio = 1;

        bytes_lidos = fread(dados, 1, 63, arq);

        msg_out = cria_mensagem((unsigned char)bytes_lidos, sequencia_envio++, DADOS, 0, (unsigned char *)dados);
        memset(dados, 0, 64);

        envia_mensagem(msg_out, buffer_out, socket);
        // printf("Enviei mensagem: %d\n", msg_out->sequencia);

        do
        {
            destroi_mensagem(msg_in);
            recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
            msg_in = desempacota_mensagem(buffer_in);
        } while (msg_in->tipo != ACK || (msg_in->tipo == ACK && msg_in->sequencia != msg_out->sequencia));

        destroi_mensagem(msg_out);
        // printf("Recebi ACK da mensagem: %d\n", msg_in->sequencia);
    }

    // recv(socket, buffer_in, sizeof(unsigned char) * 67, 0);
    // msg = desempacota_mensagem(buffer_in);

    msg_out = cria_mensagem(0, sequencia_envio, FIM_ARQUIVO, 0, NULL);
    envia_mensagem(msg_out, buffer_out, socket);
    imprime_mensagem(msg_out);
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
