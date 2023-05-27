#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>

#include "entrada.h"

#define BLUE "\x1b[34m"
#define WHITE "\x1b[37m"

int valida_arqs(char *params)
{
    DIR *dp = opendir("./");
    if (!dp)
    {
        fprintf(stderr, "diretorio nao pode ser aberto\n");
        return 1;
    }
    struct dirent *ep;
    char arq[100];
    int j = 0;
    for (int i = 0; params[i] != '\0';)
    {
        arq[j] = params[i];
        arq[++j] = '\0';
        ++i;
        if (params[i] == ' ' || params[i] == '\0')
        {
            int valid = 0;
            while ((ep = readdir(dp)))
            {
                if (strcmp(ep->d_name, arq) == 0)
                    valid = 1;
            }
            if (valid == 0)
                printf("arquivo \"%s\" invalido\n", arq);
            j = 0;
            if (params[i] != '\0')
                ++i;
            rewinddir(dp);
        }
    }

    closedir(dp);
    return 0;
}

int cd_local(char *path)
{
    char s[100];
    chdir(path);
    printf("%s\n", getcwd(s, 100));
    return 0;
}

entrada_t *get_entrada()
{
    printf(BLUE "\noperacoes:\ncd local: cd\nbackup: backup <nome arquivos ou regex>\n");
    printf("recuperar backup: recupera <nome arquivos ou regex>\n");
    printf("define dir p/ backup: bkpdir <caminho>\nverifica backup: verifica <nome arquivo>\n\n:" WHITE);

    char comando[50];
    entrada_t *entrada = malloc(sizeof(entrada_t *));
    char *line = NULL;
    size_t size = 200;

    if (getline(&line, &size, stdin) == -1)
        printf("No line\n");
    else
        printf("line: %s\n", line);

    line[strlen(line) - 1] = '\0';

    char **aux = malloc(10 * sizeof(char *));
    entrada->params = malloc(10 * sizeof(char *));
    for (int i = 0; i < 10; i++)
        aux[i] = malloc(200 * sizeof(char));

    char *ptr = strtok(line, " ");
    int num_entradas = 0;
    aux[num_entradas++] = ptr;

    while (ptr != NULL)
    {
        ptr = strtok(NULL, " ");
        if (ptr != NULL)
            aux[num_entradas++] = ptr;
    }

    strcpy(comando, aux[0]);
    for (int i = 1; i < num_entradas; i++)
    {
        if (aux != NULL && strlen(aux[i]) > 0)
            entrada->params[i - 1] = aux[i];
    }

    entrada->num_params = num_entradas - 1;

    if (strcmp("cd", comando) == 0)
    {
        cd_local(entrada->params[0]);
        entrada = NULL;
    }
    else if (strcmp("backup", comando) == 0)
        entrada->comando = BACKUP;
    else if (strcmp("recupera", comando) == 0)
        printf("recupera\n");
    else if (strcmp("bkpdir", comando) == 0)
        printf("bkpdir\n");
    else if (strcmp("verifica", comando) == 0)
        printf("verifica\n");
    else
        fprintf(stderr, "ERRO: comando invalido!\n\n");

    printf("comando: %d\n", entrada->comando);
    for (int i = 0; i < num_entradas - 1; i++)
        printf("arq %d: %s\n", i, entrada->params[i]);

    return entrada;
    // todo: free aux
}
