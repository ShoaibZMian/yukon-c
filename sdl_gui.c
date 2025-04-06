#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdbool.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define CARD_WIDTH 80
#define CARD_HEIGHT 120
#define CARD_SPACING 20

// Function prototypes
void cleanup_and_exit(SDL_Window* window, SDL_Renderer* renderer, int exit_code);
void draw_card(SDL_Renderer* renderer, int x, int y, int value, int suit, bool is_hidden);
void draw_game_board(SDL_Renderer* renderer);

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Card Game GUI", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    // Enable vsync
    SDL_SetRenderVSync(renderer, 1); // 1 for vsync enabled
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Main loop flag
    bool quit = false;

    // Event handler
    SDL_Event e;

    // Main loop
    while (!quit) {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            // User requests quit
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
            // User presses a key
            else if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_ESCAPE) {
                    quit = true;
                }
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255); // Dark green background
        SDL_RenderClear(renderer);

        // Draw game board
        draw_game_board(renderer);

        // Update screen
        SDL_RenderPresent(renderer);
    }

    // Clean up and exit
    cleanup_and_exit(window, renderer, 0);
    return 0;
}

void cleanup_and_exit(SDL_Window* window, SDL_Renderer* renderer, int exit_code) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void draw_card(SDL_Renderer* renderer, int x, int y, int value, int suit, bool is_hidden) {
    // Card background (white)
    SDL_FRect card_rect = {x, y, CARD_WIDTH, CARD_HEIGHT};
    
    if (is_hidden) {
        // Draw card back (blue pattern)
        SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);
        SDL_RenderFillRect(renderer, &card_rect);
        
        // Draw pattern
        SDL_SetRenderDrawColor(renderer, 0, 0, 150, 255);
        for (int i = 0; i < CARD_WIDTH; i += 10) {
            SDL_FRect pattern_rect = {x + i, y, 5, CARD_HEIGHT};
            SDL_RenderFillRect(renderer, &pattern_rect);
        }
    } else {
        // Draw card face
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &card_rect);
        
        // Draw border
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderRect(renderer, &card_rect);
        
        // Draw suit and value
        // This is a simplified version - in a real app, you'd use textures for the suits and values
        SDL_SetRenderDrawColor(renderer, 
                              (suit == 1 || suit == 2) ? 255 : 0,  // Red for Hearts and Diamonds
                              0, 
                              (suit == 3 || suit == 4) ? 255 : 0,  // Blue for Clubs and Spades
                              255);
        
        // Draw a simple shape based on the suit
        SDL_FRect suit_rect = {x + CARD_WIDTH/2 - 15, y + CARD_HEIGHT/2 - 15, 30, 30};
        
        switch (suit) {
            case 1: // Hearts
                // Simple heart shape
                SDL_RenderFillRect(renderer, &suit_rect);
                break;
            case 2: // Diamonds
                // Simple diamond shape
                {
                    // Draw a diamond shape using filled rectangles
                    SDL_FRect diamond_rect = {x + CARD_WIDTH/2 - 15, y + CARD_HEIGHT/2 - 15, 30, 30};
                    SDL_RenderFillRect(renderer, &diamond_rect);
                }
                break;
            case 3: // Clubs
                // Simple club shape
                SDL_RenderFillRect(renderer, &suit_rect);
                break;
            case 4: // Spades
                // Simple spade shape
                {
                    // Draw a spade shape using filled rectangles
                    SDL_FRect spade_rect = {x + CARD_WIDTH/2 - 15, y + CARD_HEIGHT/2 - 15, 30, 30};
                    SDL_RenderFillRect(renderer, &spade_rect);
                }
                break;
        }
        
        // Draw value in the corner
        // In a real app, you'd use SDL_ttf to render text
    }
}

void draw_game_board(SDL_Renderer* renderer) {
    // Draw the seven columns
    int start_x = 50;
    int start_y = 50;
    
    // Example of drawing some cards
    // In a real implementation, you would get the card data from your game state
    
    // Draw column headers
    for (int i = 0; i < 7; i++) {
        SDL_FRect header_rect = {start_x + i * (CARD_WIDTH + CARD_SPACING), 10, CARD_WIDTH, 30};
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(renderer, &header_rect);
        
        // In a real app, you'd use SDL_ttf to render text
        // For now, just draw a border
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderRect(renderer, &header_rect);
    }
    
    // Draw foundation piles
    for (int i = 0; i < 4; i++) {
        SDL_FRect foundation_rect = {
            start_x + (i + 8) * (CARD_WIDTH + CARD_SPACING), 
            10, 
            CARD_WIDTH, 
            CARD_HEIGHT
        };
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
        SDL_RenderFillRect(renderer, &foundation_rect);
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderRect(renderer, &foundation_rect);
    }
    
    // Example: Draw some cards in the columns
    // Column 1
    draw_card(renderer, start_x, start_y, 1, 1, false); // Ace of Hearts
    draw_card(renderer, start_x, start_y + CARD_HEIGHT/3, 2, 2, false); // 2 of Diamonds
    draw_card(renderer, start_x, start_y + 2*CARD_HEIGHT/3, 3, 3, false); // 3 of Clubs
    
    // Column 2
    draw_card(renderer, start_x + (CARD_WIDTH + CARD_SPACING), start_y, 4, 4, false); // 4 of Spades
    draw_card(renderer, start_x + (CARD_WIDTH + CARD_SPACING), start_y + CARD_HEIGHT/3, 5, 1, false); // 5 of Hearts
    
    // Column 3
    draw_card(renderer, start_x + 2*(CARD_WIDTH + CARD_SPACING), start_y, 6, 2, false); // 6 of Diamonds
    
    // Column 4
    draw_card(renderer, start_x + 3*(CARD_WIDTH + CARD_SPACING), start_y, 7, 3, true); // Hidden card
    
    // Column 5
    draw_card(renderer, start_x + 4*(CARD_WIDTH + CARD_SPACING), start_y, 8, 4, false); // 8 of Spades
    
    // Column 6
    draw_card(renderer, start_x + 5*(CARD_WIDTH + CARD_SPACING), start_y, 9, 1, false); // 9 of Hearts
    
    // Column 7
    draw_card(renderer, start_x + 6*(CARD_WIDTH + CARD_SPACING), start_y, 10, 2, false); // 10 of Diamonds
}
