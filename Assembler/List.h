#ifndef ARRAY_H
#define ARRAY_H

#include <iostream>
#include <cstring>

// Celula
class Cell
{
	public:
	char* str;
	Cell* link;
	
	// Construtor
	Cell (char* x)
	{
		if (x)
		{
			this->str = NULL;
			link = NULL;

			int n = strlen(x);
			this->str = new char[n+1]();

			if (this->str)
			{
				strcpy(this->str, x);
			}
		}
	}

	// Destrutor
	~Cell ()
	{
		if (str)
		{
			delete (str);
		}
		link = NULL;
	}
};

// Lista
class List
{
	private:

	// Atributos
	Cell* head;
	Cell* tail;
	int n;

	void free (Cell* ptr)
	{
		if (ptr)
		{
			free(ptr->link);
			delete(ptr);
		}
	}

	public:

	// Construtor
	List ()
	{
		this->head = NULL;
		this->tail = NULL;
		this->n = 0;
	}

	// Destrutor
	~List ()
	{
		if (head)
		{
			free (head);
			head = NULL;
			tail = NULL;
		}
	}

	// Quantidade de celulas
	int getSize (void)
	{
		return(n);
	}

	// Inserir linha
	void insert (char* x)
	{
		if (x)
		{
			if (!head)
			{
				head = new Cell(x);
				tail = head;
			}
			else
			{
				tail->link = new Cell(x);
				tail = tail->link;
			}
			n++;
		}
	}

	// Remover linha
	char* remove (void)
	{
		char* res = NULL;
		if (head && tail)
		{
			res = (char*)calloc(strlen(head->str)+1, sizeof(char));
			if (res)
			{
				strcpy(res, head->str);

				Cell* tmp = head;
				head = head->link;
				delete(tmp);
			}
		}

		return (res);
	}

	// Receber linha em uma posicao [p]
	char* get (int p)
	{
		char* res = NULL;
		if (head && tail)
		{
			Cell* ptr = head;
			int i = 0;

			while (ptr && i < p)
			{
				ptr = ptr->link;
				i++;
			}

			if (ptr)
			{
				res = (char*)calloc(strlen(ptr->str)+1, sizeof(char));
				if (res)
				{
					strcpy(res, ptr->str);
				}
			}
		}
		return (res);
	}
};

#endif
