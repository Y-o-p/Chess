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
			movePiece(m_board, m_squareSelected, s);
			m_validMoveSquares.clear();
			m_whitesTurn = !m_whitesTurn;
			m_state = SELECT;
			break;
		}
		
		if (p.white == m_whitesTurn && p.type != EMPTY)
		{
			m_squareSelected = s;
			calculateValidMoveSquares(m_squareSelected);
			m_state = MOVE;
		}
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

void ChessGame::movePiece(Board& board, const Square& a, const Square& b)
{
	Piece& pieceA = board[a.rank][a.file];
	Piece& pieceB = board[b.rank][b.file];
	pieceB = pieceA;
	pieceB.moved = true;
	pieceA = Piece();

	// Is it a castle?
	if (pieceB.type == KING && abs(b.file - a.file) == 2)
	{
		int rank = pieceB.white ? 7 : 0;
		// Queen side or king side castle?
		if (b.file == 6)
		{
			movePiece(board, { 7, rank }, { 5, rank });
		}
		else if (b.file == 2)
		{
			movePiece(board, { 0, rank }, { 3, rank });
		}
	}
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

		return {};
	};
	// Checks for collision and adds it to list and returns false if can't continue because either
	// out of bounds or ran into a piece
	auto checkAndAddValidSquare = [=](const Square& sToCheck) -> bool
	{
		if (!isSquareOnBoard(sToCheck))
			return false;
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
			m_validMoveSquares.push_back(sToCheck);
		return true;
	};
	auto calculateLine = [=](bool horizontal, bool diagonal)
	{
		vector<int> moveDirs = { -1, 1 };
		// If it's not a bishop: add a horizontal component
		if (!(diagonal && !horizontal))
			moveDirs.push_back(0);

		for (int firstDir : moveDirs)
		{
			for (int secondDir : moveDirs)
			{
				// If it's a rook: restrict the diagonal
				if (horizontal && !diagonal)
					if (abs(firstDir) == abs(secondDir))
						continue;
				Square scan = s;
				bool scanning = true;
				while (scanning)
				{
					scan.file += firstDir;
					scan.rank += secondDir;

					// Keep scanning this line until we go out of bounds or run into another piece
					scanning = checkAndAddValidSquare(scan);
				}
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

			// Castling
			if (p.moved)
				break;
			vector<int> castleOffsets;
			// Check if rooks have moved
			int rank = p.white ? 7 : 0;
			if (m_board[rank][0].type == ROOK && 
				m_board[rank][0].white == p.white &&
				m_board[rank][0].moved == false)
				castleOffsets.push_back(-2);
			if (m_board[rank][7].type == ROOK &&
				m_board[rank][7].white == p.white &&
				m_board[rank][7].moved == false)
				castleOffsets.push_back(2);

			for (auto offset : castleOffsets)
			{
				Square midSquare = s + Square(sign(offset), 0);
				Square offSquare = s + Square(offset, 0);
				// Check for collisions
				if (checkCollision(midSquare).has_value()
					|| checkCollision(offSquare).has_value())
					continue;
				// Check if castling while in check
				if ((p.white && m_whiteInCheck) || (!p.white && m_blackInCheck))
					continue;
				// Check if castling through check
				if (hypCheck(s, midSquare) || hypCheck(s, offSquare))
					continue;
				// Is able to castle
				m_validMoveSquares.push_back(offSquare);
			}
		}
		break;
	case KNIGHT:
		{
			array<int, 2> dirsA = { -2, 2 };
			array<int, 2> dirsB = { -1, 1 };
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
	movePiece(boardCopy, a, b);
	bool whiteInCheck, blackInCheck;
	isInCheck(boardCopy, whiteInCheck, blackInCheck);
	if (boardCopy[b.rank][b.file].white)
		return whiteInCheck;
	else
		return blackInCheck;
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
