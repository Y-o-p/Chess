#pragma once

#include <list>

struct Square
{
	int file = 0, rank = 0;
	bool operator ==(const Square& s)
	{
		return (file == s.file && rank == s.rank);
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