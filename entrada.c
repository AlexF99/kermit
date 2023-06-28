#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <glob.h>

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

    entrada_t *entrada = malloc(sizeof(entrada_t *));
    char *line = NULL;
    size_t size = 200;

    char pattern[200];
    glob_t paths;
    int result;

    if (getline(&line, &size, stdin) == -1)
        printf("No line\n");
    else
        printf("line: %s\n", line);

    line[strlen(line) - 1] = '\0';

    entrada->params = malloc(10 * sizeof(char *));
    for (int i = 0; i < 10; i++)
        entrada->params[i] = malloc(50 * sizeof(char));

    char *comando = strtok(line, " ");

    strcpy(pattern, line + strlen(comando) + 1);
    result = glob(pattern, 0, NULL, &paths);

    if (result != 0)
    {
        entrada->params = malloc(10 * sizeof(char *));
        char **aux = malloc(10 * sizeof(char *));
        for (int i = 0; i < 10; i++)
        {
            entrada->params[i] = malloc(200 * sizeof(char));
            aux[i] = malloc(200 * sizeof(char));
        }

        char *ptr = strtok(pattern, " ");
        int num_entradas = 0;
        aux[num_entradas++] = ptr;

        while (ptr != NULL)
        {
            ptr = strtok(NULL, " ");
            if (ptr != NULL)
                aux[num_entradas++] = ptr;
        }
        entrada->num_params = num_entradas;
        printf("%d\n", num_entradas);
        for (int i = 0; i < num_entradas; i++)
        {
            if (aux != NULL && strlen(aux[i]) > 0)
                strcpy(entrada->params[i], aux[i]);
        }
    }
    else
    {
        entrada->params = malloc(paths.gl_pathc * sizeof(char *));
        for (int i = 0; i < paths.gl_pathc; i++)
            entrada->params[i] = malloc(200 * sizeof(char));
        for (size_t i = 0; i < paths.gl_pathc; ++i)
        {
            entrada->num_params++;
            strcpy(entrada->params[i], paths.gl_pathv[i]);
        }

        globfree(&paths);
    }

    if (strcmp("cd", comando) == 0)
    {
        entrada->comando = CD;
        cd_local(entrada->params[0]);
    }
    else if (strcmp("backup", comando) == 0)
        entrada->comando = BACKUP;
    else if (strcmp("recupera", comando) == 0)
        entrada->comando = RECUPERA;
    else if (strcmp("bkpdir", comando) == 0)
        entrada->comando = BKPDIR;
    else if (strcmp("verifica", comando) == 0)
        printf("verifica\n");
    else
        fprintf(stderr, "ERRO: comando invalido!\n\n");

    return entrada;
}
