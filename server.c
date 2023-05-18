#include <stdio.h>
#include "socket.h"

int main(int argc, char const *argv[])
{
    int socket = ConexaoRawSocket("2175");
    printf("socket: %d", socket);
    return 0;
}
