#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "File.h"
#include "iostream"
#include "cmath"
#include <cstdint>

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

	public:

	// Construtor
	Assembler (const char* filename) 
	{
		input = NULL;
		output = NULL;
		tick = false;

		mem = new byte[3](); // mem[0] = X; mem[1] = Y; mem[2] = W;

		for (int i = 0; i < 3; i++)
		{
			mem[i] = 0x0;
		}

		if (filename)
		{
			File* f = new File(filename); // Le arquivo
			input = f->read(); // Recebe dados
		}
	}

	// Destrutor
	~Assembler ()
	{
		if (input)
		{
			delete(input);
			input = NULL;
		}
		if (output)
		{
			delete(output);
			output = NULL;
		}
		if (mem)
		{
			delete(mem);
			mem = NULL;
		}
	}

	// Gravar saida no arquivo
	void Export (const char* filename)
	{
		if (filename)
		{
			File* f = new File(filename);
			f->write(output);
		}
	}

	// Checar se e numero
	bool isNumber (char c)
	{
		return (c >= '0' && c <= '9');
	}

	// Checar se e letra
	bool isLetter (char c)
	{
		return ( (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') );
	}

 	// Compara strings
	bool match (char* x, const char* y)
	{
		return (strcmp(x, y) == 0);
	}

	// Converter substring para byte
	byte parseByte (char* str, int s, int f)
	{
		byte res = 0x0;
		if (str)
		{
			int e = f-s;

			for (int i = s; i < f; i++)
			{
				res += (byte)(str[i] - '0') * pow(10, f-i-1);
			}
		}
		return (res);
	}

	// Reconhecer instrucao
	byte getInstruction (char* str, int s, int f)
	{
		byte res = 0x0;
		if (str)
		{
			// Clonar substring para comparacao
			char* min = (char*)calloc(f-s+1, sizeof(char));
			for (int i = s; i < f; i++)
			{
				min[i-s] = str[i];
			}

			// Encontrar instrucao
			if (match(min, "umL")) res = 0x1;
            else if (match(min, "zeroL")) res = 0x0;
            else if (match(min, "AonB")) res = 0x2;     // A + B'
            else if (match(min, "nAonB")) res = 0x3;    // A' + B'
            else if (match(min, "AeBn")) res = 0x4;     // (A.B)'
            else if (match(min, "nB")) res = 0x5;
            else if (match(min, "nA")) res = 0x6;
            else if (match(min, "nAxnB")) res = 0x7;    // A'⊕B'
            else if (match(min, "AxB")) res = 0x8;      // A⊕B
            else if (match(min, "copiaA")) res = 0x9;   // A
            else if (match(min, "copiaB")) res = 0xA;   // B
            else if (match(min, "AeB")) res = 0xB;      // A.B
            else if (match(min, "AenB")) res = 0xC;     // A.B'
            else if (match(min, "nAeB")) res = 0xD;     // A'.B
            else if (match(min, "AoB")) res = 0xE;      // A+B
            else if (match(min, "nAeBn")) res = 0xF;    // (A'.B)'


			delete (min);
		}
		return (res);
	}

	// Linhas individuais
	void assemble (char* line)
	{
		bool op = false;

		if (line)
		{
			op = false;
			int n = strlen(line);

			byte* cursor = NULL; // Selecionador

			char c = line[0]; // Primeiro valor

			if (c == 'X' || c == 'x' || c == 'A' || c == 'a')
			{
				cursor = &X; // Seleciona X
			}
			else if (c == 'Y' || c == 'y' || c == 'B' || c == 'b')
			{
				cursor = &Y; // Seleciona Y
			}
			else if (c == 'W' || c == 'w')
			{
				cursor = &W; // Seleciona W
				op = true; // E operacao
			}

			int s = 0; // Inicio substring
			int f = 0; // Fim substring

			int i = 1; // Index

			while (line[i] != '=') // Pular '='
			{
				i++;
			}

			if (!op) // Nao e operador
			{
				while (!isNumber(line[i])) // Encontrar inicio do numero
				{
					i++;
				}
				s = i;
	
				while (isNumber(line[i])) // Encontrar fim do numero
				{
					i++;
				}
				f = i;

				*cursor = parseByte(line, s, f); // Converte substring e grava no cursor
			}
			else // E operador
			{
				while (!isLetter(line[i])) // Encontrar inicio da instrucao
				{
					i++;
				}
				s = i;

				while (isLetter(line[i])) // Encontrar fim da instrucao
				{
					i++;
				}
				f = i;

				*cursor = getInstruction (line, s, f); // Reconhecer instrucao
				tick = true; // W foi atualizado
			}
		}
	}

 	// Todas as linhas
	void assemble (void)
	{
		int n = input->getSize() - 1; // Quantidade de linhas

		output = new List(); // Lista de saida

		for (int i = 1; i < n; i++)
		{
			char* line_in = input->get(i); // Selecionar linha

			assemble(line_in); // Interpretar linha

			if (tick) // Se W foi atualizado
			{
				char* line_out = new char[9](); // Linha de saida
				snprintf (line_out, 8, "%1X%1X%1X", X,Y,W); 	 // Converte para Hexa
				output->insert(line_out); 						 // Grava linha na saida
				tick = false; 									 // dismarca tick

				delete (line_out);
			}
			delete (line_in); // Limpa memoria
		}
	}

};

#endif

int main (int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "ERRO: Parametros invalidos!\nForneca o arquivo de entrada como parametro.\n"; 
	}
	else if (argv && argv[1])
	{
		// Nome do arquivo de entrada
		char* infile = argv[1];
		int n = (int)strlen(infile);
	 	
		// Nome arquivo de saida
		char* outfile = (char*)calloc(n+1,sizeof(char));

		int i = 0;
		while (i < n && infile[i] != '.') // Copia nome
		{
			outfile[i] = infile[i];
			i++;
		}
		strcat(outfile, ".hex"); // Adiciona .hex

		// Recebe arquivo de entrada
		Assembler* as = new Assembler(infile);
 
	 	// Converter entrada para Hexa
		as->assemble();

		// Gravar arquivo de saida
		as->Export(outfile);

		// Limpar memoria
		if (outfile)
		{
			delete(outfile);
		}
		if (as)
		{
			delete(as);
		}
	}
	return (0);
}