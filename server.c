#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "socket.h"
#include "mensagem.h"

int main(int argc, char const *argv[])
{
    int socket = ConexaoRawSocket("lo");

    unsigned char *buffer = (unsigned char *)malloc(8); // to receive data
    memset(buffer, 0, 8);

    for (;;)
    {
        sleep(1);
        recv(socket, buffer, htons(8), 0);
        printf("%s\n", buffer);
    }
    return 0;
}
