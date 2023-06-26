enum COMANDOS
{
    CD = 1,
    BACKUP = 2,
    RECUPERA = 3,
    BKPDIR = 4,
    VERIFICA = 5,
    MD5_E = 6
};

typedef struct entrada_t
{
    int comando;
    char **params;
    int num_params;
} entrada_t;

int cd_local(char *path);

entrada_t *get_entrada();