#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "socket.h"
#include "mensagem.h"

void print_byte(unsigned char c, int tam)
{
    for (int i = 0; i < tam; i++)
        printf("%d", !!((c << i) & 0x80));
    printf("\n");
}

int main(int argc, char const *argv[])
{
    printf("operacoes:\ncd local: cd\nbackup: backup <nome arquivos ou regex>\n");
    printf("recuperar backup: recupera <nome arquivos ou regex>\n");
    printf("define dir p/ backup\nbkpdir <caminho>\nverifica backup: verifica <nome arquivo>\n");
    char entrada[100];
    char ch;

    for (int loop = 0; (ch = getc(stdin)) != '\n';)
    {
        entrada[loop] = ch;
        ++loop;
    }

    printf("entrada: %s\n", entrada);

    int socket = ConexaoRawSocket("lo");
    unsigned char *buffer = (unsigned char *)malloc(8);
    memset(buffer, 0, 8);
    memcpy(buffer, "ale", 8);
    for (;;)
    {
        sleep(1);
        send(socket, buffer, htons(8), 0);
    }
    return 0;
}
