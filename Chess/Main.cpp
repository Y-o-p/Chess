#include <iostream>
#include <iomanip>
#include <chrono>
#include <optional>

#include <SDL_main.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "Chess.h"

using namespace std;

//void renderTexture(SDL_Texture* texture, int)

int main(int argc, char* argv[])
{
	// SDL initialization
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1) return -1;
	if (TTF_Init() == -1) return -1;
	
	SDL_Window* window;
	SDL_Renderer* renderer;
	const int PADDING = 64;
	int panelWidth = 300;
	int chessPieceSize = 128;
	int width = chessPieceSize * 8 + panelWidth + PADDING * 2, height = chessPieceSize * 8 + PADDING * 2;
	SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_RESIZABLE, &window, &renderer);

	if (window == NULL)
	{
		cout << "Could not create the window: " << SDL_GetError() << endl;
		return 1;
	}

	// Fonts and textures
	TTF_Font* font = TTF_OpenFont("times new roman.ttf", 24);
	auto title = TTF_RenderText_Solid(font, "Chess", { 255, 255, 255, 255 });
	SDL_Texture* chessSpriteSheet = IMG_LoadTexture(renderer, "pieces_1.png");
	SDL_Texture* moveable = IMG_LoadTexture(renderer, "moveable.png");
	SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, title);

	// Game related variables
	vector<Move> possibleMoves;
	ChessGame game;
	auto board = game.getBoard();

	// Main game loop
	bool polling = true;
	bool update = true;
	while (polling)
	{
		// Poll events
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				polling = false;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (e.button.button == SDL_BUTTON_LEFT)
				{
					update = true;
				}
				break;
			case SDL_WINDOWEVENT:
				SDL_GetWindowSize(window, &width, &height);
				chessPieceSize = (height - PADDING * 2) / 8;
				panelWidth = width - PADDING - chessPieceSize * 8 - 2 * PADDING;
				update = true;
			default:
				break;
			}
		}

		// Game logic
		if (update)
		{
			int x, y;
			SDL_GetMouseState(&x, &y);
			x -= PADDING;
			y -= PADDING;
			x /= chessPieceSize;
			y /= chessPieceSize;
			
			if (x < 8 && x >= 0 && y < 8 && y >= 0)
			{
				auto outcome = game.processInput({ x, y });
				switch (outcome)
				{
				case WHITE_CHECKMATE:
					cout << "White wins!" << endl;
					break;
				case BLACK_CHECKMATE:
					cout << "Black wins!" << endl;
					break;
				case STALEMATE:
					cout << "Stalemate!" << endl;
					break;
				}
			}

			board = game.getBoard();
			possibleMoves = game.getValidMoveSquares();
			
			SDL_SetRenderDrawColor(renderer, 140, 225, 159, 255);
			SDL_RenderClear(renderer);

			for (int y = 0; y < 8; y++)
			{
				for (int x = 0; x < 8; x++)
				{
					if ((x + y * 9) % 2 == 0)
					{
						// White
						SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					}
					else
					{
						// Black
						SDL_SetRenderDrawColor(renderer, 140, 159, 225, 255);
					}
					SDL_Rect square = {
						x * chessPieceSize + PADDING,
						y * chessPieceSize + PADDING,
						chessPieceSize,
						chessPieceSize
					};
					
					// Render the square
					SDL_RenderFillRect(renderer, &square);
				
					// Render the piece
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
				SDL_Rect destRect = { 
					possibleMove.s.file * chessPieceSize + chessPieceSize / 4 + PADDING,
					possibleMove.s.rank * chessPieceSize + chessPieceSize / 4 + PADDING,
					chessPieceSize / 2,
					chessPieceSize / 2};
				SDL_RenderCopy(renderer, moveable, &srcRect, &destRect);
			}

			// Sidebar
			SDL_Rect sidebar = {
				PADDING + 8 * chessPieceSize,
				PADDING,
				panelWidth + PADDING,
				8 * chessPieceSize
			};
			SDL_RenderFillRect(renderer, &sidebar);

			SDL_Rect srcRect = { 0, 0, 128, 128};
			SDL_Rect destRect = { chessPieceSize * 8, 0, 128, 64 };
			SDL_RenderCopy(renderer, titleTexture, NULL, &destRect);

			SDL_RenderPresent(renderer);
			update = false;
		}
	}

	TTF_CloseFont(font);
	TTF_Quit();
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}