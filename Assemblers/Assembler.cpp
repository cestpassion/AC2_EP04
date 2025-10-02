#ifndef ASSEMBLER_H                   // Include guard: impede múltiplas inclusões do mesmo header.
#define ASSEMBLER_H

#include "File.h"                     // Declaração da classe File (I/O de arquivos e List).
#include "iostream"                   // std::cout / std::cerr (usados em mensagens).
#include "cmath"                      // pow etc. (aqui não é estritamente necessário).
#include <cstdint>                    // tipos fixos (uint8_t).
#include <cstring>                    // funções C de string (strlen, strcmp, strcat…).
#include <cctype>                     // classificação de caracteres (isspace, isalpha…).
#include <cstdlib>   // calloc, free    // Alocação/Desalocação estilo C (calloc/free).

#define byte uint8_t                  // Alias pra byte = uint8_t.
#define X mem[0]                      // Macros para acessar registradores no array mem[].
#define Y mem[1]
#define W mem[2]

class Assembler
{
private:
    // Atributos
    List* input;                      // Lista de linhas lidas do arquivo de entrada (.ULA).
    List* output;                     // Lista de linhas a serem gravadas no arquivo de saída (.hex).
    byte* mem;                        // Pequena “memória”: X, Y e W (3 bytes).
    bool tick;                        // Marca se W foi atualizado em uma linha (gera saída .hex).

    // Helpers
    static inline bool isHexDigit(char c) {       // Testa se c é dígito HEX (0–9, A–F, a–f).
        return ((c >= '0' && c <= '9') ||
                (c >= 'A' && c <= 'F') ||
                (c >= 'a' && c <= 'f'));
    }
    static inline byte hexValue(char c) {         // Converte char HEX para valor 0..15.
        if (c >= '0' && c <= '9') return (byte)(c - '0');
        if (c >= 'A' && c <= 'F') return (byte)(10 + (c - 'A'));
        if (c >= 'a' && c <= 'f') return (byte)(10 + (c - 'a'));
        return 0;
    }

public:
    // Construtor
    Assembler (const char* filename)              // Inicializa assembler e lê o arquivo de entrada (se dado).
    : input(nullptr), output(nullptr), mem(nullptr), tick(false)
    {
        mem = new byte[3]();                      // Aloca 3 bytes zerados para X,Y,W.
        for (int i = 0; i < 3; i++) mem[i] = 0x0; // Redundante, mas garante zera.

        if (filename) {
            File* f = new File(filename);         // Cria um File para o caminho fornecido.
            input = f->read();   // pega List*   // Lê o arquivo: retorna uma List* com as linhas.
            // (se quiser: delete f;  // depende da sua implementação)  // File é não usado depois; pode liberar.
        }
    }

    // Destrutor
    ~Assembler ()
    {
        if (input)  { delete input;  input  = nullptr; } // Libera lista de entrada.
        if (output) { delete output; output = nullptr; } // Libera lista de saída (se houver).
        if (mem)    { delete[] mem;  mem    = nullptr; } // Libera o array de 3 bytes (delete[] correto).
    }

    // Gravar saida no arquivo
    void Export (const char* filename)            // Escreve a List* output no arquivo filename.
    {
        if (!filename) return;                    // Se não há destino, sai.
        File* f = new File(filename);             // Abre “File” para o destino.
        f->write(output);                         // Grava a lista de saída (.hex).
        // delete f; // opcional                   // Pode liberar f aqui; não é essencial em programa curto.
    }

    // Checar se e numero (decimal) — mantido por compatibilidade (não usado para X/Y em HEX).
    static inline bool isNumber (char c)
    {
        return (c >= '0' && c <= '9');
    }

    // Checar se e letra (A..Z ou a..z).
    static inline bool isLetter (char c)
    {
        return ( (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') );
    }

    // Compara strings (C-strings): retorna true se iguais.
    static inline bool match (char* x, const char* y)
    {
        return (std::strcmp(x, y) == 0);
    }

    // Converter substring DECIMAL para byte (reserva do 2025/1; aqui quase não usado).
    static inline byte parseByte (char* str, int s, int f)
    {
        byte res = 0x0;
        if (str && f > s) {
            for (int i = s; i < f; i++) {
                if (!isNumber(str[i])) break;     // Para se achar algo que não é dígito.
                res = (byte)(res * 10 + (byte)(str[i] - '0'));
            }
        }
        return (res);
    }

    // Reconhecer instrucao — TABELA 2025/2 (mnemônico → opcode 0x0..0xF).
    byte getInstruction (char* str, int s, int f)
    {
        byte res = 0x0;
        if (str && f > s) {
            // Copia a substring [s,f) para um buffer novo terminado em '\0' (para comparar com strcmp).
            char* min = (char*)std::calloc((size_t)(f - s + 1), sizeof(char));
            for (int i = s; i < f; i++) min[i - s] = str[i];

            // Mapeamento oficial 2025/2
            if      (match(min, "umL"))     res = 0x1;
            else if (match(min, "zeroL"))   res = 0x0;
            else if (match(min, "AonB"))    res = 0x2; // A + B'
            else if (match(min, "nAonB"))   res = 0x3; // A' + B'
            else if (match(min, "AeBn"))    res = 0x4; // (A.B)'
            else if (match(min, "nB"))      res = 0x5;
            else if (match(min, "nA"))      res = 0x6;
            else if (match(min, "nAxnB"))   res = 0x7; // A'⊕B'
            else if (match(min, "AxB"))     res = 0x8; // A⊕B
            else if (match(min, "copiaA"))  res = 0x9; // A
            else if (match(min, "copiaB"))  res = 0xA; // B
            else if (match(min, "AeB"))     res = 0xB; // A.B
            else if (match(min, "AenB"))    res = 0xC; // A.B'
            else if (match(min, "nAeB"))    res = 0xD; // A'.B
            else if (match(min, "AoB"))     res = 0xE; // A+B
            else if (match(min, "nAeBn"))   res = 0xF; // (A'.B)'

            std::free(min);                 // Libera a substring alocada.
        }
        return (res);                        // Retorna o opcode (0..15).
    }

    // Linhas individuais: interpreta UMA linha (X=…; Y=…; W=…;) e atualiza X,Y,W/tick.
    void assemble (char* line)
    {
        if (!line) return;                  // Linha nula → ignora.

        // Ignora linhas de controle e vazias (ex.: "inicio:", "fim.")
        if (std::strstr(line, "inicio") || std::strstr(line, "fim")) return;
        int n = (int)std::strlen(line);     // Tamanho da linha.
        int start = 0;
        while (start < n && std::isspace((unsigned char)line[start])) start++; // Pula espaços iniciais.
        if (start >= n) return;             // Linha só de espaços → ignora.
        if (line[start] == ';' || line[start] == '#') return; // Linha de comentário simples → ignora.

        bool op = false;                    // Flag: true quando for W= (operação).
        byte* cursor = nullptr;             // Aponta para X, Y ou W conforme a linha.

        char c = line[start];               // Primeiro caractere útil da linha.
        if (c == 'X' || c == 'x' || c == 'A' || c == 'a')      cursor = &X; // Aceita X/A como sinônimo.
        else if (c == 'Y' || c == 'y' || c == 'B' || c == 'b') cursor = &Y; // Aceita Y/B como sinônimo.
        else if (c == 'W' || c == 'w') { cursor = &W; op = true; }          // W → operação.
        else return;                        // Qualquer outro prefixo → ignora.

        // Procura '=' com segurança.
        int i = start + 1;
        while (i < n && line[i] != '=') i++;
        if (i >= n) return;                 // Linha sem '=' → ignora.
        i++;                                // Pula '='.

        // Pula espaços antes do valor.
        while (i < n && std::isspace((unsigned char)line[i])) i++;
        if (i >= n) return;                 // Sem nada após '=' → ignora.

        if (!op) {
            // Caso X= ou Y=: pegar 1 dígito HEX após '=' (2025/2).
            while (i < n && !isHexDigit(line[i])) i++; // Avança até achar um dígito HEX.
            if (i >= n) return;                         // Não achou → ignora.
            *cursor = (hexValue(line[i]) & 0xF);        // Converte e salva (só 4 bits).
        } else {
            // Caso W=: ler o mnemônico (somente letras contíguas).
            while (i < n && !isLetter(line[i])) i++; // Início do mnemônico.
            int s = i;
            while (i < n && isLetter(line[i])) i++;  // Fim do mnemônico.
            int f = i;
            if (f > s) {
                *cursor = getInstruction(line, s, f); // Converte mnemônico → opcode.
                tick = true;                           // Marca que W foi atualizado (gera linha .hex).
            }
        }
    }

    // Todas as linhas: percorre input, chama assemble(line), e quando tick==true, emite uma linha no output.
    void assemble (void)
    {
        if (!input) {
            std::cerr << "ERRO: entrada vazia ou nao lida.\n";
            return;
        }

        output = new List();                // Prepara lista de saída.

        // Algumas versões de List usam índice 1..N-1; aqui usamos 0..N-1 com checagem de ponteiro.
        const int total = input->getSize();
        for (int i = 0; i < total; i++)
        {
            char* line_in = input->get(i); // Obtém cópia da i-ésima linha (o chamador deve liberar, dependendo da List).
            if (!line_in) continue;        // Se nulo, pula.

            assemble(line_in);             // Interpreta a linha (pode setar X,Y ou W/tick).

            if (tick) {                    // Se W foi atualizado nesta linha…
                char* line_out = new char[9]();                  // Reserva buffer p/ “XYZ\0”.
                std::snprintf(line_out, 8, "%1X%1X%1X", X, Y, W); // Formata 3 nibbles HEX (X,Y,W).
                output->insert(line_out);                        // Insere a linha no output (lista).
                // não deletar line_out aqui (provável shallow-copy na List) // Atenção: se a List copiar, poderia deletar.
                tick = false;                                     // Limpa o tick.
            }

            // não deletar line_in aqui — a List deve gerenciar        // Evita double-free se a List gerencia internamente.
        }
    }
};

#endif                                    // Fim do include guard.

int main (int argc, char** argv)          // Ponto de entrada do binário “assembler”.
{
    if (argc != 2)                        // Espera exatamente 1 argumento: arquivo .ULA
    {
        std::cerr << "ERRO: Parametros invalidos!\nForneca o arquivo de entrada como parametro.\n";
        return 1;
    }

    if (argv && argv[1])                  // Se veio caminho de entrada…
    {
        char* infile = argv[1];           // Caminho do .ULA de entrada.
        int n = (int)std::strlen(infile);

        // Cria nome de saída com .hex (copia tudo até o primeiro '.' e concatena ".hex").
        char* outfile = (char*)std::calloc((size_t)n + 5, sizeof(char)); // +5 cabe ".hex" e '\0'.
        int i = 0;
        while (i < n && infile[i] != '.') {
            outfile[i] = infile[i];
            i++;
        }
        std::strcat(outfile, ".hex");     // Acrescenta extensão .hex

        Assembler* as = new Assembler(infile); // Instancia o montador (lê o .ULA).
        as->assemble();                         // Converte input → output (List com linhas .hex).
        as->Export(outfile);                    // Grava o .hex no disco.
		std::cout << "Gerado: " << outfile << std::endl;  // Mensagem de feedback do caminho gerado.

        if (outfile) std::free(outfile);        // Libera o buffer alocado com calloc.
        delete as;                              // Libera o Assembler (e suas List internas).
    }
    return 0;                                   // Fim normal do programa.
}
