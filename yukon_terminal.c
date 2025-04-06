#define _CRT_SECURE_NO_WARNINGS 1 // To allow unsafe code
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>


typedef struct Card {
	int value; // 1-13 (Ace, 2, 3..., Jack, Queen, King)
	int suit; // (1-4, Hearts, Diamonds, Clubs, Spades)
	bool is_hidden; // TODO implement in deal_cards logic and in print_seven_rows logic, and remember to set last card visible when all are hidden
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
		//int swap = 3;

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

void print_deck(Card* deck) {
	const char* suits[] = { "Hearts", "Diamonds", "Clubs", "Spades" };
	const char* values[] = { "Ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King" };
	while (deck != NULL)
	{
		printf("%s of %s\n", values[deck->value - 1], suits[deck->suit - 1]); deck = deck->next;
	}
}

void print_seven_rows(Card** seven_rows, Card** four_pockets) {
	const char* suits[] = { "H", "D", "C", "S" };
	const char* values[] = { "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K" };

	int max_cards_in_row = 0;

	// Find the maximum number of cards in a row
	for (int i = 0; i < 7; i++) {
		int count = 0;
		Card* current_card = seven_rows[i];
		while (current_card != NULL) {
			count++;
			current_card = current_card->next;
		}
		if (count > max_cards_in_row) {
			max_cards_in_row = count;
		}
	}

	// Print header
	printf(" ");
	for (int i = 1; i <= 7; i++) {
		printf(" C%d   ", i);
	}
	for (int i = 1; i <= 4; i++) {
		printf(" F%d   ", i);
	}
	printf("\n");

	// Print cards in a tabular format
	for (int j = 0; j < max_cards_in_row; j++) {
		for (int i = 0; i < 7; i++) {
			Card* current_card = seven_rows[i];
			int k = 0;
			while (current_card != NULL && k < j) {
				current_card = current_card->next;
				k++;
			}
			if (current_card != NULL) {
				printf("%2s%-2s", values[current_card->value - 1], suits[current_card->suit - 1]);
			}
			else {
				printf("    ");
			}
			printf("  ");
		}
		for (int i = 0; i < 4; i++) {
			Card* current_card = four_pockets[i];
			int k = 0;
			while (current_card != NULL && k < j) {
				current_card = current_card->next;
				k++;
			}
			if (current_card != NULL) {
				printf("%2s%-2s", values[current_card->value - 1], suits[current_card->suit - 1]);
			}
			else {
				printf("    ");
			}
			printf("  ");
		}
		printf("\n");
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
				sprintf(current_card_str, "%c%c", value_char, "HDCS"[current_row->suit - 1]);

				// Compare the first two characters of the card string
				if (strncmp(current_card_str, card_str, 2) == 0) {
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
	if (from->value != 1 && to == NULL) {
		is_allowed = false;
	}
	else {
		if (to != NULL) {
			if ((from->value + 1) != to->value) {
				is_allowed = false;
			}
			if (from->suit == to->suit) {
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


int main()
{
	srand(time(NULL));
	Card* deck = create_deck();
	Card* seven_rows[7] = { NULL };
	Card* four_pockets[4] = { NULL };

	//print_deck(deck);
	shuffle_card(&deck);
	//print_deck(deck);
	 
	deal_cards(&deck, seven_rows);


	print_seven_rows(seven_rows, four_pockets);
	while (is_seven_rows_empty(seven_rows)==false) {
		char read_from_console[20];
		printf("Enter command:");
		scanf("%s", read_from_console);

		LocationTranslator* lt = translate_command(read_from_console);
		Card* card_to_move = get_card(lt, seven_rows, CardToMove, false);
		Card* card_new_location = get_card(lt, seven_rows, CardNewLocation, false);
		
		if (card_new_location != NULL) {
			bool rulesPassed = is_move_allowed_to_seven_rows(card_to_move, card_new_location);
			if (rulesPassed) {
				card_to_move = get_card(lt, seven_rows, CardToMove, true); // set prev to null if rule passed
				card_new_location->next = card_to_move;
			}
			else {
				printf("Move not allowed");
				printf("\n");
			}
		}
		else {
			if (lt->to_tab == 'F') {
				bool rulesPassed = is_move_allowed_to_four_pockets(card_to_move, card_new_location);
				if (rulesPassed) {
					card_to_move = get_card(lt, seven_rows, CardToMove, true); // set prev to null if rule passed
					four_pockets[lt->to_index - 1] = card_to_move;
				}
				else {
					printf("Move not allowed");
					printf("\n");
				}
			}
		}
		print_seven_rows(seven_rows, four_pockets);
		cleanup_location_translator(lt);

		
		
	}
	printf("\n You have won.");
	printf("Press Enter to exit...");
	getchar(); // Wait for Enter key
	cleanup_resources(deck, seven_rows, four_pockets);
    
}
