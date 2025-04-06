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
#define CARD_WIDTH 80
#define CARD_HEIGHT 120
#define CARD_SPACING 20
#define CARD_OVERLAP 30  // How much cards overlap in columns

// Game state
typedef struct Card {
    int value; // 1-13 (Ace, 2, 3..., Jack, Queen, King)
    int suit; // (1-4, Hearts, Diamonds, Clubs, Spades)
    struct Card* next;
} Card;

typedef struct LocationTranslator {
    char from_tab; // C || F
    int from_index; // 1-7
    char from_card[3]; // 5H

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
float drag_offset_x = 0;
float drag_offset_y = 0;
bool is_dragging = false;

// Input buffer for commands
char command_buffer[20] = "";
int command_buffer_index = 0;

// Function prototypes
void cleanup_and_exit(SDL_Window* window, SDL_Renderer* renderer, int exit_code);
void draw_card(SDL_Renderer* renderer, float x, float y, int value, int suit);
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

// Card game logic functions from udemy.c
LocationTranslator* translate_command(const char* command);
Card* create_deck();
void shuffle_card(Card** deck);
void deal_cards(Card** deck, Card** seven_rows);
Card* get_card_by_index(Card* deck, int deckIndex);
Card* remove_card_from_deck(Card** deck, int index);
Card* get_card(LocationTranslator* lt, Card* seven_rows[7], GetCardType type, bool set_prev_to_null);
bool is_move_allowed_to_seven_rows(Card* from, Card* to);
bool is_move_allowed_to_four_pockets(Card* from, Card* to);
void free_card_list(Card* list);
void cleanup_location_translator(LocationTranslator* lt);
bool is_seven_rows_empty(Card* seven_rows[7]);
char convert_to_char(int value);

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
    SDL_Surface* background_surface = SDL_LoadBMP("background.bmp");
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

void draw_card(SDL_Renderer* renderer, float x, float y, int value, int suit) {
    SDL_FRect card_rect = {x, y, CARD_WIDTH, CARD_HEIGHT};
    
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
        if (four_pockets[i] != NULL) {
            Card* current = four_pockets[i];
            draw_card(renderer, foundation_x, foundation_y, 
                     current->value, current->suit);
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
                     current->value, current->suit);
            
            current = current->next;
            card_index++;
        }
    }
    
    // Draw the card being dragged, if any
    if (is_dragging && selected_card != NULL) {
        draw_card(renderer, drag_offset_x - CARD_WIDTH/2, drag_offset_y - CARD_HEIGHT/2, 
                 selected_card->value, selected_card->suit);
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
    
    // Check if click is in one of the seven columns
    for (int i = 0; i < 7; i++) {
        float col_x = start_x + i * (CARD_WIDTH + CARD_SPACING);
        
        if (x >= col_x && x <= col_x + CARD_WIDTH) {
            // Find the bottommost card in this column
            Card* current = seven_rows[i];
            if (current != NULL) {
                // Traverse to the last card in the column
                while (current->next != NULL) {
                    current = current->next;
                }
                
                // Select the bottom card
                selected_card = current;
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
    
    // Check if release is in one of the seven columns
    for (int i = 0; i < 7; i++) {
        float col_x = start_x + i * (CARD_WIDTH + CARD_SPACING);
        
        if (x >= col_x && x <= col_x + CARD_WIDTH) {
            // Try to move the card to this column
            if (i != selected_from_column) {
                char cmd[20];
                // Get the card value and suit as a string
                char value_char = convert_to_char(selected_card->value);
                char suit_char = "HDCS"[selected_card->suit - 1];
                sprintf(cmd, "C%d:%c%c->C%d", selected_from_column + 1, value_char, suit_char, i + 1);
                process_command(cmd);
            }
            break;
        }
    }
    
    // Check if release is in one of the foundation piles (now on the right side)
    for (int i = 0; i < 4; i++) {
        float foundation_x = start_x + 7 * (CARD_WIDTH + CARD_SPACING) + 20;
        float foundation_y = start_y + i * (CARD_HEIGHT + 20);
        
        if (x >= foundation_x && x <= foundation_x + CARD_WIDTH && 
            y >= foundation_y && y <= foundation_y + CARD_HEIGHT) {
            // Try to move the card to this foundation pile
            char cmd[20];
            // Get the card value and suit as a string
            char value_char = convert_to_char(selected_card->value);
            char suit_char = "HDCS"[selected_card->suit - 1];
            sprintf(cmd, "C%d:%c%c->F%d", selected_from_column + 1, value_char, suit_char, i + 1);
            process_command(cmd);
            break;
        }
    }
    
    // Reset dragging state
    selected_card = NULL;
    selected_from_column = -1;
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
    
    if (lt->from_tab == 'C' && lt->to_tab == 'C') {
        // Move from column to column
        Card* card_to_move = get_card(lt, seven_rows, CardToMove, false);
        Card* card_new_location = get_card(lt, seven_rows, CardNewLocation, false);
        
        if (card_new_location != NULL) {
            bool rules_passed = is_move_allowed_to_seven_rows(card_to_move, card_new_location);
            if (rules_passed) {
                card_to_move = get_card(lt, seven_rows, CardToMove, true); // set prev to null if rule passed
                card_new_location->next = card_to_move;
            }
        }
    } else if (lt->from_tab == 'C' && lt->to_tab == 'F') {
        // Move from column to foundation
        Card* card_to_move = get_card(lt, seven_rows, CardToMove, false);
        
        if (card_to_move != NULL) {
            bool rules_passed = is_move_allowed_to_four_pockets(card_to_move, four_pockets[lt->to_index - 1]);
            if (rules_passed) {
                card_to_move = get_card(lt, seven_rows, CardToMove, true); // set prev to null if rule passed
                four_pockets[lt->to_index - 1] = card_to_move;
            }
        }
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

    int parsed = sscanf(command, "%c%d:%2s->%c%d", &result->from_tab, &result->from_index, result->from_card, &result->to_tab, &result->to_index);

    if (parsed < 5) {
        sscanf(command, "%c%d->%c%d", &result->from_tab, &result->from_index, &result->to_tab, &result->to_index);
        strcpy(result->from_card, "  ");
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
    int card_counter = 6;
    for (int i = 0; i < 7; i++) {
        if (i == 0) {
            Card* card = remove_card_from_deck(deck, 0);
            card->next = seven_rows[i];
            seven_rows[i] = card;
        }
        else {
            for (int ii = 0; ii < card_counter; ii++) {
                Card* card = remove_card_from_deck(deck, 0);
                
                card->next = seven_rows[i];
                seven_rows[i] = card;
            }
            card_counter++;
        }
    }
}

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

Card* get_card(LocationTranslator* lt, Card* seven_rows[7], GetCardType type, bool set_prev_to_null) {
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

    // tab can be either 'C' or 'F', but we're only dealing with seven_rows in this example
    if (tab != 'C') {
        return NULL;
    }

    // index should be between 1 and 7
    if (index < 1 || index > 7) {
        return NULL;
    }

    Card* current_row = seven_rows[index - 1];

    if (type == CardNewLocation) {
        // Iterate to the last card in the row
        Card* prevCard = NULL;
        while (current_row != NULL && current_row->next != NULL) {
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
                char current_card_str[4];
                char value_char = convert_to_char(current_row->value);
                char suit_char = "HDCS"[current_row->suit - 1];
                sprintf(current_card_str, "%c%c", value_char, suit_char);

                // Compare the card string
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
                    return current_row;
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
    bool is_allowed = true;
    if ((from->value + 1) == to->value) {
    }
    else {
        is_allowed = false;
    }
    if (
        (from->suit == 0 && to->suit == 2) ||
        (from->suit == 0 && to->suit == 3) ||
        (from->suit == 1 && to->suit == 2) ||
        (from->suit == 1 && to->suit == 3) ||
        (from->suit == 2 && to->suit == 0) ||
        (from->suit == 2 && to->suit == 1) ||
        (from->suit == 3 && to->suit == 0) ||
        (from->suit == 3 && to->suit == 1)
        ) {
    }
    else {
        is_allowed = false;
    }
    return is_allowed;
}

bool is_move_allowed_to_four_pockets(Card* from, Card* to) {
    bool is_allowed = true;
    if (from->value != 1 && to == NULL) {
        is_allowed = false;
    }
    else {
        if (to != NULL) {
            if ((from->value + 1) != to->value) {
                is_allowed = false;
            }
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
