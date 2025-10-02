#ifndef FILE_H
#define FILE_H

#include <iostream>
#include <fstream>
#include "List.h"

class File
{
	private:
	
	// Atributos
	const char* filename;

	public:

	// Construtor
	File (const char* filename)
	{
		if (filename)
		{
			this->filename = filename;
		}
	}

	// Ler lista de um arquivo
	List* read (void)
	{
		List* res = new List();

		std::fstream fs (filename, std::ios::in);

		if (fs)
		{
			std::string line = "";
			while (std::getline(fs, line) )
			{
				res->insert(line.data());
			}
			fs.close();
		}
		return (res);
	}

	// Gravar lista em um arquivo
	void write (List* list)
	{
		if (list)
		{
			std::fstream fs (filename, std::ios::out);

			if (fs)
			{
				int n = list->getSize();

				for (int i = 0; i < n-1; i++)
				{
					fs << list->remove();
					fs << "\n";
				}

				// Adicionar espaco no fim
				char* last = list->remove();
				strcat(last, " ");
				fs << last;

				if (last)
				{
					delete (last);
				}

				fs.close();
			}
		}
	}
};

#endif
