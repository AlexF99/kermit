#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>

#define BLUE "\x1b[34m"
#define WHITE "\x1b[37m"

int valida_arqs(char *params)
{
    DIR *dp = opendir("./");
    if (!dp)
    {
        fprintf(stderr, "./ nao pode ser aberto\n");
        return -1;
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

void get_entrada()
{
    printf(BLUE "\noperacoes:\ncd local: cd\nbackup: backup <nome arquivos ou regex>\n");
    printf("recuperar backup: recupera <nome arquivos ou regex>\n");
    printf("define dir p/ backup: bkpdir <caminho>\nverifica backup: verifica <nome arquivo>\n\n:" WHITE);
    char comando[50];
    char params[100];
    char ch;

    for (int i = 0; (ch = getc(stdin)) != ' ';)
    {
        comando[i] = ch;
        comando[++i] = '\0';
    }
    for (int i = 0; (ch = getc(stdin)) != '\n';)
    {
        params[i] = ch;
        params[++i] = '\0';
    }

    if (strcmp("cd", comando) == 0)
        cd_local(params);
    else if (strcmp("backup", comando) == 0)
        valida_arqs(params);
    else if (strcmp("recupera", comando) == 0)
        printf("recupera\n");
    else if (strcmp("bkpdir", comando) == 0)
        printf("bkpdir\n");
    else if (strcmp("verifica", comando) == 0)
        printf("verifica\n");
    else
    {
        comando[0] = '\0';
        params[0] = '\0';
        fprintf(stderr, "ERRO: comando invalido!\n\n");
    }
}