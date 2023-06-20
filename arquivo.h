#ifndef __ARQ__
#define __ARQ__

int envia_arquivo(FILE * arq, unsigned char *buffer_out, unsigned char *buffer_in, int socket);
int recebe_arquivo(FILE * arq, unsigned char *buffer_out, unsigned char *buffer_in, int socket);

#endif