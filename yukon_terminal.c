#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "yukon_logic.h" // Include the game logic header

// Game states
typedef enum {
    STATE_INITIAL,    // Initial state, waiting for LD command
    STATE_DECK_LOADED, // Deck loaded with LD, waiting for SW command
    STATE_DECK_SHOWN,  // Deck shown with SW, waiting for OK command
    STATE_GAME_STARTED // Game started, normal gameplay
} GameState;

// Function prototypes for terminal-specific functions
void print_deck(Card* deck);
void print_deck_grid(Card* deck, bool show_faces);
void print_seven_rows(Card** seven_rows, Card** four_pockets);


// Terminal-specific functions

// Print deck in standard format (not used in the current implementation)
void print_deck(Card* deck) {
	const char* suits[] = { "Hearts", "Diamonds", "Clubs", "Spades" };
	const char* values[] = { "Ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King" };
	while (deck != NULL)
	{
		printf("%s of %s\n", values[deck->value - 1], suits[deck->suit - 1]); deck = deck->next;
	}
}

// Print deck in a 7x8 grid format (similar to the game layout)
void print_deck_grid(Card* deck, bool show_faces) {
    const char* suits[] = { "H", "D", "C", "S" };
    const char* values[] = { "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K" };

    // Calculate how many cards per column (approximately 7-8 cards per column)
    int total_cards = 52;
    int cards_per_column = total_cards / 7;
    if (total_cards % 7 > 0) {
        cards_per_column++;
    }

    printf("  C1    C2    C3    C4    C5    C6    C7    F1    F2    F3    F4\n");

    // Create a temporary array to hold the cards
    Card* card_grid[7][8]; // 7 columns, up to 8 cards per column
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 8; j++) {
            card_grid[i][j] = NULL;
        }
    }

    // Fill the grid with cards from the deck
    Card* current = deck;
    int col = 0;
    int row = 0;

    while (current != NULL) {
        card_grid[col][row] = current;
        col++;
        if (col >= 7) {
            col = 0;
            row++;
        }
        current = current->next;
    }

    // Print the grid
    for (int j = 0; j < 8; j++) {
        for (int i = 0; i < 7; i++) {
            if (card_grid[i][j] != NULL) {
                if (show_faces) {
                    // Show face-up card
                    char value_str[4];
                    get_value_str(card_grid[i][j]->value, value_str);
                    char suit_char = "HDCS"[card_grid[i][j]->suit - 1];
                    printf("%2s%-2s", value_str, suits[card_grid[i][j]->suit - 1]);
                } else {
                    // Show hidden card
                    printf("%2s%-2s", "[", "]");
                }
            } else {
                printf("    ");
            }
            printf("  ");
        }

        // Print empty foundation piles
        for (int i = 0; i < 4; i++) {
            printf("    ");
            printf("  ");
        }

        printf("\n");
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

    // Set initial game state
    GameState game_state = STATE_INITIAL;

    // Variabler til statuslinjen
    char last_command[50] = "None";
    char message[100] = "Welcome to Group 41 Yukon Solitaire. Type LD to load deck.";

    // Clear screen
    system("cls");

    // Display the initial status line
    display_status_line(last_command, message);

    // Main game loop
    while (true) {
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

            // Reset game state
            game_state = STATE_INITIAL;

            // Nulstil statuslinjen
            strcpy(last_command, "None");
            strcpy(message, "Welcome to Group 41 Yukon Solitaire. Type LD to load deck.");

            // Clear screen and display status
            system("cls");
            display_status_line(last_command, message);

            // Fortsæt spillet
            continue;
        }

        // Handle commands based on current game state
        if (game_state == STATE_INITIAL) {
            // In initial state, only accept LD command
            if (strcmp(read_from_console, "LD") == 0 || strcmp(read_from_console, "ld") == 0) {
                // Clean up any existing deck
                if (deck != NULL) {
                    free_card_list(deck);
                    deck = NULL;
                }

                // Create an ordered deck with all cards hidden
                deck = create_ordered_deck();

                // Update game state
                game_state = STATE_DECK_LOADED;

                // Clear screen
                system("cls");

                // Print the deck with hidden cards in a grid format
                printf("Loaded deck (all cards hidden):\n");
                print_deck_grid(deck, false); // false = don't show faces
                printf("\nMessage: OK\n");

                // Display status line
                strcpy(message, "OK - Type SW to show deck");
                display_status_line(last_command, message);
            } else {
                // Invalid command for this state
                strcpy(message, "Invalid command. Type LD to load deck.");
                system("cls");
                display_status_line(last_command, message);
            }
        }
        else if (game_state == STATE_DECK_LOADED) {
            // In deck loaded state, only accept SW command
            if (strcmp(read_from_console, "SW") == 0 || strcmp(read_from_console, "sw") == 0) {
                // Clear screen
                system("cls");

                // Print the deck with all cards face-up in a grid format
                printf("Current deck (all cards face-up):\n");
                print_deck_grid(deck, true); // true = show faces
                printf("\nMessage: OK\n");

                // Update game state
                game_state = STATE_DECK_SHOWN;

                // Display status line
                strcpy(message, "OK - Type OK to start game");
                display_status_line(last_command, message);
            } else {
                // Invalid command for this state
                strcpy(message, "Invalid command. Type SW to show deck.");
                system("cls");
                display_status_line(last_command, message);
            }
        }
        else if (game_state == STATE_DECK_SHOWN) {
            // In deck shown state, only accept OK command
            if (strcmp(read_from_console, "OK") == 0 || strcmp(read_from_console, "ok") == 0) {
                // Initialize the game using the logic component
                // First clean up any existing game
                cleanup_resources(deck, seven_rows, four_pockets);

                // Reset pointers
                deck = NULL;
                for (int i = 0; i < 7; i++) {
                    seven_rows[i] = NULL;
                }
                for (int i = 0; i < 4; i++) {
                    four_pockets[i] = NULL;
                }

                // Initialize new game
                initialize_game(&deck, seven_rows, four_pockets);

                // Update game state
                game_state = STATE_GAME_STARTED;

                // Clear screen
                system("cls");

                // Display the initial game state
                print_seven_rows(seven_rows, four_pockets);
                strcpy(message, "Game started");
                display_status_line(last_command, message);
            } else {
                // Invalid command for this state
                strcpy(message, "Invalid command. Type OK to start game.");
                system("cls");
                printf("Current deck (all cards face-up):\n");
                print_deck_grid(deck, true); // true = show faces
                printf("\nMessage: OK\n");
                display_status_line(last_command, message);
            }
        }
        else if (game_state == STATE_GAME_STARTED) {
            // In game started state, process normal game commands
            if (!process_command(read_from_console, seven_rows, four_pockets)) {
                strcpy(message, "Move not allowed");
            } else {
                strcpy(message, "Move successful");
            }

            // Check if game is won
            if (is_seven_rows_empty(seven_rows)) {
                strcpy(message, "Congratulations! You have won!");
                system("cls");
                print_seven_rows(seven_rows, four_pockets);
                display_status_line(last_command, message);

                printf("\nPress Enter to exit...");
                getchar(); // Consume newline from previous input
                getchar(); // Wait for Enter key

                // Clean up resources
                cleanup_resources(deck, seven_rows, four_pockets);

                return 0;
            }

            // Display the updated game state
            system("cls");
            print_seven_rows(seven_rows, four_pockets);
            display_status_line(last_command, message);
        }
    }

    // This code is unreachable because we handle the game won condition in the game loop
    // and return from there, but we'll keep it for safety

    // Clean up resources
    cleanup_resources(deck, seven_rows, four_pockets);

    return 0;
}
