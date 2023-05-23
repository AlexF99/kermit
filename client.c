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

int main(int argc, char const *argv[])
{
    int socket = ConexaoRawSocket("lo");
    ssize_t message_size;
    unsigned char *buffer = (unsigned char *)malloc(68);
    memset(buffer, 0, 68);
    memcpy(buffer, "lewis", 5);

    mensagem_t * msg = cria_mensagem(0b00100111, 0b00101011, 0xf, 0x0);
    empacota_mensagem(msg);
    return;

    for (;;)
    {
        sleep(1);
        message_size = send(socket, buffer, sizeof(unsigned char) * 68, 0);
        printf("%zd\n", message_size);
    }
    return 0;
}
