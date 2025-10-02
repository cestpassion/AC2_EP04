#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "File.h"
#include "iostream"
#include "cmath"
#include <cstdint>
#include <cstring>
#include <cctype>
#include <cstdlib>   // calloc, free

#define byte uint8_t
#define X mem[0]
#define Y mem[1]
#define W mem[2]

class Assembler
{
private:
    // Atributos
    List* input;  // Linhas de entrada
    List* output; // Linhas de saida
    byte* mem;
    bool tick;    // W atualizado

    // Helpers
    static inline bool isHexDigit(char c) {
        return ((c >= '0' && c <= '9') ||
                (c >= 'A' && c <= 'F') ||
                (c >= 'a' && c <= 'f'));
    }
    static inline byte hexValue(char c) {
        if (c >= '0' && c <= '9') return (byte)(c - '0');
        if (c >= 'A' && c <= 'F') return (byte)(10 + (c - 'A'));
        if (c >= 'a' && c <= 'f') return (byte)(10 + (c - 'a'));
        return 0;
    }

public:
    // Construtor
    Assembler (const char* filename)
    : input(nullptr), output(nullptr), mem(nullptr), tick(false)
    {
        mem = new byte[3](); // X,Y,W = 0
        for (int i = 0; i < 3; i++) mem[i] = 0x0;

        if (filename) {
            File* f = new File(filename);
            input = f->read();   // pega List*
            // (se quiser: delete f;  // depende da sua implementação)
        }
    }

    // Destrutor
    ~Assembler ()
    {
        if (input)  { delete input;  input  = nullptr; }
        if (output) { delete output; output = nullptr; }
        if (mem)    { delete[] mem;  mem    = nullptr; }
    }

    // Gravar saida no arquivo
    void Export (const char* filename)
    {
        if (!filename) return;
        File* f = new File(filename);
        f->write(output);
        // delete f; // opcional
    }

    // Checar se e numero (decimal)
    static inline bool isNumber (char c)
    {
        return (c >= '0' && c <= '9');
    }

    // Checar se e letra
    static inline bool isLetter (char c)
    {
        return ( (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') );
    }

    // Compara strings
    static inline bool match (char* x, const char* y)
    {
        return (std::strcmp(x, y) == 0);
    }

    // Converter substring DECIMAL para byte (mantido por compat)
    static inline byte parseByte (char* str, int s, int f)
    {
        byte res = 0x0;
        if (str && f > s) {
            for (int i = s; i < f; i++) {
                if (!isNumber(str[i])) break;
                res = (byte)(res * 10 + (byte)(str[i] - '0'));
            }
        }
        return (res);
    }

    // Reconhecer instrucao — TABELA 2025/2
    byte getInstruction (char* str, int s, int f)
    {
        byte res = 0x0;
        if (str && f > s) {
            char* min = (char*)std::calloc((size_t)(f - s + 1), sizeof(char));
            for (int i = s; i < f; i++) min[i - s] = str[i];

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

            std::free(min);
        }
        return (res);
    }

    // Linhas individuais
    void assemble (char* line)
    {
        if (!line) return;

        // Ignorar linhas de controle, vazias e comentários simples
        if (std::strstr(line, "inicio") || std::strstr(line, "fim")) return;
        int n = (int)std::strlen(line);
        int start = 0;
        while (start < n && std::isspace((unsigned char)line[start])) start++;
        if (start >= n) return;
        if (line[start] == ';' || line[start] == '#') return; // comentário

        bool op = false;
        byte* cursor = nullptr;

        char c = line[start];
        if (c == 'X' || c == 'x' || c == 'A' || c == 'a')      cursor = &X;
        else if (c == 'Y' || c == 'y' || c == 'B' || c == 'b') cursor = &Y;
        else if (c == 'W' || c == 'w') { cursor = &W; op = true; }
        else return; // linha desconhecida

        // Encontrar '=' com segurança
        int i = start + 1;
        while (i < n && line[i] != '=') i++;
        if (i >= n) return; // linha sem '='
        i++; // pular '='

        // pular espaços
        while (i < n && std::isspace((unsigned char)line[i])) i++;
        if (i >= n) return;

        if (!op) {
            // X= ou Y= → ler 1 dígito HEX após '='
            // pular lixo até achar dígito HEX
            while (i < n && !isHexDigit(line[i])) i++;
            if (i >= n) return;
            *cursor = (hexValue(line[i]) & 0xF);
        } else {
            // W= → mnemônico (somente letras)
            while (i < n && !isLetter(line[i])) i++;
            int s = i;
            while (i < n && isLetter(line[i])) i++;
            int f = i;
            if (f > s) {
                *cursor = getInstruction(line, s, f);
                tick = true;
            }
        }
    }

    // Todas as linhas
    void assemble (void)
    {
        if (!input) {
            std::cerr << "ERRO: entrada vazia ou nao lida.\n";
            return;
        }

        output = new List();

        // Nota: muitas implementações dessa List usam 1..N-1; para garantir,
        // vamos de 0..getSize()-1 e checar ponteiro nulo.
        const int total = input->getSize();
        for (int i = 0; i < total; i++)
        {
            char* line_in = input->get(i);
            if (!line_in) continue;

            assemble(line_in);

            if (tick) {
                char* line_out = new char[9]();
                std::snprintf(line_out, 8, "%1X%1X%1X", X, Y, W);
                output->insert(line_out);
                // não deletar line_out aqui (provável shallow-copy na List)
                tick = false;
            }

            // não deletar line_in aqui — a List deve gerenciar
        }
    }
};

#endif

int main (int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "ERRO: Parametros invalidos!\nForneca o arquivo de entrada como parametro.\n";
        return 1;
    }

    if (argv && argv[1])
    {
        char* infile = argv[1];
        int n = (int)std::strlen(infile);

        // Cria nome de saída com .hex
        char* outfile = (char*)std::calloc((size_t)n + 5, sizeof(char));
        int i = 0;
        while (i < n && infile[i] != '.') {
            outfile[i] = infile[i];
            i++;
        }
        std::strcat(outfile, ".hex");

        Assembler* as = new Assembler(infile);
        as->assemble();
        as->Export(outfile);
		std::cout << "Gerado: " << outfile << std::endl;

        if (outfile) std::free(outfile);
        delete as;
    }
    return 0;
}

