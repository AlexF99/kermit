#include <stdio.h>
#include "socket.h"

int main(int argc, char const *argv[])
{
    int socket = ConexaoRawSocket("a8:1e:84:45:6a:cd");
    printf("socket: %d", socket);
    return 0;
}
