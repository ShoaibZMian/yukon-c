#include "yukon_logic.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>  // Kun brugt til fejlmeddelelser, ikke til UI

// Create a standard 52-card deck
Card* create_deck() {
    Card* deck = NULL;
    Card* new_card;
    for (int suit = 1; suit <= 4; suit++) {
        for (int newValue = 1; newValue <= 13; newValue++) {
            new_card = (Card*)malloc(sizeof(Card));
            new_card->value = newValue;
            new_card->suit = suit;
            new_card->is_hidden = false; // Initialize as visible by default
            new_card->next = deck;
            deck = new_card;
        }
    }
    return deck;
}

// Shuffle the deck using Fisher-Yates algorithm
void shuffle_card(Card** deck) {
    int deck_size = 0;
    Card* current = *deck;

    // Count the number of cards in the deck
    while (current != NULL) {
        deck_size++;
        current = current->next;
    }

    // Shuffle the deck
    for (int i = deck_size - 1; i > 0; i--) {
        int j = rand() % (i + 1);

        // Swap cards at positions i and j
        Card* card_i = get_card_by_index(*deck, i);
        Card* card_j = get_card_by_index(*deck, j);

        if (card_i != NULL && card_j != NULL) {
            int temp_value = card_i->value;
            int temp_suit = card_i->suit;

            card_i->value = card_j->value;
            card_i->suit = card_j->suit;

            card_j->value = temp_value;
            card_j->suit = temp_suit;
        }
    }
}

// Get a card by its index in the deck
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

// Remove a card from the deck by its index
Card* remove_card_from_deck(Card** deck, int index) {
    if (*deck == NULL) {
        return NULL;
    }

    Card* current = *deck;
    Card* previous = NULL;
    int i = 0;

    // Find the card at the specified index
    while (current != NULL && i < index) {
        previous = current;
        current = current->next;
        i++;
    }

    // If we found the card
    if (current != NULL && i == index) {
        // If it's the first card in the deck
        if (previous == NULL) {
            *deck = current->next;
        }
        else {
            previous->next = current->next;
        }
        current->next = NULL;
        return current;
    }

    return NULL;
}

// Deal cards to the seven rows according to Yukon solitaire rules
void deal_cards(Card** deck, Card** seven_rows) {
    // In Yukon solitaire:
    // C1: 1 card, all visible
    // C2: 6 cards (1 hidden, 5 visible)
    // C3: 7 cards (2 hidden, 5 visible)
    // C4: 8 cards (3 hidden, 5 visible)
    // C5: 9 cards (4 hidden, 5 visible)
    // C6: 10 cards (5 hidden, 5 visible)
    // C7: 11 cards (6 hidden, 5 visible)

    // Deal first column (1 card)
    Card* card = remove_card_from_deck(deck, 0);
    card->is_hidden = false;
    card->next = NULL;
    seven_rows[0] = card;

    // Deal remaining columns
    for (int i = 1; i < 7; i++) {
        // Number of cards in this column
        int num_cards = i + 5;

        // Start with an empty column
        seven_rows[i] = NULL;

        // Create a temporary array to hold the cards for this column
        Card* column_cards[20]; // More than enough for any column

        // Deal cards for this column into the temporary array
        for (int j = 0; j < num_cards; j++) {
            column_cards[j] = remove_card_from_deck(deck, 0);

            // Set initial visibility based on position
            // The first 'i' cards are hidden, the rest are visible
            column_cards[j]->is_hidden = (j < i);
        }

        // Add cards to the column in the correct order
        // The first card in the array should be at the top of the column
        for (int j = num_cards - 1; j >= 0; j--) {
            column_cards[j]->next = seven_rows[i];
            seven_rows[i] = column_cards[j];
        }
    }

    // Now update the hidden status of all cards
    for (int i = 0; i < 7; i++) {
        Card* current = seven_rows[i];
        while (current != NULL) {
            is_hidden(current, seven_rows);
            current = current->next;
        }
    }
}

// Find the last card in a linked list
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

// Check if a card is hidden
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

    // Count the total number of cards in this column
    int total_cards = 0;
    current = seven_rows[column_index];
    while (current != NULL) {
        total_cards++;
        current = current->next;
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

// Convert card value to string representation
void get_value_str(int value, char *value_str) {
    switch (value) {
        case 1:
            strcpy(value_str, "A");
            break;
        case 11:
            strcpy(value_str, "J");
            break;
        case 12:
            strcpy(value_str, "Q");
            break;
        case 13:
            strcpy(value_str, "K");
            break;
        default:
            sprintf(value_str, "%d", value);
            break;
    }
}

// Check if all seven rows are empty (game won)
bool is_seven_rows_empty(Card* seven_rows[7]) {
    for (int i = 0; i < 7; i++) {
        if (seven_rows[i] != NULL) {
            return false;
        }
    }
    return true;
}

// Check if a move to the seven rows is allowed
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

// Check if a move to the four pockets (foundations) is allowed
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

// Check if a move from foundation to column is allowed
bool can_move_from_foundation_to_column(Card* from, Card* to) {
    // Same rules as moving to seven rows
    return is_move_allowed_to_seven_rows(from, to);
}

// Parse a command string into a LocationTranslator structure
LocationTranslator* translate_command(const char* command) {
    LocationTranslator* lt = (LocationTranslator*)malloc(sizeof(LocationTranslator));
    if (lt == NULL) {
        return NULL;
    }

    // Initialize with default values
    lt->from_tab = '\0';
    lt->from_index = 0;
    lt->from_card[0] = '\0';
    lt->to_tab = '\0';
    lt->to_index = 0;

    // Parse the command string
    // Format: "C1:10H->C2" or "F1:10H->C2"
    if (strlen(command) < 8) {
        free(lt);
        return NULL;
    }

    // Parse from_tab and from_index
    lt->from_tab = command[0];
    lt->from_index = command[1] - '0';

    // Parse from_card
    int i = 3; // Start after the colon
    int j = 0;
    while (command[i] != '-' && j < 4) {
        lt->from_card[j++] = command[i++];
    }
    lt->from_card[j] = '\0';

    // Parse to_tab and to_index
    if (i + 3 >= strlen(command)) {
        free(lt);
        return NULL;
    }

    lt->to_tab = command[i + 2];
    lt->to_index = command[i + 3] - '0';

    return lt;
}

// Get a card based on the location translator
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

        // For CardNewLocation, return the top card of the foundation pile
        if (type == CardNewLocation) {
            // Get the foundation pile
            Card* foundation_pile = four_pockets[index - 1];

            if (foundation_pile == NULL) {
                return NULL;
            }

            // Find the last card in the foundation pile (the top card)
            Card* foundation_card = find_last_card(foundation_pile);

            return foundation_card;
        }

        // For CardToMove, we don't support moving cards from foundation piles
        if (type == CardToMove) {
            return NULL;
        }
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

bool process_command(const char* command, Card* seven_rows[7], Card* four_pockets[4]) {
    LocationTranslator* lt = translate_command(command);
    if (!lt) {
        return false; // Invalid command format
    }

    // Foundation to column move
    if (lt->from_tab == 'F' && lt->to_tab == 'C') {
        Card* card_to_move = find_last_card(four_pockets[lt->from_index - 1]);
        Card* destination = find_last_card(seven_rows[lt->to_index - 1]);

        if (!card_to_move || !can_move_from_foundation_to_column(card_to_move, destination)) {
            cleanup_location_translator(lt);
            return false;
        }

        // Remove from foundation
        Card* pile = four_pockets[lt->from_index - 1];
        if (pile == card_to_move) {
            four_pockets[lt->from_index - 1] = NULL;
        } else {
            Card* prev = pile;
            while (prev->next != card_to_move) prev = prev->next;
            prev->next = NULL;
        }

        // Add to column
        card_to_move->next = NULL;
        if (destination) {
            destination->next = card_to_move;
        } else {
            seven_rows[lt->to_index - 1] = card_to_move;
        }

        cleanup_location_translator(lt);
        return true;
    }

    // Column to column move
    if (lt->from_tab == 'C' && lt->to_tab == 'C') {
        Card* card_to_move = get_card(lt, seven_rows, four_pockets, CardToMove, false);
        Card* destination = get_card(lt, seven_rows, four_pockets, CardNewLocation, false);

        if (!card_to_move || is_hidden(card_to_move, seven_rows)) {
            cleanup_location_translator(lt);
            return false;
        }

        bool allowed = destination ? is_move_allowed_to_seven_rows(card_to_move, destination)
                                   : (card_to_move->value == 13);
        if (!allowed) {
            cleanup_location_translator(lt);
            return false;
        }

        // Detach sublist
        Card* prev = NULL;
        Card* current = seven_rows[lt->from_index - 1];
        while (current && current != card_to_move) {
            prev = current;
            current = current->next;
        }
        if (prev) prev->next = NULL;
        else seven_rows[lt->from_index - 1] = NULL;

        // Attach to destination
        if (destination) {
            Card* bottom = destination;
            while (bottom->next) bottom = bottom->next;
            bottom->next = card_to_move;
        } else {
            seven_rows[lt->to_index - 1] = card_to_move;
        }

        // Expose previous card
        if (prev) prev->is_hidden = false;

        cleanup_location_translator(lt);
        return true;
    }

    // Column to foundation move
    if (lt->from_tab == 'C' && lt->to_tab == 'F') {
        Card* card_to_move = get_card(lt, seven_rows, four_pockets, CardToMove, false);
        Card* destination = find_last_card(four_pockets[lt->to_index - 1]);

        if (!card_to_move || is_hidden(card_to_move, seven_rows) || !is_move_allowed_to_four_pockets(card_to_move, destination)) {
            cleanup_location_translator(lt);
            return false;
        }

        // Detach single card
        Card* prev = NULL;
        Card* current = seven_rows[lt->from_index - 1];
        while (current && current != card_to_move) {
            prev = current;
            current = current->next;
        }
        if (prev) prev->next = card_to_move->next;
        else seven_rows[lt->from_index - 1] = card_to_move->next;
        card_to_move->next = NULL;

        // Attach to foundation
        if (destination) {
            destination->next = card_to_move;
        } else {
            four_pockets[lt->to_index - 1] = card_to_move;
        }

        // Expose previous card
        if (prev) prev->is_hidden = false;

        cleanup_location_translator(lt);
        return true;
    }

    cleanup_location_translator(lt);
    return false; // Command not matched
}


// Clean up the location translator
void cleanup_location_translator(LocationTranslator* lt) {
    if (lt != NULL) {
        free(lt);
    }
}

// Free a linked list of cards
void free_card_list(Card* list) {
    Card* current_card = list;
    Card* temp;
    while (current_card != NULL) {
        temp = current_card;
        current_card = current_card->next;
        free(temp);
    }
}

// Clean up all game resources
void cleanup_resources(Card* deck, Card* seven_rows[7], Card* four_pockets[4]) {
    // Free the deck
    free_card_list(deck);

    // Free the seven rows
    for (int i = 0; i < 7; i++) {
        free_card_list(seven_rows[i]);
    }

    // Free the four pockets
    for (int i = 0; i < 4; i++) {
        free_card_list(four_pockets[i]);
    }
}

// Initialize the game
void initialize_game(Card** deck, Card** seven_rows, Card** four_pockets) {
    // Create and shuffle the deck
    *deck = create_deck();
    shuffle_card(deck);

    // Initialize seven_rows and four_pockets to NULL
    for (int i = 0; i < 7; i++) {
        seven_rows[i] = NULL;
    }

    for (int i = 0; i < 4; i++) {
        four_pockets[i] = NULL;
    }

    // Deal cards to the seven rows
    deal_cards(deck, seven_rows);
}
