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
