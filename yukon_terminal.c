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






// Funktion til at vise statuslinjen i bunden
void display_status_line(const char* last_command, const char* message) {
    printf("\n----------------------------------------\n");
    printf("Last Command: %s | Message: %s\n", last_command, message);
    printf("Input > ");
}

int main()
{
    srand(time(NULL));

    // Initialize game components
    Card* deck = NULL;
    Card* seven_rows[7] = { NULL };
    Card* four_pockets[4] = { NULL };

    // Variabler til statuslinjen
    char last_command[50] = "None";
    char message[100] = "Welcome to Group 41 Yukon Solitaire";

    // Initialize the game using the logic component
    initialize_game(&deck, seven_rows, four_pockets);

    // Display the initial game state
    print_seven_rows(seven_rows, four_pockets);
    display_status_line(last_command, message);

    // Main game loop
    while (!is_seven_rows_empty(seven_rows)) {
        char read_from_console[20];
        scanf("%s", read_from_console);

        // Gem den sidste kommando
        strcpy(last_command, read_from_console);

        // Check for QQ command to quit the program
        if (strcmp(read_from_console, "QQ") == 0 || strcmp(read_from_console, "qq") == 0) {
            strcpy(message, "Quitting the game...");
            printf("\n%s\n", message);
            cleanup_resources(deck, seven_rows, four_pockets);
            return 0;
        }

        // Check for Q command to restart the game
        if (strcmp(read_from_console, "Q") == 0 || strcmp(read_from_console, "q") == 0) {
            strcpy(message, "Restarting the game...");
            printf("\n%s\n", message);

            // Ryd skærmen helt
            system("cls");
            printf("Restarting game...\n");


            // Ryd alt data fuldstændigt
            cleanup_resources(deck, seven_rows, four_pockets);

            // Nulstil alle pointere
            deck = NULL;
            for (int i = 0; i < 7; i++) {
                seven_rows[i] = NULL;
            }
            for (int i = 0; i < 4; i++) {
                four_pockets[i] = NULL;
            }

            // Nulstil statuslinjen
            strcpy(last_command, "None");
            strcpy(message, "Welcome to Yukon Solitaire");

            // Initialiser spillet igen
            initialize_game(&deck, seven_rows, four_pockets);

            // Vis det nye spil
            print_seven_rows(seven_rows, four_pockets);
            display_status_line(last_command, message);

            // Fortsæt spillet
            continue;
        }

        // Process the command using the shared logic function
        if (!process_command(read_from_console, seven_rows, four_pockets)) {
            strcpy(message, "Move not allowed");
        } else {
            strcpy(message, "Move successful");
        }

        // Display the updated game state
        system("cls"); // Ryd skærmen (Windows)
        print_seven_rows(seven_rows, four_pockets);
        display_status_line(last_command, message);
    }

    // Game won
    strcpy(message, "Congratulations! You have won!");
    system("cls"); // Ryd skærmen (Windows)
    print_seven_rows(seven_rows, four_pockets);
    display_status_line(last_command, message);

    printf("\nPress Enter to exit...");
    getchar(); // Consume newline from previous input
    getchar(); // Wait for Enter key

    // Clean up resources
    cleanup_resources(deck, seven_rows, four_pockets);

    return 0;
}
