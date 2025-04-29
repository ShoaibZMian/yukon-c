#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "yukon_logic.h" // Include the game logic header

// Window dimensions
#define WINDOW_WIDTH 1050
#define WINDOW_HEIGHT 1000

// Card dimensions
#define CARD_WIDTH 100   // Adjust these values based on your card images
#define CARD_HEIGHT 130
#define CARD_SPACING 20
#define CARD_OVERLAP 40  // How much cards overlap in columns

// Game state variables
Card* deck = NULL;
Card* seven_rows[7] = { NULL };
Card* four_pockets[4] = { NULL };

// Selected card for dragging
Card* selected_card = NULL;
int selected_from_column = -1;
int selected_from_foundation = -1;
float drag_offset_x = 0;
float drag_offset_y = 0;
bool is_dragging = false;


// Function prototypes for GUI-specific functions
void cleanup_and_exit(SDL_Window* window, SDL_Renderer* renderer, int exit_code);
void draw_card(SDL_Renderer* renderer, float x, float y, int value, int suit, bool is_hidden);
void draw_game_board(SDL_Renderer* renderer);
void process_mouse_down(int x, int y);
void process_mouse_up(int x, int y);
void process_mouse_motion(int x, int y);
void init_game();
void cleanup_game();
void load_textures(SDL_Renderer* renderer);
void free_textures();
char convert_to_char(int value); // Helper function for GUI display

int main(int argc, char* argv[]) {
    // Initialize random seed
    srand(time(NULL));

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

    // Initialize game
    init_game();

    // Load textures
    load_textures(renderer);

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
            // Mouse button pressed
            else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (e.button.button == 1) { // Left mouse button
                    process_mouse_down(e.button.x, e.button.y);
                }
            }
            // Mouse button released
            else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                if (e.button.button == 1) { // Left mouse button
                    process_mouse_up(e.button.x, e.button.y);
                }
            }
            // Mouse motion
            else if (e.type == SDL_EVENT_MOUSE_MOTION) {
                process_mouse_motion(e.motion.x, e.motion.y);
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
    cleanup_game();
    cleanup_and_exit(window, renderer, 0);
    return 0;
}

void cleanup_and_exit(SDL_Window* window, SDL_Renderer* renderer, int exit_code) {
    // Free textures
    free_textures();

    // Destroy renderer and window
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// Card textures
SDL_Texture* card_textures[13][4] = { NULL }; // [value][suit]
SDL_Texture* card_back_texture = NULL;
SDL_Texture* background_texture = NULL;
SDL_Texture* king_of_hearts_texture = NULL; // Special texture for King of Hearts

// Function to load card textures
void load_textures(SDL_Renderer* renderer) {
    // Load background texture from GUI_assets folder
    SDL_Surface* background_surface = SDL_LoadBMP("GUI_assets/BG2.bmp");
    if (background_surface) {
        background_texture = SDL_CreateTextureFromSurface(renderer, background_surface);
        SDL_DestroySurface(background_surface);
    }

    // Load card textures
    // This is just a placeholder - you would need to create actual card images
    char filename[100];
    for (int suit = 0; suit < 4; suit++) {
        for (int value = 0; value < 13; value++) {
            // Format would be like "cards/hearts_ace.bmp", "cards/clubs_king.bmp", etc.
            sprintf(filename, "cards/%s_%s.bmp",
                    suit == 0 ? "hearts" : suit == 1 ? "diamonds" : suit == 2 ? "clubs" : "spades",
                    value == 0 ? "ace" : value == 10 ? "jack" : value == 11 ? "queen" : value == 12 ? "king" :
                    (char[]){value + '1', '\0'});

            SDL_Surface* card_surface = SDL_LoadBMP(filename);
            if (card_surface) {
                card_textures[value][suit] = SDL_CreateTextureFromSurface(renderer, card_surface);
                SDL_DestroySurface(card_surface);
            }
        }
    }

    // Load card back texture
    SDL_Surface* card_back_surface = SDL_LoadBMP("cards/back.bmp");
    if (card_back_surface) {
        card_back_texture = SDL_CreateTextureFromSurface(renderer, card_back_surface);
        SDL_DestroySurface(card_back_surface);
    }

    // Load King of Hearts texture from gui_assets folder
    SDL_Surface* king_of_hearts_surface = SDL_LoadBMP("gui_assets/king_of_hart.bmp");
    if (king_of_hearts_surface) {
        king_of_hearts_texture = SDL_CreateTextureFromSurface(renderer, king_of_hearts_surface);
        SDL_DestroySurface(king_of_hearts_surface);
        printf("King of Hearts image loaded successfully!\n");
    } else {
        printf("Failed to load King of Hearts image: %s\n", SDL_GetError());
    }
}

// Function to free textures
void free_textures() {
    if (background_texture) {
        SDL_DestroyTexture(background_texture);
        background_texture = NULL;
    }

    for (int suit = 0; suit < 4; suit++) {
        for (int value = 0; value < 13; value++) {
            if (card_textures[value][suit]) {
                SDL_DestroyTexture(card_textures[value][suit]);
                card_textures[value][suit] = NULL;
            }
        }
    }

    if (card_back_texture) {
        SDL_DestroyTexture(card_back_texture);
        card_back_texture = NULL;
    }

    if (king_of_hearts_texture) {
        SDL_DestroyTexture(king_of_hearts_texture);
        king_of_hearts_texture = NULL;
    }
}

void draw_card(SDL_Renderer* renderer, float x, float y, int value, int suit, bool is_hidden) {
    SDL_FRect card_rect = {x, y, CARD_WIDTH, CARD_HEIGHT};

    // If the card is hidden, draw the card back
    if (is_hidden) {
        // If we have a card back texture, use it
        if (card_back_texture) {
            SDL_RenderTexture(renderer, card_back_texture, NULL, &card_rect);
            return;
        }

        // Fallback to drawing a card back manually
        SDL_SetRenderDrawColor(renderer, 50, 50, 200, 255); // Blue back
        SDL_RenderFillRect(renderer, &card_rect);

        // Draw border
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderRect(renderer, &card_rect);

        // Draw a pattern on the back
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < 5; i++) {
            SDL_FRect pattern_rect = {
                x + CARD_WIDTH * 0.2f + i * CARD_WIDTH * 0.15f,
                y + CARD_HEIGHT * 0.2f + i * CARD_HEIGHT * 0.15f,
                CARD_WIDTH * 0.6f - i * CARD_WIDTH * 0.3f,
                CARD_HEIGHT * 0.6f - i * CARD_HEIGHT * 0.3f
            };
            SDL_RenderRect(renderer, &pattern_rect);
        }

        return;
    }

    // For visible cards, proceed as before
    // Special case for King of Hearts
    if (value == 13 && suit == 1 && king_of_hearts_texture) {
        SDL_RenderTexture(renderer, king_of_hearts_texture, NULL, &card_rect);
        return;
    }

    // Adjust value and suit for array indexing (0-based)
    int value_index = value - 1;
    int suit_index = suit - 1;

    // If we have a texture for this card, use it
    if (value_index >= 0 && value_index < 13 &&
        suit_index >= 0 && suit_index < 4 &&
        card_textures[value_index][suit_index]) {
        SDL_RenderTexture(renderer, card_textures[value_index][suit_index], NULL, &card_rect);
        return;
    }

    // Fallback to drawing the card manually if texture isn't available
    // Card background (white with rounded corners)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &card_rect);

    // Draw border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderRect(renderer, &card_rect);

    // Set color based on suit (red for Hearts/Diamonds, black for Clubs/Spades)
    SDL_SetRenderDrawColor(renderer,
                          (suit == 1 || suit == 2) ? 255 : 0,  // Red for Hearts and Diamonds
                          0,
                          0,
                          255);

    // Get value string
    char value_str[3];
    if (value == 1) {
        strcpy(value_str, "A");
    } else if (value == 11) {
        strcpy(value_str, "J");
    } else if (value == 12) {
        strcpy(value_str, "Q");
    } else if (value == 13) {
        strcpy(value_str, "K");
    } else {
        sprintf(value_str, "%d", value);
    }

    // Get suit letter
    char suit_letter = 'X';
    switch (suit) {
        case 1: // Hearts
            suit_letter = 'H';
            break;
        case 2: // Diamonds
            suit_letter = 'D';
            break;
        case 3: // Clubs
            suit_letter = 'C';
            break;
        case 4: // Spades
            suit_letter = 'S';
            break;
    }

    // Create combined value+suit string
    char card_text[4];
    sprintf(card_text, "%s%c", value_str, suit_letter);

    // Draw value+suit in top-left corner
    SDL_RenderDebugText(renderer, x + 5, y + 5, card_text);

    // Draw suit symbol for visual representation
    char suit_symbol[2] = {0};
    switch (suit) {
        case 1: // Hearts
            suit_symbol[0] = 3; // Heart symbol
            break;
        case 2: // Diamonds
            suit_symbol[0] = 4; // Diamond symbol
            break;
        case 3: // Clubs
            suit_symbol[0] = 5; // Club symbol
            break;
        case 4: // Spades
            suit_symbol[0] = 6; // Spade symbol
            break;
    }

    // Draw suit symbols in the middle of the card based on value
    if (value >= 1 && value <= 10) {
        // For number cards, draw the appropriate number of suit symbols
        int num_symbols = value;
        if (value == 1) num_symbols = 1; // Ace has one symbol

        // Define positions for suit symbols based on card value (up to 10 symbols per card)
        float positions[][2] = {
            // Ace (1 symbol)
            {0.5, 0.5},

            // Two (2 symbols)
            {0.5, 0.3}, {0.5, 0.7},

            // Three (3 symbols)
            {0.5, 0.3}, {0.5, 0.5}, {0.5, 0.7},

            // Four (4 symbols)
            {0.3, 0.3}, {0.7, 0.3}, {0.3, 0.7}, {0.7, 0.7},

            // Five (5 symbols)
            {0.3, 0.3}, {0.7, 0.3}, {0.5, 0.5}, {0.3, 0.7}, {0.7, 0.7},

            // Six (6 symbols)
            {0.3, 0.3}, {0.7, 0.3}, {0.3, 0.5}, {0.7, 0.5}, {0.3, 0.7}, {0.7, 0.7},

            // Seven (7 symbols)
            {0.3, 0.2}, {0.7, 0.2}, {0.5, 0.35}, {0.3, 0.5}, {0.7, 0.5}, {0.3, 0.8}, {0.7, 0.8},

            // Eight (8 symbols)
            {0.3, 0.2}, {0.7, 0.2}, {0.3, 0.4}, {0.7, 0.4}, {0.3, 0.6}, {0.7, 0.6}, {0.3, 0.8}, {0.7, 0.8},

            // Nine (9 symbols)
            {0.3, 0.2}, {0.7, 0.2}, {0.3, 0.4}, {0.5, 0.4}, {0.7, 0.4}, {0.3, 0.6}, {0.7, 0.6}, {0.3, 0.8}, {0.7, 0.8},

            // Ten (10 symbols)
            {0.3, 0.2}, {0.7, 0.2}, {0.3, 0.35}, {0.7, 0.35}, {0.5, 0.5}, {0.3, 0.65}, {0.7, 0.65}, {0.3, 0.8}, {0.7, 0.8}, {0.5, 0.2}
        };

        // Draw the suit symbols based on card value
        int start_index = 0;

        // Calculate the starting index in the positions array for this card value
        for (int v = 1; v < value; v++) {
            start_index += v;
        }

        // Draw the appropriate number of suit symbols
        for (int i = 0; i < num_symbols; i++) {
            float pos_x = x + positions[start_index + i][0] * CARD_WIDTH;
            float pos_y = y + positions[start_index + i][1] * CARD_HEIGHT;

            // Draw suit symbol
            SDL_RenderDebugText(renderer, pos_x - 5, pos_y - 5, suit_symbol);
        }
    } else {
        // For face cards (J, Q, K), draw a larger symbol in the center
        SDL_RenderDebugText(renderer, x + CARD_WIDTH/2 - 5, y + CARD_HEIGHT/2 - 5, suit_symbol);
        SDL_RenderDebugText(renderer, x + CARD_WIDTH/2 - 15, y + CARD_HEIGHT/2 - 15, value_str);
    }

    // Draw value+suit in bottom-right corner (upside down)
    SDL_RenderDebugText(renderer, x + CARD_WIDTH - 25, y + CARD_HEIGHT - 15, card_text);
}

void draw_game_board(SDL_Renderer* renderer) {
    // Draw background if available
    if (background_texture) {
        SDL_FRect bg_rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderTexture(renderer, background_texture, NULL, &bg_rect);
    }

    int start_x = 50;
    int start_y = 50;

    // Draw column headers (smaller and more subtle)
    for (int i = 0; i < 7; i++) {
        SDL_FRect header_rect = {start_x + i * (CARD_WIDTH + CARD_SPACING), 10, CARD_WIDTH, 30};
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(renderer, &header_rect);

        char header_text[5];
        sprintf(header_text, "C%d", i + 1);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderRect(renderer, &header_rect);
        SDL_RenderDebugText(renderer, start_x + i * (CARD_WIDTH + CARD_SPACING) + 10, 15, header_text);
    }

    // Draw foundation piles on the right side with gold borders like in the reference image
    for (int i = 0; i < 4; i++) {
        float foundation_x = start_x + 7 * (CARD_WIDTH + CARD_SPACING) + 20;
        float foundation_y = start_y + i * (CARD_HEIGHT + 20);

        // Draw empty foundation outline with gold color
        SDL_FRect foundation_rect = {
            foundation_x,
            foundation_y,
            CARD_WIDTH,
            CARD_HEIGHT
        };

        // Draw gold border
        SDL_SetRenderDrawColor(renderer, 218, 165, 32, 255); // Gold color
        SDL_RenderRect(renderer, &foundation_rect);

        // Draw cards in foundation piles
        Card* top_of_foundation = find_last_card(four_pockets[i]);
        if (top_of_foundation != NULL) {
            draw_card(renderer, foundation_x, foundation_y,
                      top_of_foundation->value,
                      top_of_foundation->suit,
                      false); // foundation cards are always face-up
        }

    }

    // Draw cards in the seven columns with cascading effect like in the reference image
    for (int i = 0; i < 7; i++) {
        float x = start_x + i * (CARD_WIDTH + CARD_SPACING);
        float y = start_y;

        Card* current = seven_rows[i];
        int card_index = 0;

        while (current != NULL) {
            // Skip drawing if this is the selected card being dragged
            if (is_dragging && i == selected_from_column && current == selected_card) {
                current = current->next;
                card_index++;
                continue;
            }

            // Draw cards with cascading effect
            float overlap = CARD_OVERLAP;
            draw_card(renderer, x, y + card_index * overlap,
                     current->value, current->suit, current->is_hidden);

            current = current->next;
            card_index++;
        }
    }

// Draw the stack of cards being dragged, if any
if (is_dragging && selected_card != NULL) {
    // Draw all cards in the stack
    Card* current = selected_card;
    int card_index = 0;

    while (current != NULL) {
        // Draw each card in the stack with a cascading effect
        float overlap = CARD_OVERLAP / 2; // Use a smaller overlap for dragging
        draw_card(renderer,
                 drag_offset_x - CARD_WIDTH/2,
                 drag_offset_y - CARD_HEIGHT/2 + card_index * overlap,
                 current->value, current->suit, false); // Dragged cards are always visible

        current = current->next;
        card_index++;
    }
}


}

void process_mouse_down(int x, int y) {
    int start_x = 50;
    int start_y = 50;

    // Check foundations first
    for (int i = 0; i < 4; i++) {
        float foundation_x = start_x + 7 * (CARD_WIDTH + CARD_SPACING) + 20;
        float foundation_y = start_y + i * (CARD_HEIGHT + 20);

        if (x >= foundation_x && x <= foundation_x + CARD_WIDTH &&
            y >= foundation_y && y <= foundation_y + CARD_HEIGHT) {

            // Only allow selecting the top card from foundation
            if (four_pockets[i] != NULL) {
                Card* top_card = find_last_card(four_pockets[i]);
                if (top_card != NULL) {
                    selected_card = top_card;
                    selected_from_foundation = i;
                    selected_from_column = -1;
                    is_dragging = true;
                    drag_offset_x = x - foundation_x;
                    drag_offset_y = y - foundation_y;
                }
                return;
            }
        }
    }

    // Check if click is in one of the seven columns
    for (int i = 0; i < 7; i++) {
        float col_x = start_x + i * (CARD_WIDTH + CARD_SPACING);

        if (x >= col_x && x <= col_x + CARD_WIDTH) {
            // Count cards in this column and find the card that was clicked
            Card* current = seven_rows[i];
            if (current != NULL) {
                // Count total cards in this column
                int total_cards = 0;
                Card* temp = current;
                while (temp != NULL) {
                    total_cards++;
                    temp = temp->next;
                }

                // Calculate which card was clicked based on y position
                int card_index = (y - start_y) / CARD_OVERLAP;

                // Make sure the index is within bounds
                if (card_index >= total_cards) {
                    card_index = total_cards - 1;
                }

                // Find the card at the calculated index
                Card* clicked_card = current;
                for (int j = 0; j < card_index && clicked_card->next != NULL; j++) {
                    clicked_card = clicked_card->next;
                }

                // Check if the card is hidden
                if (is_hidden(clicked_card, seven_rows)) {
                    // Can't select hidden cards
                    return;
                }

                // In Yukon solitaire, we select the clicked card and all cards below it
                // The clicked_card is already the card we want to select
                selected_card = clicked_card;
                selected_from_column = i;
                drag_offset_x = x;
                drag_offset_y = y;
                is_dragging = true;
                break;
            }
        }
    }
}

void process_mouse_up(int x, int y) {
    if (!is_dragging || selected_card == NULL) {
        return;
    }

    int start_x = 50;
    int start_y = 50;

    // Check for column destinations
    for (int i = 0; i < 7; i++) {
        float column_x = start_x + i * (CARD_WIDTH + CARD_SPACING);
        float column_y = start_y;

        if (x >= column_x && x <= column_x + CARD_WIDTH) {
            // If dragging from foundation
            if (selected_from_foundation != -1) {
                char cmd[20];
                char value_str[5];
                get_value_str(selected_card->value, value_str);
                char suit_char = "HDCS"[selected_card->suit - 1];

                // Build command e.g. "F1:10H->C3"
                sprintf(cmd, "F%d:%s%c->C%d",
                        selected_from_foundation + 1,
                        value_str,
                        suit_char,
                        i + 1);

                        if (!process_command(cmd, seven_rows, four_pockets)) {
                            // Optionelt: vis fejl i GUI-loggen
                        }

                break;
            }
        }
    }

    // Check if release is in one of the seven columns
    for (int i = 0; i < 7; i++) {
        float col_x = start_x + i * (CARD_WIDTH + CARD_SPACING);

        if (x >= col_x && x <= col_x + CARD_WIDTH) {
            // Try to move the card to this column
            if (i != selected_from_column) {
                char cmd[20];
                char value_str[5];
                char suit_char = "HDCS"[selected_card->suit - 1];

                // FIX: Actually fill 'value_str' before using it
                get_value_str(selected_card->value, value_str);

                // Build command e.g. "C3:10H->C5"
                sprintf(cmd, "C%d:%s%c->C%d",
                        selected_from_column + 1,
                        value_str,
                        suit_char,
                        i + 1);

                        if (!process_command(cmd, seven_rows, four_pockets)) {
                            // Optionelt: vis fejl i GUI-loggen
                        }

            }
            break;  // stop checking other columns
        }
    }

    // Check if release is in one of the foundation piles (now on the right side)
    for (int i = 0; i < 4; i++) {
        float foundation_x = start_x + 7 * (CARD_WIDTH + CARD_SPACING) + 20;
        float foundation_y = start_y + i * (CARD_HEIGHT + 20);

        if (x >= foundation_x && x <= foundation_x + CARD_WIDTH &&
            y >= foundation_y && y <= foundation_y + CARD_HEIGHT) {
            // For foundation piles, we can only move a single card (no next card)
            if (selected_card->next != NULL) {
                // Can't move a stack to foundation
                // Reset dragging state
                selected_card = NULL;
                selected_from_column = -1;
                is_dragging = false;
                return;
            }

            // Try to move the card to this foundation pile
            char cmd[20];

            // Get the card value and suit as a string
            char value_str[5];
            get_value_str(selected_card->value, value_str);
            char suit_char = "HDCS"[selected_card->suit - 1];
            sprintf(cmd, "C%d:%s%c->F%d", selected_from_column + 1, value_str, suit_char, i + 1);
            if (!process_command(cmd, seven_rows, four_pockets)) {
                // Optionelt: vis fejl i GUI-loggen
            }

            break;
        }
    }

    // Reset dragging state
    selected_card = NULL;
    selected_from_column = -1;
    selected_from_foundation = -1;
    is_dragging = false;
}

void process_mouse_motion(int x, int y) {
    if (is_dragging) {
        drag_offset_x = x;
        drag_offset_y = y;
    }
}




void init_game() {
    // Initialize the game using the logic component
    initialize_game(&deck, seven_rows, four_pockets);
}

void cleanup_game() {
    // Clean up resources using the logic component
    cleanup_resources(deck, seven_rows, four_pockets);

    // Reset pointers
    deck = NULL;
    for (int i = 0; i < 7; i++) {
        seven_rows[i] = NULL;
    }
    for (int i = 0; i < 4; i++) {
        four_pockets[i] = NULL;
    }
}

// GUI-specific helper function
char convert_to_char(int value) {
    if (value == 1) {
        return 'A';
    }
    else if (value == 11) {
        return 'J';
    }
    else if (value == 12) {
        return 'Q';
    }
    else if (value == 13) {
        return 'K';
    }
    else {
        return value <= 9 ? (value + '0') : 0;
    }
}
