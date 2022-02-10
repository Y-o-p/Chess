#pragma once

#include <list>
#include <string>

using namespace std;

struct Square
{
	int file = 0, rank = 0;
	
	Square() {}
	Square(const int& file, const int& rank)
	{
		this->file = file;
		this->rank = rank;
	}
	Square(const Square& copy) 
	{
		file = copy.file;
		rank = copy.rank;
	}
	bool operator ==(const Square& s)
	{
		return (file == s.file && rank == s.rank);
	}
	Square operator +(const Square& s) const
	{
		return { file + s.file, rank + s.rank };
	}

	string asNotation() const
	{
		return string(1, static_cast<char>(file + 'a')) + to_string(8 - rank);
	}
};

enum TYPE
{
	KING,
	QUEEN,
	BISHOP,
	KNIGHT,
	ROOK,
	PAWN,
	EMPTY
};

struct Piece
{
	Piece() {}

	Piece(const TYPE& type, bool white, bool moved)
	{
		this->type = type;
		this->white = white;
		this->moved = moved;
	}

	Piece(const Piece& copy)
	{
		type = copy.type;
		white = copy.white;
		moved = copy.moved;
	}

	TYPE type = EMPTY;
	bool white = true;
	bool moved = false;
};