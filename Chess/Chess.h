#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <cmath>
#include <array>
using namespace std;

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

enum STATE
{
	/*WHITE_SELECT,
	WHITE_ATTACK,
	WHITE_CHECK,
	WHITE_CHECKMATE,
	BLACK_SELECT,
	BLACK_ATTACK,
	BLACK_CHECK,
	BLACK_CHECKMATE*/
	SELECT,
	MOVE
};

enum class INPUT_RETURN
{
	OKAY,
	IN_CHECK,
	INVALID_MOVE
};

struct Piece
{
	TYPE type = EMPTY;
	bool white = true;
	bool moved = false;

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
};

typedef array<array<Piece, 8>, 8> Board;

struct Square
{
	int file = 0, rank = 0;
};

class ChessGame
{
public:
	ChessGame();
	~ChessGame();

	INPUT_RETURN processInput(const Square& s);
	const Board& getBoard();
	bool isWhiteInCheck() const;
	bool isBlackInCheck() const;

private:
	void setUpBoard(string code);
	bool movePiece(const Square& a, const Square& b);
	bool isValidMove(const Board& board, const Square& a, const Square& b) const;
	void isInCheck(const Board& board);
	Square m_squareSelected = Square();
	Board m_board = { {Piece()} };
	STATE m_state = SELECT;
	bool m_whitesTurn = true;
	bool m_whiteCanCastle = true, m_blackCanCastle = true;
	bool m_whiteInCheck = false, m_blackInCheck = false;
};