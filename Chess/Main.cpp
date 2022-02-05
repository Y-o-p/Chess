#include <iostream>
#include <iomanip>
#include <chrono>
#include <optional>

#include <SDL_main.h>
#include <SDL.h>
#include <SDL_image.h>

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
	WHITE_SELECT,
	WHITE_ATTACK,
	WHITE_CHECK,
	WHITE_CHECKMATE,
	BLACK_SELECT,
	BLACK_ATTACK,
	BLACK_CHECK,
	BLACK_CHECKMATE
};

struct Piece
{
	TYPE type = EMPTY;
	bool white = true;
};

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_Window* window;
	SDL_Renderer* renderer;
	const int CHESS_PIECE_SIZE = 64;
	float width = CHESS_PIECE_SIZE * 8, height = CHESS_PIECE_SIZE * 8;
	SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);

	if (window == NULL)
	{
		cout << "Could not create the window: " << SDL_GetError() << endl;
		return 1;
	}

	SDL_Texture* chessSpriteSheet = IMG_LoadTexture(renderer, "pieces_1.png");

	Piece board[8][8] =
	{
		{{ROOK, false}, {KNIGHT, false}, {BISHOP, false },{ QUEEN, false}, {KING, false},{ BISHOP, false}, {KNIGHT, false}, {ROOK, false}},
		{{PAWN, false }, { PAWN, false}, { PAWN, false}, { PAWN, false}, { PAWN, false}, { PAWN, false}, { PAWN, false}, { PAWN, false}},
		{Piece(),Piece(),Piece(),Piece(),Piece(),Piece(),Piece(),Piece()},
		{Piece(),Piece(),Piece(),Piece(),Piece(),Piece(),Piece(),Piece()},
		{Piece(),Piece(),Piece(),Piece(),Piece(),Piece(),Piece(),Piece()},
		{Piece(),Piece(),Piece(),Piece(),Piece(),Piece(),Piece(),Piece()},
		{{PAWN}, {PAWN}, {PAWN}, {PAWN}, {PAWN}, {PAWN}, {PAWN}, {PAWN}},
		{{ROOK}, {KNIGHT}, {BISHOP}, {QUEEN}, {KING}, {BISHOP}, {KNIGHT}, {ROOK} }
	};
	Piece* pieceSelected = nullptr;

	bool running = true;
	STATE gameState = WHITE_SELECT;
	while (running)
	{
		// Poll events
		SDL_Event e;
		bool event_pressed = false;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				running = false;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (e.button.button == SDL_BUTTON_LEFT)
				{
					event_pressed = true;
				}
				break;
			default:
				break;
			}
		}

		// Game logic
		if (event_pressed)
		{
			int x, y;
			SDL_GetMouseState(&x, &y);
			cout << (x /= CHESS_PIECE_SIZE) << ", " << (y /= CHESS_PIECE_SIZE) << endl;
			Piece& p = board[y][x];

			switch (gameState)
			{
			case WHITE_SELECT:
				if (p.white && p.type != EMPTY)
				{
					pieceSelected = &p;
					gameState = WHITE_ATTACK;
				}
				break;
			case WHITE_ATTACK:
				if (p.type != EMPTY)
				{
					if (!p.white)
					{
						// If it's black's piece
						p = *pieceSelected;
						pieceSelected->type = EMPTY;
						gameState = BLACK_SELECT;
					}
				}
				else
				{
					p = *pieceSelected;
					pieceSelected->type = EMPTY;
					gameState = BLACK_SELECT;
				}
				gameState = WHITE_ATTACK;
				break;
			case WHITE_CHECK:
				break;
			case WHITE_CHECKMATE:
				break;
			case BLACK_SELECT:
				if (!p.white)
				{
					pieceSelected = &p;
				}
			case BLACK_ATTACK:
				break;
			case BLACK_CHECK:
				break;
			case BLACK_CHECKMATE:
				break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		for (int y = 0; y < 8; y++)
		{
			for (int x = 0; x < 8; x++)
			{
				if ((x + y * 9) % 2 == 0)
				{
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				}
				else
				{
					SDL_SetRenderDrawColor(renderer, 140, 159, 225, 255);
				}
				SDL_Rect square = { x * CHESS_PIECE_SIZE, y * CHESS_PIECE_SIZE, CHESS_PIECE_SIZE, CHESS_PIECE_SIZE };
				
				SDL_RenderFillRect(renderer, &square);
				
				if (board[y][x].type != EMPTY)
				{
					SDL_Rect srcRect = { board[y][x].type * 256, board[y][x].white ? 0 : 256, 256, 256 };
					SDL_RenderCopy(renderer, chessSpriteSheet, &srcRect, &square);
				}

			}
		}

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}