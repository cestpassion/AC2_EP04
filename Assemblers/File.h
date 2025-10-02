#ifndef FILE_H                  // Início do include guard: evita múltiplas inclusões do mesmo header no build.
#define FILE_H

#include <iostream>            // Importa streams padrão (std::cout, std::cerr), não usados diretamente aqui, mas comuns para debug.
#include <fstream>             // Importa std::fstream para ler/gravar arquivos.
#include "List.h"              // Header da lista encadeada usada para armazenar linhas (entrada/saída).

class File                     // Declaração da classe File: encapsula leitura/escrita de List a partir/para arquivo.
{
    private:
    
    // Atributos
    const char* filename;      // Ponteiro para o nome/caminho do arquivo-alvo (não gerencia memória; supõe-se que o literal/ponteiro viva tempo suficiente).

    public:

    // Construtor
    File (const char* filename) // Construtor recebe um caminho de arquivo.
    {
        if (filename)           // Checa ponteiro não-nulo.
        {
            this->filename = filename; // Armazena o ponteiro passado; não faz cópia profunda (atenção: se for buffer temporário, risco de dangling pointer).
        }
    }

    // Ler lista de um arquivo
    List* read (void)           // Lê o arquivo linha a linha e devolve uma List* com essas linhas.
    {
        List* res = new List(); // Cria lista vazia onde as linhas lidas serão inseridas.

        std::fstream fs (filename, std::ios::in); // Abre o arquivo em modo leitura (input).

        if (fs)                 // Verifica se a abertura foi bem-sucedida.
        {
            std::string line = "";              // Buffer temporário para cada linha lida.
            while (std::getline(fs, line) )     // Lê o arquivo até EOF, uma linha por iteração (descarta o '\n').
            {
                res->insert(line.data());       // Insere o conteúdo da linha na lista.
                                              // CUIDADO: line.data() retorna ponteiro para buffer interno da std::string.
                                              // A List deve copiar o conteúdo, senão esse ponteiro fica inválido na próxima iteração.
            }
            fs.close();                         // Fecha o arquivo explicitamente (destrutor também fecharia, mas ok).
        }
        return (res);          // Retorna a lista (pode vir vazia se o arquivo não existe/não abriu).
    }

    // Gravar lista em um arquivo
    void write (List* list)     // Grava o conteúdo de uma List* num arquivo texto, uma linha por linha.
    {
        if (list)               // Prossegue apenas se a lista é válida.
        {
            std::fstream fs (filename, std::ios::out); // Abre o arquivo em modo escrita (sobrescreve o arquivo).

            if (fs)             // Verifica se a abertura foi bem-sucedida.
            {
                int n = list->getSize();        // Obtém o tamanho da lista (quantidade total de nós/linhas).

                for (int i = 0; i < n-1; i++)   // Escreve as primeiras (n-1) linhas, cada uma seguida de '\n'.
                {
                    fs << list->remove();       // Remove o primeiro elemento da lista e escreve seu conteúdo.
                                               // ATENÇÃO: remove() está alterando (esvaziando) a lista; após write, a lista ficará vazia.
                    fs << "\n";                 // Nova linha após cada linha (exceto a última, ver abaixo).
                }

                // Adicionar espaco no fim
                char* last = list->remove();    // Remove a última linha restante da lista (se n>0).
                strcat(last, " ");              // Acrescenta um espaço ao final da string (CUIDADO: pode causar overflow se não houver buffer sobrando).
                fs << last;                     // Escreve a última linha (sem '\n', apenas com o espaço).

                if (last)                       // Se remove() retornou ponteiro alocado dinamicamente…
                {
                    delete (last);              // …libera a memória. (Supõe que a string foi alocada com new[], mas aqui é delete simples — risco se não casar o método de alocação.)
                }

                fs.close();                     // Fecha o arquivo explicitamente.
            }
        }
    }
};

#endif                           // Fim do include guard
