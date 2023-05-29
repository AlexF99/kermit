#ifndef __MSG__
#define __MSG__

typedef struct mensagem_t
{
    unsigned char inicio;    // 8bits
    unsigned char tamanho;   // 6
    unsigned char sequencia; // 6
    unsigned char tipo;      // 4
    unsigned char *dados;    // ate 64
    unsigned char paridade;  // 8
} mensagem_t;

enum TIPOS
{
    BACKUP_ARQUIVO = 0,
    BACKUP_GRUPO,
    RECUPERA_ARQUIVO,
    RECUPERA_GRUPO,
    ESC,
    VERIF,
    NOME_ARQUIVO,
    LIXO_0,
    DADOS,
    FIM_ARQUIVO,
    FIM_GRUPO_ARQUIVO,
    LIXO_1,
    ERRO,
    OK,
    ACK,
    NACK
};

#define INICIO_MSG 126

mensagem_t *cria_mensagem(unsigned char tamanho, unsigned char sequencia, unsigned char tipo, unsigned char paridade, unsigned char * dados);

void envia_mensagem(mensagem_t * msg, unsigned char * buffer, int socket);

mensagem_t *desempacota_mensagem(unsigned char *pacote);

unsigned char *empacota_mensagem(mensagem_t *mensagem);

void imprime_mensagem(mensagem_t *msg);

#endif
