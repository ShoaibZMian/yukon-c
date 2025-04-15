#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>


// Window dimensions
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 800

// Card dimensions
#define CARD_WIDTH 71   // Adjust these values based on your card images
#define CARD_HEIGHT 96
#define CARD_SPACING 20
#define CARD_OVERLAP 40  // How much cards overlap in columns

// Game state
typedef struct Card {
    int value; // 1-13 (Ace, 2, 3..., Jack, Queen, King)
    int suit; // (1-4, Hearts, Diamonds, Clubs, Spades)
    bool is_hidden; // If true, the card is hidden (face down)
    struct Card* next;
} Card;

typedef struct LocationTranslator {
    char from_tab; // C || F
    int from_index; // 1-7
    char from_card[5]; // Enough space for "10H" + '\0'

    char to_tab; // C || F
    int to_index; // 1-7
} LocationTranslator;

typedef enum {
    CardToMove,
    CardNewLocation
} GetCardType;

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

// Input buffer for commands
char command_buffer[20] = "";
int command_buffer_index = 0;

// Function prototypes
void cleanup_and_exit(SDL_Window* window, SDL_Renderer* renderer, int exit_code);
void draw_card(SDL_Renderer* renderer, float x, float y, int value, int suit, bool is_hidden);
void draw_game_board(SDL_Renderer* renderer);
void process_mouse_down(int x, int y);
void process_mouse_up(int x, int y);
void process_mouse_motion(int x, int y);
void process_key(SDL_Keycode key);
void process_command(const char* command);
void initialize_game();
void cleanup_game();
void load_textures(SDL_Renderer* renderer);
void free_textures();
bool is_hidden(Card* card, Card** seven_rows);
Card* find_last_card(Card* head);

// Card game logic functions from udemy.c
void get_value_str(int value, char *value_str);
LocationTranslator* translate_command(const char* command);
Card* create_deck();
void shuffle_card(Card** deck);
void deal_cards(Card** deck, Card** seven_rows);
Card* get_card_by_index(Card* deck, int deckIndex);
Card* remove_card_from_deck(Card** deck, int index);
Card* get_card(LocationTranslator* lt, Card* seven_rows[7], Card* four_pockets[4], GetCardType type, bool set_prev_to_null);
bool is_move_allowed_to_seven_rows(Card* from, Card* to);
bool is_move_allowed_to_four_pockets(Card* from, Card* to);
void free_card_list(Card* list);
void cleanup_location_translator(LocationTranslator* lt);
bool is_seven_rows_empty(Card* seven_rows[7]);
char convert_to_char(int value);
bool can_move_from_foundation_to_column(Card* foundation_card, Card* destination_card);

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
    initialize_game();
    
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
                } else {
                    process_key(e.key.key);
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

// Function to load card textures
void load_textures(SDL_Renderer* renderer) {
    // Load background texture
    // You would replace this with your actual background image path
    SDL_Surface* background_surface = SDL_LoadBMP("my_background.bmp");
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
    
    // Draw command input
    SDL_FRect cmd_rect = {10, WINDOW_HEIGHT - 40, WINDOW_WIDTH - 20, 30};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &cmd_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderRect(renderer, &cmd_rect);
    
    char cmd_prompt[100];
    sprintf(cmd_prompt, "Command: %s", command_buffer);
    SDL_RenderDebugText(renderer, 15, WINDOW_HEIGHT - 35, cmd_prompt);
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
                
                process_command(cmd);
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

                process_command(cmd);
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
            process_command(cmd);
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

void process_key(SDL_Keycode key) {
    if (key == SDLK_RETURN) {
        // Process the command
        process_command(command_buffer);
        
        // Clear the command buffer
        memset(command_buffer, 0, sizeof(command_buffer));
        command_buffer_index = 0;
    } else if (key == SDLK_BACKSPACE) {
        // Remove the last character
        if (command_buffer_index > 0) {
            command_buffer[--command_buffer_index] = '\0';
        }
    } else if ((key >= SDLK_A && key <= SDLK_Z) || 
               (key >= SDLK_0 && key <= SDLK_9) ||
               key == SDLK_SPACE || key == SDLK_GREATER || 
               key == SDLK_LESS || key == SDLK_MINUS || key == SDLK_COLON) {
        // Add the character to the buffer
        if (command_buffer_index < sizeof(command_buffer) - 1) {
            command_buffer[command_buffer_index++] = (char)key;
            command_buffer[command_buffer_index] = '\0';
        }
    }
}

void process_command(const char* command) {
    LocationTranslator* lt = translate_command(command);
    
    // Handle foundation to column move
    if (lt->from_tab == 'F' && lt->to_tab == 'C') {
        // Get the foundation pile
        if (lt->from_index < 1 || lt->from_index > 4) {
            printf("Invalid foundation index\n");
            cleanup_location_translator(lt);
            return;
        }
        
        // Get the source foundation pile
        Card* foundation_pile = four_pockets[lt->from_index - 1];
        if (!foundation_pile) {
            printf("Foundation pile is empty\n");
            cleanup_location_translator(lt);
            return;
        }
        
        // Get the last card in the foundation pile
        Card* card_to_move = find_last_card(foundation_pile);
        
        // Get the destination card (if any)
        Card* destination_card = NULL;
        if (seven_rows[lt->to_index - 1]) {
            destination_card = find_last_card(seven_rows[lt->to_index - 1]);
        }
        
        // Check if move is allowed
        if (!can_move_from_foundation_to_column(card_to_move, destination_card)) {
            printf("Move not allowed (rules)\n");
            cleanup_location_translator(lt);
            return;
        }
        
        // Remove card from foundation
        if (foundation_pile == card_to_move) {
            // Only card in foundation
            four_pockets[lt->from_index - 1] = NULL;
        } else {
            // Find second-to-last card
            Card* current = foundation_pile;
            while (current->next != card_to_move) {
                current = current->next;
            }
            current->next = NULL;
        }
        
        // Add to column
        card_to_move->next = NULL;
        if (!seven_rows[lt->to_index - 1]) {
            seven_rows[lt->to_index - 1] = card_to_move;
        } else {
            destination_card->next = card_to_move;
        }
        
        cleanup_location_translator(lt);
        return;
    }

    // Handle column to column move
    if (lt->from_tab == 'C' && lt->to_tab == 'C') {
        // (A) Find the "top" card of the sub‐list we want to move
        Card* card_to_move = get_card(lt, seven_rows, four_pockets, CardToMove, false);
        if (!card_to_move) {
            printf("Move not allowed (card not found)\n");
            cleanup_location_translator(lt);
            return;
        }

        // (B) Check if that top card is hidden
        if (is_hidden(card_to_move, seven_rows)) {
            printf("Cannot move a hidden card\n");
            cleanup_location_translator(lt);
            return;
        }

        // (C) Find the destination "top" card (if any)
        Card* card_new_location = get_card(lt, seven_rows, four_pockets, CardNewLocation, false);

        // (D) Also find the 'prev' pointer in the old column so we can detach the sub‐list
        Card* current = seven_rows[lt->from_index - 1];
        Card* prev = NULL;
        while (current && current != card_to_move) {
            prev = current;
            current = current->next;
        }
        // 'exposed_card' is the one before the sub‐list being moved
        Card* exposed_card = prev;

        // (E) Check if the destination is empty or has a card on top
        // We only do the normal rules check on the top card of the sub‐list
        bool rules_passed = false;
        if (card_new_location) {
            rules_passed = is_move_allowed_to_seven_rows(card_to_move, card_new_location);
        } else {
            // Destination empty => only King can be placed
            rules_passed = (card_to_move->value == 13);
        }

        if (!rules_passed) {
            printf("Move not allowed (rules)\n");
            cleanup_location_translator(lt);
            return;
        }

        // ---- ACTUAL SUB‐LIST REMOVAL ----
        // 1) Unlink the entire sub‐list from old column
        if (!prev) {
            // sub‐list starts at the head
            seven_rows[lt->from_index - 1] = NULL;
        } else {
            // sub‐list is in the middle
            prev->next = NULL;
        }

        // 2) Link that entire sub‐list under the destination
        if (card_new_location) {
            // Find the bottom of the new location's sub‐list
            Card* bottom = card_new_location;
            while (bottom->next) {
                bottom = bottom->next;
            }
            // Append everything from card_to_move downward
            bottom->next = card_to_move;
        } else {
            // The new column is empty
            seven_rows[lt->to_index - 1] = card_to_move;
        }

        // (F) Expose the card that was just above the sub‐list
        if (exposed_card) {
            exposed_card->is_hidden = false;
        }
    }

    // Handle column to foundation move
    else if (lt->from_tab == 'C' && lt->to_tab == 'F') {
        // (A) Find the card to move
        Card* card_to_move = get_card(lt, seven_rows, four_pockets, CardToMove, false);
        if (!card_to_move) {
            printf("Move not allowed\n");
            cleanup_location_translator(lt);
            return;
        }

        // (B) Check if card is hidden
        if (is_hidden(card_to_move, seven_rows)) {
            printf("Cannot move a hidden card\n");
            cleanup_location_translator(lt);
            return;
        }

        // (C) Find the top card in the foundation pile (if any)
        Card* foundation_card = NULL;
        if (four_pockets[lt->to_index - 1]) {
            foundation_card = find_last_card(four_pockets[lt->to_index - 1]);
        }

        // (D) Also find 'prev' pointer in old column so we can unlink
        Card* current = seven_rows[lt->from_index - 1];
        Card* prev = NULL;
        while (current && current != card_to_move) {
            prev = current;
            current = current->next;
        }
        Card* exposed_card = prev;

        // (E) Check rules for single‐card foundation move
        bool rules_passed = is_move_allowed_to_four_pockets(card_to_move, foundation_card);
        if (!rules_passed) {
            printf("Move not allowed (foundation rules)\n");
            cleanup_location_translator(lt);
            return;
        }

        // 1) Unlink single card from old column
        if (!prev) {
            seven_rows[lt->from_index - 1] = card_to_move->next;
        } else {
            prev->next = card_to_move->next;
        }
        card_to_move->next = NULL; // only moving one card

        // 2) Link it into the foundation
        if (!four_pockets[lt->to_index - 1]) {
            // If foundation pile is empty
            four_pockets[lt->to_index - 1] = card_to_move;
        } else {
            // Append to the end of foundation
            Card* last_card = find_last_card(four_pockets[lt->to_index - 1]);
            last_card->next = card_to_move;
        }

        // 3) Expose the card behind the moved card
        if (exposed_card) {
            exposed_card->is_hidden = false;
        }
    }

    // 4) Check for victory
    if (is_seven_rows_empty(seven_rows)) {
        printf("\nYou have won!\n");
    }

    cleanup_location_translator(lt);
}



void initialize_game() {
    // Create and shuffle the deck
    deck = create_deck();
    shuffle_card(&deck);
    
    // Deal cards to the seven rows
    deal_cards(&deck, seven_rows);
}

void cleanup_game() {
    // Free the deck
    free_card_list(deck);
    
    // Free the seven rows
    for (int i = 0; i < 7; i++) {
        free_card_list(seven_rows[i]);
        seven_rows[i] = NULL;
    }
    
    // Free the four pockets
    for (int i = 0; i < 4; i++) {
        free_card_list(four_pockets[i]);
        four_pockets[i] = NULL;
    }
}

// Implementations of card game logic functions from udemy.c

LocationTranslator* translate_command(const char* command) {
    LocationTranslator* result = (LocationTranslator*)malloc(sizeof(LocationTranslator));

    // command:
    // C1->C4
    // C2:5H->C5 where 5H is the card
    // C3:10H->C5 where 10H is the card

    // Try to parse with card specified
    int parsed = sscanf(command, "%c%d:%4[^->]->%c%d", 
        &result->from_tab, &result->from_index, result->from_card, 
        &result->to_tab, &result->to_index);

    if (parsed < 5) {
        // Try to parse without card specified
        sscanf(command, "%c%d->%c%d", 
            &result->from_tab, &result->from_index, 
            &result->to_tab, &result->to_index);
        
        strcpy(result->from_card, "  ");  // empty if no card rank typed
    }

    return result;
}

Card* create_deck() {
    Card* deck = NULL;
    Card* new_card;
    for (int suit = 1; suit <= 4; suit++) {
        for (int newValue = 1; newValue <= 13; newValue++) {
            new_card = (Card*)malloc(sizeof(Card)); // malloc is used to allocate memory dynamically.
            new_card->value = newValue;
            new_card->suit = suit;
            new_card->is_hidden = false; // Initialize as visible by default
            new_card->next = deck;
            deck = new_card;
        }
    }
    return deck;
}

Card* get_card_by_index(Card* deck, int deckIndex) {
    if (deckIndex < 0) {
        return NULL;
    }

    Card* current_card = deck;
    int i;
    for (i = 0; i < deckIndex && current_card->next != NULL; i++) {
        current_card = current_card->next;
    }

    if (i == deckIndex && current_card != NULL) {
        return current_card;
    }
    else {
        return NULL;
    }
}

void shuffle_card(Card** deck) {
    int deck_size = 51;

    for (int i = 0; i < deck_size; i++) {
        int swap = rand() % (deck_size - i) + i;

        if (i == swap) continue; // Skip if the cards are the same

        Card* card_i = get_card_by_index(*deck, i);
        Card* card_swap = get_card_by_index(*deck, swap);

        Card* card_i_prev = (i > 0) ? get_card_by_index(*deck, i - 1) : NULL;
        Card* card_swap_prev = (swap > 0) ? get_card_by_index(*deck, swap - 1) : NULL;

        if (card_i_prev) {
            card_i_prev->next = card_swap;
        }
        else {
            *deck = card_swap;
        }

        if (card_swap_prev) {
            card_swap_prev->next = card_i;
        }

        Card* temp = card_i->next;

        if (swap - i == 1) { // When cards are adjacent
            card_i->next = card_swap->next;
            card_swap->next = card_i;
        }
        else { // When cards are not adjacent
            card_i->next = card_swap->next;
            card_swap->next = temp;
        }
    }
}

Card* remove_card_from_deck(Card** deck, int index) {
    if (index < 0 || *deck == NULL) {
        return NULL;
    }

    Card* current_card = *deck;
    Card* previous_card = NULL;
    int i;

    for (i = 0; i < index && current_card->next != NULL; i++) {
        previous_card = current_card;
        current_card = current_card->next;
    }

    if (i == index && current_card != NULL) {
        // Update the next pointer of the previous card
        if (previous_card != NULL) {
            previous_card->next = current_card->next;
        }
        else {
            // When removing the first card, update the deck pointer
            *deck = current_card->next;
        }

        // Detach the card from the deck
        current_card->next = NULL;
        return current_card;
    }
    else {
        return NULL;
    }
}

void deal_cards(Card** deck, Card** seven_rows) {
    // In Yukon solitaire, the first column has 1 card, the second has 6, the third has 7, etc.
    // The number of hidden cards increases with each column:
    // C1: 0 hidden cards (all visible)
    // C2: 1 hidden card (bottom card hidden)
    // C3: 2 hidden cards (bottom two cards hidden)
    // C4: 3 hidden cards (bottom three cards hidden)
    // And so on...
    
    // Deal first column (1 card, all visible)
    Card* card = remove_card_from_deck(deck, 0);
    card->is_hidden = false;
    card->next = NULL;
    seven_rows[0] = card;
    
    // Deal remaining columns
    for (int i = 1; i < 7; i++) {
        // Number of cards in this column (5+i)
        int num_cards = 5 + i;
        
        // Number of hidden cards in this column (i)
        int hidden_cards = i;
        
        // Create a temporary array to hold the cards for this column
        Card* column_cards[20]; // More than enough for any column
        
        // Deal cards for this column into the temporary array
        for (int j = 0; j < num_cards; j++) {
            column_cards[j] = remove_card_from_deck(deck, 0);
        }
        
        // Add cards to the column in the correct order with proper visibility
        for (int j = 0; j < num_cards; j++) {
            card = column_cards[j];
            
            // Set visibility based on position
            // The bottom 'hidden_cards' cards are hidden, the rest are visible
            if (j >= (num_cards - hidden_cards)) {
                card->is_hidden = true;
            } else {
                card->is_hidden = false;
            }
            
            // Add to the column (at the beginning of the linked list)
            card->next = seven_rows[i];
            seven_rows[i] = card;
        }
    }
}

// Instead of returning a single char, build an entire string for the rank:
void get_value_str(int value, char *value_str) {
    if (value == 1) {
        strcpy(value_str, "A");
    }
    else if (value == 11) {
        strcpy(value_str, "J");
    }
    else if (value == 12) {
        strcpy(value_str, "Q");
    }
    else if (value == 13) {
        strcpy(value_str, "K");
    }
    else {
        // For 2..10, we just convert the integer to string
        sprintf(value_str, "%d", value);
    }
}

// Keep this for backward compatibility
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

Card* find_last_card(Card* head) {
    if (head == NULL) {
        return NULL;
    }
    
    Card* current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    
    return current;
}

bool is_hidden(Card* card, Card** seven_rows) {
    // If the card is not in any of the seven rows, it's not hidden
    if (card == NULL) {
        return false;
    }
    
    // If the card has been explicitly unhidden, it should remain unhidden
    if (card->is_hidden == false) {
        return false;
    }
    
    // Find which column the card is in
    int column_index = -1;
    Card* current = NULL;
    
    for (int i = 0; i < 7; i++) {
        current = seven_rows[i];
        while (current != NULL) {
            if (current == card) {
                column_index = i;
                break;
            }
            current = current->next;
        }
        if (column_index != -1) {
            break;
        }
    }
    
    // If card is not found in any column, it's not hidden
    if (column_index == -1) {
        return false;
    }
    
    // Check if this card is at the bottom of the column (has no next card)
    // If it's at the bottom, it should always be visible
    if (card->next == NULL) {
        card->is_hidden = false; // Ensure bottom cards are always unhidden
        return false;
    }
    
    // Find the position of this card in the column (0 is top, total_cards-1 is bottom)
    int card_position = 0;
    current = seven_rows[column_index];
    while (current != NULL && current != card) {
        card_position++;
        current = current->next;
    }
    
    // In Yukon solitaire, the number of hidden cards in each column increases:
    // C1: 0 hidden cards
    // C2: 1 hidden card
    // C3: 2 hidden cards
    // ...
    // C7: 6 hidden cards
    
    // Calculate how many cards should be hidden in this column
    int hidden_cards = column_index; // 0 for C1, 1 for C2, etc.
    
    // The card is hidden if its position is less than the number of hidden cards
    // (Remember: position 0 is the top card, which is the first card in the linked list)
    bool should_be_hidden = (card_position < hidden_cards);
    
    // Update the card's is_hidden property to match what we've determined
    card->is_hidden = should_be_hidden;
    
    return should_be_hidden;
}

Card* get_card(LocationTranslator* lt, Card* seven_rows[7], Card* four_pockets[4], GetCardType type, bool set_prev_to_null) {
    char tab;
    int index;
    const char* card_str;

    if (type == CardToMove) {
        tab = lt->from_tab;
        index = lt->from_index;
        card_str = lt->from_card;
    }
    else {
        tab = lt->to_tab;
        index = lt->to_index;
        card_str = "  "; // Assume to_card is empty (top card)
    }

    // tab can be either 'C' or 'F'
    if (tab != 'C' && tab != 'F') {
        return NULL;
    }

    // For foundation piles (F1-F4)
    if (tab == 'F') {
        // Foundation piles are indexed 1-4
        if (index < 1 || index > 4) {
            return NULL;
        }
        
        Card* foundation_pile = four_pockets[index - 1];
        
        if (foundation_pile == NULL) {
            return NULL;
        }

        if (type == CardNewLocation) {
            return find_last_card(foundation_pile);
        }
        
        // For CardToMove, find the specific card in the foundation
        if (type == CardToMove) {
            // If card_str is empty, return the top card
            if (strcmp(card_str, "  ") == 0) {
                return find_last_card(foundation_pile);
            }
            
            // Otherwise, find the specific card
            Card* current = foundation_pile;
            Card* prev = NULL;
            
            while (current != NULL) {
                char current_card_str[5];
                char value_str[3];
                get_value_str(current->value, value_str);
                char suit_char = "HDCS"[current->suit - 1];
                sprintf(current_card_str, "%s%c", value_str, suit_char);

                if (strcmp(current_card_str, card_str) == 0) {
                    if (set_prev_to_null && prev != NULL) {
                        prev->next = NULL;
                    }
                    return current;
                }
                
                prev = current;
                current = current->next;
            }
        }
        return NULL;
    }

    // index should be between 1 and 7
    if (index < 1 || index > 7) {
        return NULL;
    }

    Card* current_row = seven_rows[index - 1];

    if (type == CardNewLocation) {
        // For CardNewLocation, we want to find the top visible card in the column
        // In a linked list, this would be the last card in the list
        Card* prevCard = NULL;
        
        // If the column is empty, return NULL
        if (current_row == NULL) {
            return NULL;
        }
        
        // Traverse to the end of the list (bottom of the column)
        while (current_row->next != NULL) {
            prevCard = current_row;
            current_row = current_row->next;
        }
        
        
        if (set_prev_to_null && prevCard != NULL) {
            prevCard->next = NULL;
        }
        return current_row;
    }
    else {
        // If card_str is not empty, find the card in the row
        if (strcmp(card_str, "  ") != 0) {
            Card* prevCard = NULL;
            while (current_row != NULL) {
                char current_card_str[5];   // enough space for "10H" + '\0'
                char value_str[3];
                get_value_str(current_row->value, value_str);
                char suit_char = "HDCS"[current_row->suit - 1];
                sprintf(current_card_str, "%s%c", value_str, suit_char);

                // Compare the card strings
                if (strcmp(current_card_str, card_str) == 0) {
                    if (set_prev_to_null) {
                        if (prevCard != NULL) {
                            prevCard->next = NULL;
                        }
                        else {
                            // If the current_row is the only item in the list
                            seven_rows[index - 1] = NULL;
                        }
                    }
                    return current_row;
                }
                prevCard = current_row;
                //current_row = current_row->next;
                if (current_row->next != NULL) {
                    current_row = current_row->next;
                }
                else {
                    // If we've reached the end of the list and haven't found the card,
                    // return NULL to indicate the card wasn't found
                    return NULL;
                }
            }
        }
        // If card_str is empty, return the top card of the row, 
        else {
            return current_row;
        }
    }
    return NULL;
}

bool is_move_allowed_to_seven_rows(Card* from, Card* to) {
    // If the destination is empty, only a king (value 13) can be placed there
    if (to == NULL) {
        return from->value == 13;  // Allow only if the card is a king
    }

    bool is_allowed = true;

    // Værdien skal være nøjagtigt +1
    if ((from->value + 1) != to->value) {
        is_allowed = false;
    }

    // Kuløren må ikke være den samme
    if (from->suit == to->suit) {
        is_allowed = false;
    }

    return is_allowed;
}

bool is_move_allowed_to_four_pockets(Card* from, Card* to) {
    bool is_allowed = true;
    
    // Check if the card has any linked cards (next != NULL)
    // Only single cards can be moved to Foundation piles
    if (from->next != NULL) {
        return false;
    }
    
    // For empty foundation piles, only Aces can be placed
    if (from->value != 1 && to == NULL) {
        is_allowed = false;
    }
    else {
        if (to != NULL) {
            // Card must be one value higher than the top card
            if (from->value != to->value + 1) {
                is_allowed = false;
            }
            // Card must be the same suit as the foundation pile
            if (from->suit != to->suit) {
                is_allowed = false;
            }
        }
    }
    return is_allowed;
}

void free_card_list(Card* list) {
    Card* current_card = list;
    Card* temp;
    while (current_card != NULL) {
        temp = current_card;
        current_card = current_card->next;
        free(temp);
    }
}

void cleanup_location_translator(LocationTranslator* lt) {
    if (lt) {
        free(lt);
    }
}

bool is_seven_rows_empty(Card* seven_rows[7]) {
    bool is_empty = true;
    for (int i = 0; i < 7; i++) {
        if (seven_rows[i] != NULL) {
            is_empty = false;
        }
    }
    return is_empty;
}

// Add this function to check if a card can be moved from foundation to column
bool can_move_from_foundation_to_column(Card* foundation_card, Card* destination_card) {
    // If destination is empty, only King can be placed
    if (destination_card == NULL) {
        return foundation_card->value == 13;
    }
    
    // Card must be one value less than destination
    if (foundation_card->value != destination_card->value - 1) {
        return false;
    }
    
    // Must be different suit
    if (foundation_card->suit == destination_card->suit) {
        return false;
    }
    
    return true;
}
