#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#define NUM_LETRAS 26
#define ASCII_A 65
#define TAMANHO_SENHA 4

const int COPRIMES[4] = {3, 5, 7, 11};
const int SHIFTS[4] = {7, 11, 13, 17};

char* encrypt(const char* str) { //Função que codifica a palavra
    char* str_result = (char*) malloc(sizeof(char) * (TAMANHO_SENHA + 1));

    if (!str_result) {
        perror("Erro ao alocar memória");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < TAMANHO_SENHA; i++) {
        char c = str[i];

        if (c >= 'A' && c <= 'Z') {
            int val = c - ASCII_A;
            val = (val * COPRIMES[i] + SHIFTS[i]) % NUM_LETRAS;
            str_result[i] = val + ASCII_A;
        } 
        else {
            perror("Erro: String contém caracteres inválidos.");
            free(str_result);
            exit(EXIT_FAILURE);
        }
    }

    str_result[TAMANHO_SENHA] = '\0';
    return str_result;
}

//Função para decodificar a senha
void process_file(const char* filename, const char* filename_out) {

    //Criação da variáveis dos arquivos que contém as senhas e elas decodificadas
    FILE *file = fopen(filename, "r");
    FILE *fileout = fopen(filename_out, "w");

    printf("Processo PID %d: Quebrando senhas de %s\n", getpid(), filename);

    if (!file) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    char str[5];
    char linha[5];

    //Verificação das strings possíveis de AAAA a ZZZZ
    while (fgets(linha, sizeof(linha), file)) { 
        for (char c1 = 'A'; c1 <= 'Z'; c1++) {
            for (char c2 = 'A'; c2 <= 'Z'; c2++) {
                for (char c3 = 'A'; c3 <= 'Z'; c3++) {
                    for (char c4 = 'A'; c4 <= 'Z'; c4++) {
                        str[0] = c1;
                        str[1] = c2;
                        str[2] = c3;
                        str[3] = c4;
                        str[4] = '\0';

                        char* encrypted_str = encrypt(str);

                        // se codificadas correspondem às senhas, são jogadas no arquivo de saída
                        if (strcmp(encrypted_str, linha) == 0) {
                            fprintf(fileout, "%s\n", str);
                        }

                        free(encrypted_str);
                    }
                }
            }
        }
    }

    fclose(file);
    fclose(fileout);

    printf("Processo PID %d: Senhas quebradas salvas em %s\n", getpid(), filename_out);
}

int main() {
    //variáveis para auxiliar na contagem de tempo de execução do programa
    struct timeval start, end;
    double elapsed_time;

    //início da contagem
    gettimeofday(&start, NULL);

    printf("Gerando 10 processos para processar arquivos...\n");

    //criação de arrays contendo o nome dos arquivos de texto para serem jogador na função de descriptografar
    const char* filenames[] = {
        "senhas/001.txt", "senhas/002.txt", "senhas/003.txt", "senhas/004.txt", "senhas/005.txt",
        "senhas/006.txt", "senhas/007.txt", "senhas/008.txt", "senhas/009.txt", "senhas/010.txt"
    };

    const char* filenames_out[] = {
        "senhas_decriptografadas/001.txt", "senhas_decriptografadas/002.txt", "senhas_decriptografadas/003.txt", "senhas_decriptografadas/004.txt", "senhas_decriptografadas/005.txt",
        "senhas_decriptografadas/006.txt", "senhas_decriptografadas/007.txt", "senhas_decriptografadas/008.txt", "senhas_decriptografadas/009.txt", "senhas_decriptografadas/010.txt"
    };

    //numero de arquivos no total
    int num_files = sizeof(filenames) / sizeof(filenames[0]);

    //criação dos processos descendentes
    for (int i = 0; i < num_files; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // Processo filho
            process_file(filenames[i], filenames_out[i]);
            exit(0); // Termina o processo filho após processar o arquivo
        } else if (pid < 0) {
            // Erro ao criar o processo filho
            perror("Erro ao criar processo filho");
            exit(EXIT_FAILURE);
        }
    }

    // Espera todos os processos filhos terminarem
    for (int i = 0; i < num_files; i++) {
        wait(NULL);
    }

    printf("[Fim dos processos descendentes]\n");

    gettimeofday(&end, NULL);

    // Calcula o tempo total real em segundos
    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

    printf("Tempo total de execução: %.2f ms\n", elapsed_time*1000);

    return 0;
}