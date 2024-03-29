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
    SERVER_DIR,
    ESC,
    VERIF,
    NOME_ARQUIVO,
    LIXO_0,
    DADOS,
    FIM_ARQUIVO,
    FIM_GRUPO_ARQUIVO,
    ERRO,
    OK,
    ACK,
    NACK
};

enum ERROS
{
    DISCO_CHEIO = '0',
    SEM_PERMISSAO = '1',
    ARQ_NAO_EXISTE = '2',
};

#define INICIO_MSG 126

mensagem_t *cria_mensagem(unsigned char tamanho, unsigned char sequencia, unsigned char tipo, unsigned char *dados);

void envia_mensagem(mensagem_t *msg, unsigned char *buffer, int socket);

void destroi_mensagem(mensagem_t *msg);

int desempacota_mensagem(unsigned char *pacote, mensagem_t **msg);

unsigned char *empacota_mensagem(mensagem_t *mensagem);

void imprime_mensagem(mensagem_t *msg);

#endif
