#define _CRT_SECURE_NO_WARNINGS 1 // To allow unsafe code
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "yukon_logic.h" // Include the game logic header

// Function prototypes for terminal-specific functions
void print_deck(Card* deck);
void print_seven_rows(Card** seven_rows, Card** four_pockets);


// Terminal-specific functions

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

	// Find the maximum number of cards in a row (both columns and foundation piles)
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

	// Also check foundation piles
	for (int i = 0; i < 4; i++) {
		int count = 0;
		Card* current_card = four_pockets[i];
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
				// Check if the card is hidden using the is_hidden function
				bool hidden = is_hidden(current_card, seven_rows);

				if (hidden) {
					// For hidden cards, display as "[]" with proper alignment
					printf("%2s%-2s", "[", "]");
				} else {
					// For visible cards, display normally
					printf("%2s%-2s", values[current_card->value - 1], suits[current_card->suit - 1]);
				}
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
				// Foundation cards are always visible
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






int main()
{
    srand(time(NULL));

    // Initialize game components
    Card* deck = NULL;
    Card* seven_rows[7] = { NULL };
    Card* four_pockets[4] = { NULL };

    // Initialize the game using the logic component
    initialize_game(&deck, seven_rows, four_pockets);

    // Display the initial game state
    print_seven_rows(seven_rows, four_pockets);

    // Main game loop
    while (!is_seven_rows_empty(seven_rows)) {
        char read_from_console[20];
        printf("Enter command: ");
        scanf("%s", read_from_console);

        // Parse the command
        LocationTranslator* lt = translate_command(read_from_console);

        // Get the card to move
        Card* card_to_move = get_card(lt, seven_rows, four_pockets, CardToMove, false);
        if (!card_to_move) {
            printf("Move not allowed - Card not found\n");
            print_seven_rows(seven_rows, four_pockets);
            cleanup_location_translator(lt);
            continue;
        }

        // Debug info
        printf("Found card: %d of suit %d\n", card_to_move->value, card_to_move->suit);

        // Get the destination
        Card* card_new_location = get_card(lt, seven_rows, four_pockets, CardNewLocation, false);

        // Check if the card is hidden
        if (is_hidden(card_to_move, seven_rows)) {
            printf("Cannot move a hidden card\n");
            print_seven_rows(seven_rows, four_pockets);
            cleanup_location_translator(lt);
            continue;
        }

        // Process move based on destination type
        if (lt->to_tab == 'C') {
            // Moving to a column
            if (is_move_allowed_to_seven_rows(card_to_move, card_new_location)) {
                // Find the card that will be exposed after moving
                Card* exposed_card = NULL;
                Card* current = seven_rows[lt->from_index - 1];
                Card* prev = NULL;

                // Find the card before the one being moved
                while (current != NULL && current != card_to_move) {
                    prev = current;
                    current = current->next;
                }

                // If there's a card before the one being moved, it will be exposed
                exposed_card = prev;

                // Move the card
                card_to_move = get_card(lt, seven_rows, four_pockets, CardToMove, true);

                if (card_new_location != NULL) {
                    // If destination has a card, attach to it
                    card_new_location->next = card_to_move;
                } else {
                    // If destination is empty, place card directly in the column
                    seven_rows[lt->to_index - 1] = card_to_move;
                }

                // If a card was exposed, make it visible
                if (exposed_card != NULL) {
                    exposed_card->is_hidden = false;
                }
            } else {
                printf("Move not allowed\n");
            }
        } else if (lt->to_tab == 'F') {
            // Moving to a foundation
            if (is_move_allowed_to_four_pockets(card_to_move, card_new_location)) {
                // Find the card that will be exposed after moving
                Card* exposed_card = NULL;
                Card* current = seven_rows[lt->from_index - 1];
                Card* prev = NULL;

                // Find the card before the one being moved
                while (current != NULL && current != card_to_move) {
                    prev = current;
                    current = current->next;
                }

                // If there's a card before the one being moved, it will be exposed
                exposed_card = prev;

                // Move the card
                card_to_move = get_card(lt, seven_rows, four_pockets, CardToMove, true);

                // Make sure the card's next pointer is NULL since we're only moving a single card
                card_to_move->next = NULL;

                // If foundation pile is empty, set it directly
                if (four_pockets[lt->to_index - 1] == NULL) {
                    four_pockets[lt->to_index - 1] = card_to_move;
                } else {
                    // Find the last card in the foundation pile
                    Card* last_card = find_last_card(four_pockets[lt->to_index - 1]);
                    // Append the new card to the end of the linked list
                    last_card->next = card_to_move;
                }

                // If a card was exposed, make it visible
                if (exposed_card != NULL) {
                    exposed_card->is_hidden = false;
                }
            } else {
                printf("Move not allowed\n");
            }
        }

        // Display the updated game state
        print_seven_rows(seven_rows, four_pockets);
        cleanup_location_translator(lt);
    }

    // Game won
    printf("\nYou have won!\n");
    printf("Press Enter to exit...");
    getchar(); // Consume newline from previous input
    getchar(); // Wait for Enter key

    // Clean up resources
    cleanup_resources(deck, seven_rows, four_pockets);

    return 0;
}
