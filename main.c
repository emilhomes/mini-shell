#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h> //Necessario para open()
#define TAM_BUFFER 4096 //Tamanho do buffer para leitura
#define TAM_BUFFER_CP 4096 //Tamanho do buffer para cópia
#define STDOUT_FILENO 1
// Limites para o SORT
#define MAX_LINES 1000
#define MAX_LINE_LEN 256
#define TAM_BUFFER_SORT (MAX_LINES * MAX_LINE_LEN)

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
        printf("mkdir: faltando operando\n");
        return;
    }

    int result = mkdir(args[1], 0777);

    if(result != 0) {
        perror("mkdir");
    }

}

void fazer_cd(char *args[]) {

    char *caminho;

    if(args[1] == NULL) {
        caminho = getenv("HOME");
        if(caminho == NULL) {
            printf("cd: impossivel encontrar diretorio home\n");
            return;
        }
    } else {
        caminho = args[1];
    }

    if(chdir(caminho) != 0) {
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

void fazer_ls(char *args[]) {
    struct dirent *entrada;
    DIR *diretorio;
    const char *caminho = "."; 

    if(args[1] != NULL) {
        if(args[1][0] == '-') {
            if(args[2] != NULL) {
                caminho = args[2];
            } else {
                caminho = "."; 
            }
        } else {
            caminho = args[1];
        }
    }

    diretorio = opendir(caminho);

    if(diretorio == NULL) {
        perror("ls");
        return;
    }

    while((entrada = readdir(diretorio)) != NULL) {
        printf("%s\n", entrada->d_name);
    }

    closedir(diretorio);
}

void fazer_rm(char *args[]) {
    if(args[1] == NULL) {
        printf("rm: faltando operaando\n");
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

void fazer_rmdir(char *args[]) {
    if (args[1] == NULL) {
        printf("rmdir: faltando operando\n");
        return;
    }

    int result = rmdir(args[1]);

    if (result != 0) {
        perror("rmdir");
    }
}

void fazer_cat(char *args[]) {
    if (args[1] == NULL) {
        printf("cat: faltando operando\n");
        return;
    }

    int fd;
    ssize_t bytes_lidos;
    char buffer[TAM_BUFFER];

    // Abre o arquivo somente para leitura.
    fd = open(args[1], O_RDONLY);

    if (fd == -1) {
        perror("cat");
        return;
    }

    // Ler o conteúdo do arquivo em blocos e escrevê-lo na saída padrão (stdout)
    while ((bytes_lidos = read(fd, buffer, TAM_BUFFER)) > 0) {
        if (write(STDOUT_FILENO, buffer, bytes_lidos) != bytes_lidos) {
            perror("cat: erro ao escrever");
            break;
        }
    }

    // Verificar se houve erro durante a leitura
    if (bytes_lidos == -1) {
        perror("cat: erro ao ler");
    }

    // Fechar o arquivo
    close(fd);
}
 
void fazer_cp(char *args[]) {
    if (args[1] == NULL || args[2] == NULL) {
        printf("cp: faltando operando arquivo de origem ou destino\n");
        return;
    }

    int fd_origem, fd_destino;
    ssize_t bytes_lidos;
    char buffer[TAM_BUFFER_CP];

    // Abrir o arquivo de origem para leitura
    fd_origem = open(args[1], O_RDONLY);
    if (fd_origem == -1) {
        perror("cp: origem");
        return;
    }

    // Abrir o arquivo de destino para escrita.
    fd_destino = open(args[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_destino == -1) {
        perror("cp: destino");
        close(fd_origem);
        return;
    }

    // Ler da origem e escrever no destino em blocos
    while ((bytes_lidos = read(fd_origem, buffer, TAM_BUFFER_CP)) > 0) {
        if (write(fd_destino, buffer, bytes_lidos) != bytes_lidos) {
            perror("cp: erro ao escrever");
            close(fd_origem);
            close(fd_destino);
            return;
        }
    }

    // Verificar se houve erro durante a leitura
    if (bytes_lidos == -1) {
        perror("cp: erro ao ler");
    }

    //Fechar ambos os arquivos
    close(fd_origem);
    close(fd_destino);
}

// Função de comparação para qsort (ordem alfabética)
int comparar_linhas(const void *a, const void *b) {

    return strcmp(*(const char **)a, *(const char **)b);
}

void fazer_sort(char *args[]) {
    if (args[1] == NULL) {
        printf("sort: faltando operando\n");
        printf("Uso: sort <arquivo>\n");
        return;
    }

    int fd;
    ssize_t bytes_lidos;
    // buffer para armazenar todo o conteúdo do arquivo
    char *buffer_total;
    // vetor para armazenar ponteiros para o início de cada linha
    char *linhas[MAX_LINES];
    int num_linhas = 0;

    // Alocar buffer para o conteúdo do arquivo
    buffer_total = (char *)malloc(TAM_BUFFER_SORT);
    if (buffer_total == NULL) {
        perror("sort: falha ao alocar memoria");
        return;
    }

    // 1. Abrir o arquivo para leitura
    fd = open(args[1], O_RDONLY);
    if (fd == -1) {
        perror("sort");
        free(buffer_total);
        return;
    }

    bytes_lidos = read(fd, buffer_total, TAM_BUFFER_SORT - 1);
    close(fd);

    if (bytes_lidos == -1) {
        perror("sort: erro ao ler");
        free(buffer_total);
        return;
    }
    
    buffer_total[bytes_lidos] = '\0';

    char *token = buffer_total;
    
    // A primeira linha começa no início do buffer
    linhas[num_linhas++] = token;

    
    while (*token != '\0' && num_linhas < MAX_LINES) {
        if (*token == '\n') {
            *token = '\0'; // Termina a string da linha atual
            

            if (*(token + 1) != '\0') {
                linhas[num_linhas++] = (token + 1);
            }
        }
        token++;
    }
    qsort(linhas, num_linhas, sizeof(char *), comparar_linhas);

    // 5. Imprimir as linhas ordenadas na saída padrão
    for (int i = 0; i < num_linhas; i++) {
        write(STDOUT_FILENO, linhas[i], strlen(linhas[i]));
        write(STDOUT_FILENO, "\n", 1);
    }

    // Liberar a memória alocada
    free(buffer_total);
}

void fazer_grep(char *args[]) {
    pid_t pid;
    int status;

    // Cria um novo processo (fork)
    pid = fork();

    if (pid < 0) {
        // Erro ao criar o processo
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Processo filho
        
        // O caminho completo para o executável do grep
        const char *caminho_grep = "/bin/grep";

        if (execve(caminho_grep, args, NULL) == -1) {
            // execve só retorna se houver um erro
            perror("execve (grep)");
            // Se falhar, o processo filho deve terminar
            exit(EXIT_FAILURE);
        }
    } else {
        
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
        }
        
        // Opcional: Verificar se o filho terminou normalmente
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            // O processo filho (grep) terminou com erro
            fprintf(stderr, "grep: O comando terminou com status de erro %d\n", WEXITSTATUS(status));
        }
    }
}

int main() {

    char comando[100];
    char *meuArgv[100];

    while(1) {
        //Iniciando o shell
        printf("meu-shell$ ");

        //Fazendo a leitura inicial do comando 
        //Juntamente com a verificação, para saber
        //se o comando não é nulo
        if(fgets(comando, sizeof(comando), stdin) == NULL) {
            break;
        } 

        //Chamando função de separar as linhas
        separar_linhas(comando, meuArgv);

        //Verificação para saber se o primeiro argumento
        //passado, não é nulo
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
        } else if(strcmp(meuArgv[0], "rmdir") == 0) {
            fazer_rmdir(meuArgv);
        } else if(strcmp(meuArgv[0], "cat") == 0) {
            fazer_cat(meuArgv);
        } else if(strcmp(meuArgv[0], "cp") == 0) {
            fazer_cp(meuArgv);
        } else if(strcmp(meuArgv[0], "sort") == 0) {
            fazer_sort(meuArgv);
        } else if(strcmp(meuArgv[0], "grep") == 0) {
            fazer_grep(meuArgv);
        } else {
            printf("meu-shell: comando nao encontrado: %s\n", meuArgv[0]);
        }
    }

    return 0;

}