#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "socket.h"
#include <net/ethernet.h>
#include "mensagem.h"

int main(int argc, char const *argv[])
{
    printf("pqp\n");
    int socket = ConexaoRawSocket("lo");
    ssize_t message_size;

    unsigned char *buffer = (unsigned char *)malloc(68); // to receive data
    memset(buffer, 0, 68);

    for (;;)
    {
        sleep(1);
        message_size = recv(socket, buffer, sizeof(unsigned char) * 68, 0);
        printf("%zi\n", message_size);
        printf("%s\n\n", buffer);
    }
    return 0;
}
