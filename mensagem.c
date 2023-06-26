#include "mensagem.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MARCADOR_INICIO 0x7e

mensagem_t *cria_mensagem(unsigned char tamanho, unsigned char sequencia, unsigned char tipo, unsigned char *dados)
{
    mensagem_t *msg = malloc(sizeof(mensagem_t));
    msg->dados = calloc(sizeof(unsigned char) * tamanho, tamanho);

    if (!msg->dados)
    {
        perror("Erro ao alocar área de dados!");
        exit(1);
    }

    if (dados)
        memcpy(msg->dados, dados, tamanho);

    msg->inicio = MARCADOR_INICIO;
    msg->tamanho = tamanho;
    msg->sequencia = sequencia;
    msg->tipo = tipo;

    return msg;
}

void print_byte(unsigned char c, int tam)
{
    for (int i = 0; i < tam; i++)
        printf("%d", !!((c << i) & 0x80));
    printf("\n");
}

unsigned char paridade_byte(unsigned char *check)
{
    unsigned char result = 0;
    for (int i = 0; i < 8; i++)
    {
        unsigned char bit_a = (check[0] >> i) & 1;
        unsigned char bit_b = (check[1] >> i) & 1;
        unsigned char bit_parity = bit_a ^ bit_b;

        result |= (bit_parity << i);
    }
    return result;
}

unsigned char *empacota_mensagem(mensagem_t *msg)
{
    unsigned char *check_paridade = malloc(sizeof(unsigned char) * 2);
    unsigned char *pct_mensagem = malloc(sizeof(unsigned char) * 67);
    unsigned char aux_e;
    unsigned char aux_d;

    memcpy(pct_mensagem, &msg->inicio, sizeof(unsigned char));

    // Monta tamanho / sequencia
    aux_e = msg->tamanho;
    aux_d = msg->sequencia;
    aux_e = aux_e << 2;
    aux_d = aux_d >> 4;
    aux_e = aux_e | aux_d;

    memcpy(check_paridade, &aux_e, sizeof(unsigned char));

    memcpy(pct_mensagem + 1, &aux_e, sizeof(unsigned char));

    // Monta sequencia / tipo
    aux_e = msg->sequencia;
    aux_d = msg->tipo;
    aux_e = aux_e << 4;
    aux_e = aux_e | aux_d;

    memcpy(check_paridade + 1, &aux_e, sizeof(unsigned char));

    memcpy(pct_mensagem + 2, &aux_e, sizeof(unsigned char));
    memcpy(pct_mensagem + 3, msg->dados, sizeof(unsigned char) * msg->tamanho);

    // monta paridade
    unsigned char p = paridade_byte(check_paridade);
    memcpy(&(msg->paridade), &p, sizeof(unsigned char));
    memcpy(pct_mensagem + 3 + msg->tamanho, &msg->paridade, sizeof(unsigned char));

    free(check_paridade);
    return pct_mensagem;
}

int desempacota_mensagem(unsigned char *pacote, mensagem_t **msg)
{
    unsigned char inicio_mensagem = (unsigned char)*pacote;

    if (inicio_mensagem != INICIO_MSG)
    {
        *msg = cria_mensagem(0, 0, LIXO_1, 0);
        return 0;
    }

    unsigned char tamanho;
    unsigned char sequencia;
    unsigned char tipo;
    unsigned char paridade;
    unsigned char aux_d, aux_e;
    unsigned char check_paridade[2];

    // Desmonta tamanho / sequencia / tipo
    aux_e = *(pacote + 1);
    aux_d = *(pacote + 2);

    tamanho = aux_e >> 2;
    sequencia = ((aux_e << 4) & 0b00111111) | (aux_d >> 4);
    tipo = aux_d & 0b00001111;
    paridade = *(pacote + 3 + tamanho);

    check_paridade[0] = aux_e;
    check_paridade[1] = aux_d;

    *msg = cria_mensagem(tamanho, sequencia, tipo, (pacote + 3));
    if (paridade != paridade_byte(check_paridade))
    {
        // vai gerar um NACK
        printf("erro de paridade!!\n");
        return -1;
    }
    return 0;
}

void imprime_mensagem(mensagem_t *msg)
{
    if (!msg)
        return;

    printf("Marcador de inicio: %x\nTamanho: %x\nSequencia: %x\nTipo: %x\nParidade: %x\n\n", msg->inicio, msg->tamanho, msg->sequencia, msg->tipo, msg->paridade);
}

void envia_mensagem(mensagem_t *msg, unsigned char *buffer, int socket)
{
    unsigned char *pacote = empacota_mensagem(msg);
    memcpy(buffer, pacote, sizeof(unsigned char) * 67);
    send(socket, buffer, sizeof(unsigned char) * 67, 0);
    free(pacote);
}

void destroi_mensagem(mensagem_t *msg)
{
    if (!msg)
        return;

    free(msg->dados);
    free(msg);
    msg = NULL;
}