#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

void separar_linhas(char *linha, char *args[]) {

    char *token = strtok(linha, " \t\n");

    int i = 0;

    while(token != NULL) {
        args[i] = token;
        i++;
        token = strtok(NULL, " \t\n");
    }

    args[i] = NULL;
}

void fazer_mkdir(char *args[]) {

    if(args[1] == NULL) {
        printf("mkdir? faltando operando\n");
        return;
    }

    int result = mkdir(args[1], 0777);

    if(result != 0) {
        perror("mkdir");
    }

}

void fazer_cd(char *args[]) {

    if(args[1] == NULL) {
        printf("cd - faltando argumento");
        return;
    }

    int result = chdir(args[1]);

    if(result != 0) {
        perror("cd");
    }
}

void fazer_pwd(char *args[]) {
    
    char diretorio[1024];

    if(getcwd(diretorio, sizeof(diretorio)) != NULL) {
        printf("%s\n", diretorio);
    } else {
        perror("pwd");
    }

}

void fazer_ls(char *args) {
    struct dirent *entrada;
    DIR *diretorio;

    const char *caminho = (args[1] == NULL) ?  "." : args[1];

    diretorio = opendir(caminho);

    if(diretorio == NULL) {
        perror("ls");
        return;
    }
}

void fazer_rm(char *args) {
    if(args[1] == NULL) {
        printf("rm? faltando operaando\n");
        return;
    }

    int result = unlink(args[1]);

    if(result != 0) {
        perror("rm");
    }

}

void fazer_mv(char *args[]) {

    //precisa de origem e destino
    if(args[1] == NULL || args[2] == NULL) {
        printf("mv: faltando operando arquivo de origem ou destino");
        return;
    }

    int result = rename(args[1], args[2]);

    if(result != 0) {
        perror("mv");
    }
}
 
int main() {

    char comando[100];
    char *meuArgv[100];

    while(1) {
        printf("meu-shell$ ");

        if(fgets(comando, sizeof(comando), stdin) == NULL) {
            break;
        } 

        separar_linhas(comando, meuArgv);

        if(meuArgv[0] == NULL) {
            continue;
        }

        if(strcmp(meuArgv[0], "exit") == 0) {
            break;
        } else if(strcmp(meuArgv[0], "mkdir") == 0) {
            fazer_mkdir(meuArgv);
        } else if(strcmp(meuArgv[0], "cd") == 0) {
            fazer_cd(meuArgv);
        } else if(strcmp(meuArgv[0], "pwd") == 0) {
            fazer_pwd(meuArgv);
        } else if(strcmp(meuArgv[0], "ls") == 0) {
            fazer_ls(meuArgv);
        } else if(strcmp(meuArgv[0], "rm") == 0) {
            fazer_rm(meuArgv);
        } else if(strcmp(meuArgv[0], "mv") == 0) {
            fazer_mv(meuArgv);
        }
    }

    return 0;

}