#ifndef YUKON_LOGIC_H
#define YUKON_LOGIC_H

#include <stdbool.h>

// Card structure
typedef struct Card {
    int value;      // 1-13 (Ace, 2, 3..., Jack, Queen, King)
    int suit;       // 1-4 (Hearts, Diamonds, Clubs, Spades)
    bool is_hidden; // If true, the card is hidden (face down)
    struct Card* next;
} Card;

// Location translator for commands
typedef struct LocationTranslator {
    char from_tab;     // C || F
    int from_index;    // 1-7
    char from_card[5]; // Enough space for "10H" + '\0'

    char to_tab;       // C || F
    int to_index;      // 1-7
} LocationTranslator;

typedef enum {
    CardToMove,
    CardNewLocation
} GetCardType;

// Card and deck management functions
Card* create_deck();
void shuffle_card(Card** deck);
Card* get_card_by_index(Card* deck, int deckIndex);
Card* remove_card_from_deck(Card** deck, int index);
void deal_cards(Card** deck, Card** seven_rows);
void free_card_list(Card* list);
void cleanup_resources(Card* deck, Card* seven_rows[7], Card* four_pockets[4]);

// Card utility functions
Card* find_last_card(Card* head);
bool is_hidden(Card* card, Card** seven_rows);
void get_value_str(int value, char *value_str);
bool is_seven_rows_empty(Card* seven_rows[7]);

// Command execution
bool process_command(const char* command, Card* seven_rows[7], Card* four_pockets[4]);

// Move validation functions
bool is_move_allowed_to_seven_rows(Card* from, Card* to);
bool is_move_allowed_to_four_pockets(Card* from, Card* to);
bool can_move_from_foundation_to_column(Card* from, Card* to);

// Command parsing and execution
LocationTranslator* translate_command(const char* command);
Card* get_card(LocationTranslator* lt, Card* seven_rows[7], Card* four_pockets[4], GetCardType type, bool set_prev_to_null);
void cleanup_location_translator(LocationTranslator* lt);

// Game initialization
void initialize_game(Card** deck, Card** seven_rows, Card** four_pockets);

#endif // YUKON_LOGIC_H
