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

void envia_arquivo(char *nome_arquivo, unsigned char *buffer, int socket)
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
    mensagem_t *msg = cria_mensagem(strlen(nome_arquivo), 0, BACKUP_ARQUIVO, 0);
    memcpy(msg->dados, nome_arquivo, strlen(nome_arquivo));
    unsigned char *pacote = empacota_mensagem(msg);
    memcpy(buffer, pacote, sizeof(unsigned char) * 68);
    send(socket, buffer, sizeof(unsigned char) * 68, 0);

    while (!feof(arq))
    {
        if (n_mensagens >= 64)
            n_mensagens = 1;

        bytes_lidos = fread(dados, 1, 63, arq);

        msg = cria_mensagem((unsigned char)bytes_lidos, (unsigned char)n_mensagens, DADOS, 0);
        memcpy(msg->dados, dados, bytes_lidos);
        memset(dados, 0, 64);
        ++n_mensagens;

        pacote = empacota_mensagem(msg);
        memcpy(buffer, pacote, sizeof(unsigned char) * 68);
        send(socket, buffer, sizeof(unsigned char) * 68, 0);

        // printf("Enviando mensagem: (Client)\n");
        // imprime_mensagem(msg);
    }

    msg = cria_mensagem(0, n_mensagens, FIM_ARQUIVO, 0);
    pacote = empacota_mensagem(msg);
    memcpy(buffer, pacote, sizeof(unsigned char) * 68);
    send(socket, buffer, sizeof(unsigned char) * 68, 0);

    // printf("Enviando mensagem: (Client)\n");
    // imprime_mensagem(msg);

    fclose(arq);
}

int main(int argc, char const *argv[])
{
    int socket = ConexaoRawSocket("lo");
    unsigned char *buffer = (unsigned char *)malloc(68);
    memset(buffer, 0, 68);

    for (;;)
    {
        entrada_t *entrada = get_entrada();
        if (entrada->comando == BACKUP)
        {
            for (int i = 0; i < entrada->num_params; i++)
                envia_arquivo(entrada->params[i], buffer, socket);
        }
    }
    return 0;
}
