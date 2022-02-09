/*
	Author: Jesse Rheal
	
	-Input-
	* Square (file and rank) the player has selected

	-Output-
	* Board data such as piece type and color
	* Input return such as if there's a bad move
	* List of moves
	* Pieces taken
	* Checkmate

	-How this class works-
	* Does not rely on inhertiance
	* Data types are stored as structs
	* Chess board is an abstract version of how a chess game works
	* The inner workings of how the chess game plays is hidden
	
	-How moving a piece works-
	* Input takes in a length two vector
	* If a piece is selected then the possible moves the piece can make are stored
		1. Tests if the square is inside the board
		2. Checks if the piece can make that move normally
		3. Checks if there's a piece blocking it
		4. Checks if the move puts either king in check
		5. If it gets through all stages then the square is added to a list of possible moves
	* The next input will either move the piece or select a different piece
*/

#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <cmath>
#include <array>
#include <vector>
#include <optional>

#include "Piece.h"

using namespace std;

enum STATE
{
	SELECT,
	MOVE
};

enum class INPUT_RETURN
{
	OKAY,
	IN_CHECK,
	CHECKMATE,
	INVALID_MOVE
};

typedef array<array<Piece, 8>, 8> Board;

class ChessGame
{
public:
	ChessGame();
	~ChessGame();

	INPUT_RETURN processInput(const Square& s);
	const Board& getBoard();
	void getPieceSelected(Piece& piece, Square& s);
	const vector<Square>& getValidMoveSquares() const;
	bool isWhiteInCheck() const;
	bool isBlackInCheck() const;
	bool isSquareOnBoard(const Square& s) const;

private:
	void setUpBoard(string code);
	void movePiece(Board& board, const Square& a, const Square& b);
	void calculateValidMoveSquares(const Square& s);
	bool hypCheck(const Square& a, const Square& b);
	bool isValidMove(const Board& board, const Square& a, const Square& b) const;
	void isInCheck(const Board& board, bool& whiteInCheck, bool& blackInCheck);
	
	Square m_squareSelected = Square();
	vector<Square> m_validMoveSquares;
	Board m_board = { {Piece()} };
	STATE m_state = SELECT;
	bool m_whitesTurn = true;
	bool m_whiteCanCastle = true, m_blackCanCastle = true;
	bool m_whiteInCheck = false, m_blackInCheck = false;
};