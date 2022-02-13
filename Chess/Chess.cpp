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
	Piece p = m_board[s.rank][s.file];
	Piece selected = m_board[m_squareSelected.rank][m_squareSelected.file];

	switch (m_state)
	{
	case SELECT:
		if (p.white == m_whitesTurn && p.type != EMPTY)
		{
			m_squareSelected = s;
			m_validMoveSquares = calculateValidMoveSquares(m_squareSelected);
			m_state = MOVE;
		}
		break;
	case MOVE:
		auto it = find_if(
			m_validMoveSquares.begin(),
			m_validMoveSquares.end(),
			[&](Move const& m) { return m.s == s; }
		);
		if (it != m_validMoveSquares.end())
		{
			bool inCheckBefore = m_inCheck[!selected.white];
			bool takes = (m_board[s.rank][s.file].type != EMPTY);
			// Move the piece
			movePiece(m_board, m_squareSelected, *it);
			m_whitesTurn = !m_whitesTurn;
			auto outcome = isGameOver();

			// Record the move
			bool check = false, checkmate = false;
			if (outcome == BLACK_CHECKMATE || outcome == WHITE_CHECKMATE)
			{
				checkmate = true;
			}
			else
			{
				check = !inCheckBefore && m_inCheck[!selected.white];
			}
			recordMove(m_squareSelected, s, selected.type, takes, check, checkmate);

			// Change states	
			m_validMoveSquares.clear();
			m_state = SELECT;
			cout << "White in check: " << m_inCheck[1] << ", Black in check: " << m_inCheck[0] << endl;
			return outcome;
		}
		
		if (p.white == m_whitesTurn && p.type != EMPTY)
		{
			m_squareSelected = s;
			m_validMoveSquares = calculateValidMoveSquares(m_squareSelected);
			m_state = MOVE;
		}
		break;
	}
	
	return OKAY;
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

const vector<Move>& ChessGame::getValidMoveSquares() const
{
	return m_validMoveSquares;
}

bool ChessGame::isWhiteInCheck() const
{
	return m_inCheck[1];
}

bool ChessGame::isBlackInCheck() const
{
	return m_inCheck[0];
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

void ChessGame::movePiece(Board& board, const Square& a, const Move& move)
{
	Piece& pieceA = board[a.rank][a.file];
	Piece& pieceB = board[move.s.rank][move.s.file];
	pieceB = pieceA;
	pieceB.moved = true;
	pieceA = Piece();

	switch (move.move)
	{
	case NORMAL:
		break;
	case QUEEN_CASTLE:
	{
		int rank = pieceB.white ? 7 : 0;
		movePiece(board, { 7, rank }, { { 5, rank }, NORMAL });
		break;
	}
	case KING_CASTLE:
	{
		int rank = pieceB.white ? 7 : 0;
		movePiece(board, { 0, rank }, { { 3, rank }, NORMAL });
		break;
	}
	case EN_PASSANT:
		int verticalDir = pieceB.white ? -1 : 1;
		board[move.s.rank - verticalDir][move.s.file] = Piece();
	}
}

vector<Move> ChessGame::calculateValidMoveSquares(const Square& s)
{
	vector<Move> validMoveSquares;
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
	auto checkAndAddValidSquare = [&](const Move& move) -> bool
	{
		if (!isSquareOnBoard(move.s))
			return false;
		auto collision = checkCollision(move.s);
		if (collision.has_value())
		{
			if (collision->white != p.white)
			{
				// Enemy piece >:(
				// Add capturable piece
				if (!hypCheck(s, move))
					validMoveSquares.push_back(move);
			}
			// Can no longer check along this line
			return false;
		}
		// Add unoccupied square
		if (!hypCheck(s, move))
			validMoveSquares.push_back(move);
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
					scanning = checkAndAddValidSquare({scan, NORMAL});
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
					checkAndAddValidSquare({scan, NORMAL});
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
				MOVE_TYPE moveType = offset == -2 ? QUEEN_CASTLE : KING_CASTLE;
				Square midSquare = s + Square(sign(offset), 0);
				Square offSquare = s + Square(offset, 0);
				// Check for collisions
				if (checkCollision(midSquare).has_value()
					|| checkCollision(offSquare).has_value())
					continue;
				// Check if castling while in check
				if (m_inCheck[p.white])
					continue;
				// Check if castling through check
				if (hypCheck(s, { midSquare, NORMAL }) || hypCheck(s, { offSquare, NORMAL }))
					continue;
				// Is able to castle
				validMoveSquares.push_back({ offSquare, moveType });
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
					checkAndAddValidSquare({ scan, NORMAL });
					scan = { s.file + y, s.rank + x };
					checkAndAddValidSquare({ scan, NORMAL });
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
			if (!checkCollision(scan).has_value() && !hypCheck(s, {scan, NORMAL}))
				validMoveSquares.push_back({ scan, NORMAL });
			if (!p.moved)
			{
				// Can move two spaces
				scan = { s.file, s.rank + 2 * verticalDir };
				if (!checkCollision(scan).has_value() && !hypCheck(s, { scan, NORMAL }))
					validMoveSquares.push_back({ scan, NORMAL });
			}
			// Attack
			for (auto i : { -1, 1 })
			{
				scan = { s.file + i, s.rank + 1 * verticalDir };
				if (!isSquareOnBoard(scan))
					continue;
				auto collision = checkCollision(scan);
				if (collision.has_value() && collision->white != p.white && !hypCheck(s, { scan, NORMAL }))
					validMoveSquares.push_back({ scan, NORMAL });
				
				// En passant
				Square start = { s.file + i, s.rank + 2 * verticalDir };
				if (!isSquareOnBoard(start))
					continue;
				Square end = { s.file + i, s.rank };
				Piece scanPiece = m_board[end.rank][end.file];
				if (m_moves.empty())
					break;
				if (m_moves.back() == (start.asNotation() + end.asNotation()))
				{
					if ((p.white != scanPiece.white) && scanPiece.type == PAWN)
					{
						validMoveSquares.push_back({ scan, EN_PASSANT });
					}
				}
			}

		}
		break;
	case ROOK:
		calculateLine(true, false);
		break;
	}

	return validMoveSquares;
}

// Simulate if there is check based on the move (doesn't actually perform the move)
bool ChessGame::hypCheck(const Square& a, const Move& move)
{
	Board boardCopy = m_board;
	movePiece(boardCopy, a, move);
	bool whiteInCheck, blackInCheck;
	isInCheck(boardCopy, whiteInCheck, blackInCheck);
	if (boardCopy[move.s.rank][move.s.file].white)
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
	if (pieceB.type != EMPTY && pieceA.white == pieceB.white)
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
		if ((absHDist > 0 && absVDist == 0) || (absVDist > 0 && absHDist == 0))
			return rookLine();
		if (absVDist == 0)
			return false;
		if (absHDist / absVDist == 1 && absHDist % absVDist == 0)
			return bishopLine();
		break;
	case BISHOP:
		if (absVDist == 0)
			return false;
		if (absHDist / absVDist == 1 && absHDist % absVDist == 0)
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

INPUT_RETURN ChessGame::isGameOver()
{
	isInCheck(m_board, m_inCheck[1], m_inCheck[0]);
	
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			const Piece& p = m_board[y][x];
			if (p.type != EMPTY && p.white == m_whitesTurn)
			{
				vector<Move> moves = calculateValidMoveSquares({ x, y });
				if (!moves.empty())
				{
					return OKAY;
				}
			}
		}
	}

	// Player has no moves
	if (m_inCheck[m_whitesTurn])
	{
		if (m_whitesTurn)
			return BLACK_CHECKMATE;
		else
			return WHITE_CHECKMATE;
	}

	// Player has no moves but isn't in check
	return STALEMATE;
}

void ChessGame::recordMove(const Square& a, const Square& b, const TYPE& pieceA, bool takes, bool check, bool checkmate)
{
	string move = "";
	switch (pieceA)
	{
	case KING:
		move += "K";
		break;
	case KNIGHT:
		move += "N";
		break;
	case BISHOP:
		move += "B";
		break;
	case QUEEN:
		move += "Q";
		break;
	case PAWN:
		break;
	case ROOK:
		move += "R";
		break;
	}

	move += a.asNotation();
	if (takes) move += "x";
	move += b.asNotation();
	if (check)
		move += "+";
	if (checkmate)
		move += "++";
	
	m_moves.push_back(move);
}
