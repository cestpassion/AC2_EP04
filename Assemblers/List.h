#ifndef ARRAY_H                 // Include guard: evita que o header seja incluído múltiplas vezes no build.
#define ARRAY_H

#include <iostream>             // Inclui streams padrão; não é usado diretamente aqui (poderia ser removido).
#include <cstring>              // Inclui funções de C p/ strings (strlen, strcpy), usadas abaixo.

// Celula
class Cell
{
    public:
    char* str;                  // Ponteiro para uma string C (buffer alocado dinamicamente).
    Cell* link;                 // Ponteiro para a próxima célula (lista encadeada simples).
    
    // Construtor
    Cell (char* x)              // Constrói a célula a partir de uma string C de entrada x.
    {
        if (x)                  // Só inicializa se x não for nulo.
        {
            this->str = NULL;   // Inicializa o ponteiro de string como nulo por segurança.
            link = NULL;        // Próxima célula começa nula.

            int n = strlen(x);  // Tamanho da string de entrada (sem contar o terminador '\0').
            this->str = new char[n+1](); // Aloca (com zero-init) n+1 chars para copiar x + '\0'.

            if (this->str)      // Se a alocação deu certo…
            {
                strcpy(this->str, x); // …copia o conteúdo de x para o buffer recém-alocado.
            }
        }
    }

    // Destrutor
    ~Cell ()
    {
        if (str)                // Se há string alocada…
        {
            delete (str);       // …libera a memória alocada para a string.
                                 // (ATENÇÃO: deveria ser delete[] pois foi alocada com new[]).
        }
        link = NULL;            // Por segurança, zera o ponteiro de link (não estritamente necessário no destrutor).
    }
};

// Lista
class List
{
    private:

    // Atributos
    Cell* head;                 // Ponteiro para a primeira célula da lista.
    Cell* tail;                 // Ponteiro para a última célula (para inserção O(1)).
    int n;                      // Contador de elementos na lista.

    void free (Cell* ptr)       // Função auxiliar recursiva para liberar a lista a partir de ptr.
    {
        if (ptr)                // Caso base: se ptr é nulo, não faz nada.
        {
            free(ptr->link);    // Chama recursivamente para o próximo nó (caminha até o fim).
            delete(ptr);        // Depois libera o nó atual (chamará ~Cell, que libera str).
        }
    }

    public:

    // Construtor
    List ()
    {
        this->head = NULL;      // Inicializa lista vazia.
        this->tail = NULL;      // Idem.
        this->n = 0;            // Contador começa em 0.
    }

    // Destrutor
    ~List ()
    {
        if (head)               // Se há elementos…
        {
            free (head);        // …libera toda a corrente encadeada a partir de head (recursão).
            head = NULL;        // Zera ponteiros por segurança.
            tail = NULL;
        }
    }

    // Quantidade de celulas
    int getSize (void)
    {
        return(n);              // Retorna o contador de elementos.
    }

    // Inserir linha
    void insert (char* x)       // Insere uma nova string x ao final da lista.
    {
        if (x)                  // Só insere se x não for nulo.
        {
            if (!head)          // Caso lista vazia…
            {
                head = new Cell(x); // Cria primeira célula.
                tail = head;        // Tail aponta p/ mesma célula.
            }
            else                // Caso lista não vazia…
            {
                tail->link = new Cell(x); // Cria célula ao final…
                tail = tail->link;        // …e move o tail para a nova última.
            }
            n++;                // Incrementa o contador de elementos.
        }
    }

    // Remover linha
    char* remove (void)         // Remove a primeira linha e retorna uma cópia dinâmica da string.
    {
        char* res = NULL;       // Ponteiro de retorno (string removida).
        if (head && tail)       // Se há pelo menos um elemento…
        {
            res = (char*)calloc(strlen(head->str)+1, sizeof(char)); // Aloca buffer p/ copiar a string removida.
            if (res)
            {
                strcpy(res, head->str); // Copia a string da célula removida para o buffer de retorno.

                Cell* tmp = head;       // Guarda a cabeça atual.
                head = head->link;      // Avança a cabeça para o próximo.
                delete(tmp);            // Libera a célula antiga (destrutor libera str).
                                      // (ATENÇÃO: tail não é atualizado se a lista ficar vazia; n também não é decrementado.)
            }
        }

        return (res);            // Retorna a cópia da string (o chamador deve liberá-la).
    }

    // Receber linha em uma posicao [p]
    char* get (int p)           // Retorna uma cópia da string na posição p (0-based). Não remove.
    {
        char* res = NULL;       // Ponteiro de retorno (string copiada).
        if (head && tail)       // Só procede se há elementos.
        {
            Cell* ptr = head;   // Começa da cabeça.
            int i = 0;

            while (ptr && i < p)// Avança até a posição desejada, se existir.
            {
                ptr = ptr->link;
                i++;
            }

            if (ptr)            // Se encontrou a célula na posição p…
            {
                res = (char*)calloc(strlen(ptr->str)+1, sizeof(char)); // Aloca buffer p/ cópia.
                if (res)
                {
                    strcpy(res, ptr->str); // Copia a string da célula para o retorno.
                }
            }
        }
        return (res);           // Retorna cópia (o chamador deve liberar).
    }
};

#endif                          // Fim do include guard.
