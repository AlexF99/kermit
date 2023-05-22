typedef struct mensagem_t
{
    unsigned char inicio;    // 8bits
    unsigned char tamanho;   // 6
    unsigned char sequencia; // 6
    unsigned char tipo;      // 4
    unsigned char *dados;    // ate 64
    unsigned char paridade;  // 8
} mensagem_t;

mensagem_t *cria_mensagem(unsigned char tamanho, unsigned char sequencia, unsigned char tipo, unsigned char paridade);
