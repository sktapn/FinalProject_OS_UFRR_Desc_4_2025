# 💻 Simulador de Memória Virtual

Este projeto é um simulador de memória virtual paginada desenvolvido em Linguagem C. O programa foi criado como projeto final para a disciplina de Sistemas Operacionais (DCC403) do curso de Ciência da Computação da Universidade Federal de Roraima (UFRR).

O simulador processa traços de acesso à memória de programas reais para analisar e comparar o desempenho de três algoritmos clássicos de substituição de página: **FIFO**, **LRU** e **Random**.

## ✨ Funcionalidades

- **Simulação de Paginação:** Modela a interação entre a memória RAM e o disco.
- **Três Algoritmos de Substituição:** Implementação dos algoritmos FIFO, LRU e Random.
- **Análise de Desempenho:** Coleta e exibe estatísticas chave, como o número de *page faults* e de escritas de páginas "sujas" em disco.
- **Processamento de Logs:** Lê arquivos de log com endereços de memória hexadecimais e operações de Leitura (R) ou Escrita (W).
- **Modo Debug:** Inclui um modo de depuração opcional para visualizar o comportamento do simulador passo a passo.

## 🛠️ Tecnologias Utilizadas

- **Linguagem:** C (padrão C11)
- **Compilador:** GCC (GNU Compiler Collection)

## ⚙️ Como Compilar e Executar

### Pré-requisitos

Para compilar e executar o projeto, você precisará ter o compilador `gcc` instalado.

### Compilação

1.  **Clone o repositório:**
    ```bash
    git clone [https://github.com/sktapn/FinalProject_OS_UFRR_Desc_4_2025.git](https://github.com/sktapn/FinalProject_OS_UFRR_Desc_4_2025.git)
    ```

2.  **Navegue até a pasta do projeto:**
    ```bash
    cd FinalProject_OS_UFRR_Desc_4_2025
    ```

3.  **Compile o código:**
    O método mais simples é usar o comando `gcc` diretamente no terminal.

    ```bash
    gcc -o simvirtual simvirtual.c -Wall -Wextra
    ```

### ▶️ Como Usar

O programa é executado via linha de comando com 4 argumentos obrigatórios e 1 opcional.

**Sintaxe:**
```bash
./simvirtual <algoritmo> <arquivo_log> <tam_pagina_KB> <tam_memoria_KB> [debug]
