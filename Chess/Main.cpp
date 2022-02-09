#include <iostream>
#include <iomanip>
#include <chrono>
#include <optional>

#include <SDL_main.h>
#include <SDL.h>
#include <SDL_image.h>

#include "Chess.h"

using namespace std;

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_Window* window;
	SDL_Renderer* renderer;
	const int CHESS_PIECE_SIZE = 128;
	float width = CHESS_PIECE_SIZE * 8 + 300, height = CHESS_PIECE_SIZE * 8;
	SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);

	if (window == NULL)
	{
		cout << "Could not create the window: " << SDL_GetError() << endl;
		return 1;
	}

	SDL_Texture* chessSpriteSheet = IMG_LoadTexture(renderer, "pieces_1.png");
	SDL_Texture* moveable = IMG_LoadTexture(renderer, "moveable.png");
	vector<Square> possibleMoves;
	ChessGame game;
	auto board = game.getBoard();

	bool running = true;
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
			
			if (x < 8 && x >= 0 && y < 8 && y >=0)
				game.processInput({ x, y });

			board = game.getBoard();
			possibleMoves = game.getValidMoveSquares();
		}

		SDL_SetRenderDrawColor(renderer, 140, 225, 159, 255);
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

		// Show possible moves with the selected piece
		for (auto possibleMove : possibleMoves)
		{
			SDL_Rect srcRect = { 0, 0, 128, 128 };
			SDL_Rect destRect = { possibleMove.file * CHESS_PIECE_SIZE + CHESS_PIECE_SIZE / 4, possibleMove.rank * CHESS_PIECE_SIZE + CHESS_PIECE_SIZE / 4, CHESS_PIECE_SIZE / 2, CHESS_PIECE_SIZE / 2};
			SDL_RenderCopy(renderer, moveable, &srcRect, &destRect);
		}

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}