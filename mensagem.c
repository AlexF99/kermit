#include "mensagem.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MARCADOR_INICIO 0x7e

mensagem_t * cria_mensagem(unsigned char tamanho, unsigned char sequencia, unsigned char tipo, unsigned char paridade)
{
    mensagem_t *msg = malloc(sizeof(mensagem_t));
    msg->dados = malloc(sizeof(unsigned char) * tamanho);

    if (!msg->dados)
    {
        perror("Erro ao alocar área de dados!");
        exit(1);
    }

    msg->inicio = MARCADOR_INICIO;
    msg->tamanho = tamanho;
    msg->sequencia = sequencia;
    msg->tipo = tipo;
    msg->paridade = paridade;

    return msg;
}

void print_byte(unsigned char c, int tam)
{
    for (int i = 0; i < tam; i++)
        printf("%d", !!((c << i) & 0x80));
    printf("\n");
}

unsigned char * empacota_mensagem(mensagem_t * msg)
{
    unsigned char * pct_mensagem = malloc(sizeof(unsigned char) * 68);
    unsigned char aux_e;
    unsigned char aux_d;

    char * dados = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    msg->dados = (unsigned char *)dados;

    memcpy(pct_mensagem, &msg->inicio, sizeof(unsigned char));

    //Monta tamanho / sequencia
    aux_e = msg->tamanho;
    aux_d = msg->sequencia;
    aux_e = aux_e << 2;
    aux_d = aux_d >> 4;
    aux_e = aux_e | aux_d;

    memcpy(pct_mensagem + 1, &aux_e, sizeof(unsigned char));

    print_byte(aux_e, 8);

    //Monta sequencia / tipo
    aux_e = msg->sequencia;
    aux_d = msg->tipo;
    aux_e = aux_e << 4;
    aux_e = aux_e | aux_d;
    
    memcpy(pct_mensagem + 2, &aux_e, sizeof(unsigned char));
    memcpy(pct_mensagem + 3, msg->dados, sizeof(unsigned char) * 39);

    print_byte(aux_e, 8);
    printf("%s\n", pct_mensagem);

    unsigned char aux;
    for (int i = 0; i < 68; i++)
    {
        aux = *(pct_mensagem+i);
        printf("%c ", aux);
    }
}
