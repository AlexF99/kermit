#include "mensagem.h"
#define INICIO 0b01111110

mensagem_t *cria_mensagem(unsigned char tamanho, unsigned char sequencia, unsigned char tipo, unsigned char paridade)
{
    mensagem_t *msg = malloc(sizeof(mensagem_t));
    msg->dados = malloc(sizeof(tamanho));
    msg->inicio = INICIO;
    msg->tamanho = tamanho;
    msg->sequencia = sequencia;
    msg->tipo = tipo;
    msg->paridade = paridade;

    return msg;
}
