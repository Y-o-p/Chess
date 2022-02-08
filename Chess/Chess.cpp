#include "Chess.h"

ChessGame::ChessGame()
{
	setUpBoard("0R00 0N10 0B20 0Q30 0K40 0B50 0N60 0R70 0P01 0P11 0P21 0P31 0P41 0P51 0P61 0P71 1R07 1N17 1B27 1Q37 1K47 1B57 1N67 1R77 1P06 1P16 1P26 1P36 1P46 1P56 1P66 1P76");
}

ChessGame::~ChessGame()
{
}

INPUT_RETURN ChessGame::processInput(const Square& s)
{
	Piece& p = m_board[s.rank][s.file];

	switch (m_state)
	{
	case SELECT:
		if (p.white == m_whitesTurn && p.type != EMPTY)
		{
			m_squareSelected = s;
			calculateValidMoveSquares(m_squareSelected);
			m_state = MOVE;
		}
		break;
	case MOVE:
		auto it = find(m_validMoveSquares.begin(), m_validMoveSquares.end(), s);
		if (it != m_validMoveSquares.end())
		{
			if (movePiece(m_board, m_squareSelected, s))
			{
				m_whitesTurn = !m_whitesTurn;
			}
		}
		m_state = SELECT;
		break;
	}
	
	cout << m_whitesTurn << endl;
	return INPUT_RETURN::OKAY;
}

const Board& ChessGame::getBoard()
{
	return m_board;
}

void ChessGame::getPieceSelected(Piece& piece, Square& s)
{
	piece = m_board[s.rank][s.file];
	s = m_squareSelected;
}

const vector<Square>& ChessGame::getValidMoveSquares() const
{
	return m_validMoveSquares;
}

bool ChessGame::isWhiteInCheck() const
{
	return m_whiteInCheck;
}

bool ChessGame::isBlackInCheck() const
{
	return m_blackInCheck;
}

bool ChessGame::isSquareOnBoard(const Square& s) const
{
	return (s.file < 8 && s.file >= 0 && s.rank < 8 && s.rank >= 0);
}

void ChessGame::setUpBoard(string setUpCode)
{
	stringstream ss = stringstream(setUpCode);
	
	while (!ss.eof())
	{
		string code;
		ss >> code;
		bool side = code[0] - '0';
		char piece = code[1];
		char file = code[2] - '0';
		char rank = code[3] - '0';
		TYPE type;
		switch (piece)
		{
		case 'K':
			type = KING;
			break;
		case 'N':
			type = KNIGHT;
			break;
		case 'B':
			type = BISHOP;
			break;
		case 'Q':
			type = QUEEN;
			break;
		case 'P':
			type = PAWN;
			break;
		case 'R':
			type = ROOK;
			break;
		}

		m_board[rank][file] = { type, side, false };
	} 
}

/*bool ChessGame::canMovePiece(const Square& a, const Square& b)
{
	// Can this piece move as suggested?
	if (!isValidMove(m_board, a, b))
		return false;
	
	cout << "White in check: " << m_whiteInCheck << ", " << "Black in check: " << m_blackInCheck << endl;
	// Is the player moving into check?
	if ((pieceB.white && m_whiteInCheck) || (!pieceB.white && m_blackInCheck))
		return false;
	
	return true;
}*/

bool ChessGame::movePiece(Board& board, const Square& a, const Square& b, bool checkIfMovePossible)
{
	//if (checkIfMovePossible)
	//	if (!canMovePiece(a, b))
	//		return false;
	Piece& pieceA = board[a.rank][a.file];
	Piece& pieceB = board[b.rank][b.file];
	pieceA = board[a.rank][a.file];
	pieceB = board[b.rank][b.file];
	pieceB = pieceA;
	pieceB.moved = true;
	pieceA = Piece();
	return true;
	// Is the player trying to castle?
}

template <class T>
int sign(const T& x)
{
	if (x < 0)
	{
		return -1;
	}
	return 1;
}

bool ChessGame::isValidMove(const Board& board, const Square& a, const Square& b) const
{
	const Piece& pieceA = board[a.rank][a.file];
	const Piece& pieceB = board[b.rank][b.file];
	bool pieceBSide = board[b.rank][b.file].white;
	int hDist = b.file - a.file, vDist = b.rank - a.rank;
	int absHDist = abs(hDist), absVDist = abs(vDist);
	int dir = pieceA.white ? -1 : 1;
	// Can it occupy the square?
	if (!(pieceA.white == !pieceB.white || pieceB.type == EMPTY))
		return false;

	auto bishopLine = [&board, &absHDist, &a, &hDist, &vDist]()
	{
		for (int i = 1; i < absHDist; i++)
		{
			if (board[a.rank + i * sign(vDist)][a.file + i * sign(hDist)].type != EMPTY)
			{
				return false;
			}
		}
		return true;
	};
	auto rookLine = [&board, &absHDist, &a, &absVDist, &hDist, &vDist, &pieceA]()
	{
		for (int i = 1; i < absHDist; i++)
		{
			if (board[a.rank][a.file + i * sign(hDist)].type != EMPTY)
			{
				return false;
			}
		}
		for (int i = 1; i < absVDist; i++)
		{
			if (board[a.rank + i * sign(vDist)][a.file].type != EMPTY)
			{
				return false;
			}
		}
		return true;
	};

	switch (pieceA.type)
	{
	case KING:
		if (absHDist <= 1 && absVDist <= 1)
		{
			return true;
		}
		break;
	case QUEEN:
		if (absHDist / (absVDist == 0 ? 1 : absVDist) == 1)
		{
			return bishopLine();
		}
		if ((absHDist > 0 && absVDist == 0) || (absVDist > 0 && absHDist == 0))
		{
			return rookLine();
		}
		break;
	case BISHOP:
		if (absHDist / (absVDist == 0 ? 1 : absVDist) == 1)
			return bishopLine();
		break;
	case KNIGHT:
		if ((absHDist == 2 && absVDist == 1) || (absVDist == 2 && absHDist == 1)) 
			return true;
		break;
	case ROOK:
		if ((absHDist > 0 && absVDist == 0) || (absVDist > 0 && absHDist == 0)) 
			return rookLine();
		break;
	case PAWN:
		if (pieceA.moved)
		{
			if (vDist == 1 * dir && hDist == 0)
			{
				return true;
			}
		}
		else if ((vDist == 2 * dir || vDist == 1 * dir) && hDist == 0)
		{
			return true;
		}
		if (vDist == 1 * dir && absHDist == 1)
		{
			return true;
		}
		break;
	default:
		break;
	}

	return false;
}

void ChessGame::calculateValidMoveSquares(const Square& s)
{
	m_validMoveSquares.clear();
	auto& board = m_board;
	const Piece& p = board[s.rank][s.file];

	auto checkCollision = [=](const Square& sToCheck) -> optional<Piece>
	{
		// Check if collision with piece
		Piece piece = board[sToCheck.rank][sToCheck.file];
		if (piece.type != EMPTY)
		{
			return make_optional(piece);
		}
	};
	// Checks for collision and adds it to list and returns false if can't continue because either
	// out of bounds or ran into a piece
	auto checkAndAddValidSquare = [=](const Square& sToCheck) -> bool
	{
		if (isSquareOnBoard(sToCheck))
		{
			auto collision = checkCollision(sToCheck);
			if (collision.has_value())
			{
				if (collision->white != p.white)
				{
					// Enemy piece >:(
					// Add capturable piece
					if (!hypCheck(s, sToCheck))
						m_validMoveSquares.push_back(sToCheck);
				}
				// Can no longer check along this line
				return false;
			}
			// Add unoccupied square
			if (!hypCheck(s, sToCheck))
			{
				m_validMoveSquares.push_back(sToCheck);
			}
			return true;
		}
		return false;
	};
	auto calculateLine = [=](bool horizontal, bool diagonal)
	{
		int dir = 0, dirEnd = 0;
		if (diagonal)
		{
			dir = 0;
			dirEnd = 4;
		}
		else if (horizontal)
		{
			dir = 4;
			dirEnd = 8;
		}
		if (diagonal && horizontal)
		{
			dir = 0;
			dirEnd = 8;
		}
		for (; dir < dirEnd; dir++)
		{
			int horizontalDir, verticalDir;
			switch (dir)
			{
			// Top left
			case 0:
				horizontalDir = -1;
				verticalDir = -1;
				break;
			// Top right
			case 1:
				horizontalDir = 1;
				verticalDir = -1;
				break;
			// Bottom right
			case 2:
				horizontalDir = 1;
				verticalDir = 1;
				break;
			// Bottom left
			case 3:
				horizontalDir = -1;
				verticalDir = 1;
				break;
			// Left
			case 4:
				horizontalDir = -1;
				verticalDir = 0;
				break;
			// Top
			case 5:
				horizontalDir = 0;
				verticalDir = -1;
				break;
			// Right
			case 6:
				horizontalDir = 1;
				verticalDir = 0;
				break;
			// Bottom
			case 7:
				horizontalDir = 0;
				verticalDir = -1;
				break;

			}

			Square scan = s;
			int i = 1;
			bool scanning = true;
			while (scanning)
			{
				scan.file += i * horizontalDir;
				scan.rank += i * verticalDir;

				i++;

				scanning = checkAndAddValidSquare(scan);
			}
		}
	};

	switch (p.type)
	{
	case KING:
		{
			array<int, 3> dirs = { -1, 0, 1 };
			for (int y : dirs)
			{
				for (int x : dirs)
				{
					// All possible combinations of direction to get surrounding squares
					Square scan = { s.file + x, s.rank + y };
					checkAndAddValidSquare(scan);
				}
			}
		}
		break;
	case KNIGHT:
		{
			array<int, 3> dirsA = { -2, 2 };
			array<int, 3> dirsB = { -1, 1 };
			for (int y : dirsA)
			{
				for (int x : dirsB)
				{
					Square scan = { s.file + x, s.rank + y };
					checkAndAddValidSquare(scan);
					scan = { s.file + y, s.rank + x };
					checkAndAddValidSquare(scan);
				}
			}
		}
		break;
	case BISHOP:
		calculateLine(false, true);
		break;
	case QUEEN:
		calculateLine(true, true);
		break;
	case PAWN:
		{
			int verticalDir = p.white ? -1 : 1;
			Square scan = { s.file, s.rank + 1 * verticalDir };
			checkAndAddValidSquare(scan);
			if (!p.moved)
			{
				// Can move two spaces
				Square scan = { s.file, s.rank + 2 * verticalDir };
				checkAndAddValidSquare(scan);
			}
		}
		break;
	case ROOK:
		calculateLine(true, false);
		break;
	}
}

// Simulate if there is check based on the move (doesn't actually perform the move)
bool ChessGame::hypCheck(const Square& a, const Square& b)
{
	Board boardCopy = m_board;
	movePiece(boardCopy, a, b, false);
	bool whiteInCheck, blackInCheck;
	isInCheck(boardCopy, whiteInCheck, blackInCheck);
	if (boardCopy[b.rank][b.file].white)
	{
		if (whiteInCheck)
			return true;
		else
			return false;
	}
	else
	{
		if (blackInCheck)
			return true;
		else
			return false;
	}
}

void ChessGame::isInCheck(const Board& board, bool& whiteInCheck, bool& blackInCheck)
{
	// Find the king
	Square whiteKingPos, blackKingPos;
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (board[y][x].type == KING)
			{
				if (board[y][x].white)
					whiteKingPos = { x, y };
				else
					blackKingPos = { x, y };
			}
		}
	}
	
	whiteInCheck = false;
	blackInCheck = false;
	// Check if they're in check
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (board[y][x].type != EMPTY)
			{
				if (board[y][x].white)
				{
					if (isValidMove(board, { x, y }, blackKingPos))
						blackInCheck = true;
				}
				else
				{
					if (isValidMove(board, { x, y }, whiteKingPos))
						whiteInCheck = true;
				}
			}
		}
	}
}
