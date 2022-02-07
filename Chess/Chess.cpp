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
			m_state = MOVE;
		}
		break;
	case MOVE:
		if (movePiece(m_squareSelected, s))
		{
			m_whitesTurn = !m_whitesTurn;
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

bool ChessGame::isWhiteInCheck() const
{
	return m_whiteInCheck;
}

bool ChessGame::isBlackInCheck() const
{
	return m_blackInCheck;
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

bool ChessGame::movePiece(const Square& a, const Square& b)
{
	// Can this piece move as suggested?
	if (!isValidMove(m_board, a, b))
		return false;
	Board boardCopy = m_board;
	Piece* pieceA = &boardCopy[a.rank][a.file];
	Piece* pieceB = &boardCopy[b.rank][b.file];
	*pieceB = *pieceA;
	pieceB->moved = true;
	*pieceA = Piece();
	isInCheck(boardCopy);
	cout << "White in check: " << m_whiteInCheck << ", " << "Black in check: " << m_blackInCheck << endl;
	// Is the player moving into check?
	if ((pieceB->white && m_whiteInCheck) || (!pieceB->white && m_blackInCheck))
		return false;
	pieceA = &m_board[a.rank][a.file];
	pieceB = &m_board[b.rank][b.file];
	*pieceB = *pieceA;
	pieceB->moved = true;
	*pieceA = Piece();
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

void ChessGame::isInCheck(const Board& board)
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
	
	m_whiteInCheck = false;
	m_blackInCheck = false;
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
						m_blackInCheck = true;
				}
				else
				{
					if (isValidMove(board, { x, y }, whiteKingPos))
						m_whiteInCheck = true;
				}
			}
		}
	}
}
