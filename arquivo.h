#ifndef __ARQ__
#define __ARQ__

int envia_arquivo(FILE * arq, unsigned char *buffer_out, unsigned char *buffer_in, int socket);
int recebe_arquivo(FILE * arq, unsigned char *buffer_out, unsigned char *buffer_in, int socket);
int envia_erro_arq(int error, char * nome_arquivo, unsigned char * buffer_out, int socket);
int recebe_erro_arq(int tipo_erro, char * nome_arq_erro);

#endif