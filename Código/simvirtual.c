#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>

// Define constantes para algoritmos
#define FIFO 0
#define LRU 1
#define RANDOM 2

// Estrutura para representar um quadro de pagina na memoria fisica
typedef struct {
    bool ocupado;
    bool suja; // Dirty bit
    unsigned int numero_pagina;
    unsigned long long contador_lru; // Usado para encontrar a pagina menos recentemente usada
} QuadroPagina;


// Funcao para identificar o algoritmo. Converte string para inteiro (LRU, FIFO, RANDOM). Retorna -1 se for inválido.
int parseAlgoritmo(const char *alg) {
    if (strcmp(alg, "fifo") == 0) return FIFO;
    if (strcmp(alg, "lru") == 0) return LRU;
    if (strcmp(alg, "random") == 0) return RANDOM;
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc < 5 || argc > 6) {
        printf("Uso: %s <lru|fifo|random> <arquivo.log> <tam_pagina_KB> <tam_memoria_KB> [debug]\n", argv[0]);
        return 1;
    }

    // 1. Parse dos argumentos
    int algoritmo = parseAlgoritmo(argv[1]);
    if (algoritmo == -1) {
        printf("Erro: algoritmo invalido. Use 'fifo', 'lru' ou 'random'.\n");
        return 1;
    }

    char *arquivoLog = argv[2];
    int sizePg = atoi(argv[3]);
    int sizeFis = atoi(argv[4]);
    bool debug_mode = (argc == 6); // Ativa o modo debug se o 5' argumento existir

    if (sizePg < 2 || sizePg > 64) {
        printf("Erro: O tamanho de cada pagina deve estar entre 2 e 64KB.\n");
        return 1;
    }
    if (sizeFis < 128 || sizeFis > 16384) {
        printf("Erro: O tamanho da memoria deve estar entre 128 e 16384KB.\n");
        return 1;
    }
    if (sizeFis % sizePg != 0) {
        printf("Erro: O tamanho da memoria fisica deve ser multiplo do tamanho da pagina.\n");
        return 1;
    }

    // 2. Abertura do arquivo de log
    FILE *fp = fopen(arquivoLog, "r");
    if (fp == NULL) {
        perror("Erro ao abrir o arquivo de log");
        return 1;
    }

    // 3. Inicializacao da simulacao
    srand(time(NULL));

    unsigned int tamanho_pagina_bytes = sizePg * 1024;
    unsigned int s = 0, tmp = tamanho_pagina_bytes; // calcula s = log2(bytesPorPagina)
    while (tmp > 1) {
        tmp = tmp >> 1;
        s++;
    }

    int total_quadros = sizeFis / sizePg;
    QuadroPagina *memoria_fisica = (QuadroPagina *)malloc(total_quadros * sizeof(QuadroPagina));
    if (!memoria_fisica) {
        fprintf(stderr, "Erro: sem memória para alocar quadros.\n");
        fclose(fp);
        return 1;
    }

    for (int i = 0; i < total_quadros; i++) {
        memoria_fisica[i].ocupado = false;
        memoria_fisica[i].suja = false;
        memoria_fisica[i].contador_lru = 0;
    }

    long long total_acessos = 0;
    long long page_faults = 0;
    long long paginas_escritas_disco = 0;
    
    int ponteiro_fifo = 0;
    unsigned long long contador_global_lru = 0;

    if (debug_mode) {
        printf("--- MODO DEBUG ATIVADO ---\n");
        printf("Total de quadros na memoria: %d\n", total_quadros);
        printf("Bits de offset (s): %u\n\n", s);
    }

    // 4. Leitura do arquivo de acessos e logica da simulacao
    unsigned int endereco;
    char operacao;

    while (fscanf(fp, "%x %c", &endereco, &operacao) == 2) {
        // Valida operacao
        if (operacao != 'R' && operacao != 'r' && operacao != 'W' && operacao != 'w') {
            printf("Operacao invalida: %c. Use R ou W.\n", operacao);
            continue;
        }
        total_acessos++;
        contador_global_lru++;

        unsigned int numero_pagina_atual = endereco >> s;

        if (debug_mode) {
             printf("Acesso %ld: Endereco 0x%x, Op %c -> Pagina %u\n", (long)total_acessos, endereco, operacao, numero_pagina_atual);
        }

        int quadro_encontrado = -1;
        for (int i = 0; i < total_quadros; i++) {
            if (memoria_fisica[i].ocupado && memoria_fisica[i].numero_pagina == numero_pagina_atual) {
                quadro_encontrado = i;
                break;
            }
        }

        if (quadro_encontrado != -1) { // HIT
            if (debug_mode) printf(" > Resultado: HIT no quadro %d\n", quadro_encontrado);
            memoria_fisica[quadro_encontrado].contador_lru = contador_global_lru;
            
            if (operacao == 'W' || operacao == 'w') {
                memoria_fisica[quadro_encontrado].suja = true;
                 if (debug_mode) printf(" > Pagina %u marcada como suja.\n", numero_pagina_atual);
            }
        } else { // MISS
            page_faults++;

            int quadro_livre = -1;
            for (int i = 0; i < total_quadros; i++) {
                if (!memoria_fisica[i].ocupado) {
                    quadro_livre = i;
                    break;
                }
            }

            if (quadro_livre != -1) { // Existe quadro livre
                if (debug_mode) printf(" > Resultado: MISS. Carregando no quadro livre %d.\n", quadro_livre);
                memoria_fisica[quadro_livre].ocupado = true;
                memoria_fisica[quadro_livre].numero_pagina = numero_pagina_atual;
                memoria_fisica[quadro_livre].contador_lru = contador_global_lru;
                memoria_fisica[quadro_livre].suja = (operacao == 'W' || operacao == 'w');
            } else { // Memoria cheia, precisa substituir
                if (debug_mode) printf(" > Resultado: MISS. Memoria cheia -> PAGE FAULT!\n");
                int quadro_vitima = -1;
                // Algoritmos de substituicao
                if (algoritmo == FIFO) {
                    quadro_vitima = ponteiro_fifo;
                    ponteiro_fifo = (ponteiro_fifo + 1) % total_quadros;
                } 
                else if (algoritmo == LRU) {
                    unsigned long long min_lru = ULLONG_MAX;
                    for(int i = 0; i < total_quadros; i++) {
                        if (memoria_fisica[i].contador_lru < min_lru) {
                            min_lru = memoria_fisica[i].contador_lru;
                            quadro_vitima = i;
                        }
                    }
                } 
                else if (algoritmo == RANDOM) {
                    quadro_vitima = rand() % total_quadros;
                }

                if (debug_mode) printf("   > Vitima escolhida (quadro %d): pagina %u\n", quadro_vitima, memoria_fisica[quadro_vitima].numero_pagina);

                if (memoria_fisica[quadro_vitima].suja) {
                    paginas_escritas_disco++;
                    if (debug_mode) printf("   > Pagina vitima estava suja. Escrita para o disco contabilizada.\n");
                }

                memoria_fisica[quadro_vitima].numero_pagina = numero_pagina_atual;
                memoria_fisica[quadro_vitima].contador_lru = contador_global_lru;
                memoria_fisica[quadro_vitima].suja = (operacao == 'W' || operacao == 'w');
            }
        }
        if (debug_mode) printf("\n");
    }

    // 5. Imprimir relatorio final
    printf("\nExecutando o simulador...\n");
    printf("Arquivo de entrada: %s\n", arquivoLog);
    printf("Tamanho da memoria: %d KB\n", sizeFis);
    printf("Tamanho das paginas: %d KB\n", sizePg);
    printf("Tecnica de reposicao: %s\n", argv[1]);
    printf("Total de acessos a memoria: %ld\n", (long)total_acessos);
    printf("Paginas lidas: %ld\n", (long)page_faults);
    printf("Paginas escritas: %ld\n", (long)paginas_escritas_disco);

    // 6. Liberar recursos
    free(memoria_fisica);
    fclose(fp);
    
    return 0;
}
