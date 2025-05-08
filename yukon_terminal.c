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
void print_deck_grid(Card* deck, bool show_faces);
void print_seven_rows(Card** seven_rows, Card** four_pockets);


// Terminal-specific functions


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
    printf("Commands: LD (load deck), LD deck (load deck from file), SW (show deck), SD (save deck), SI (interleave shuffle), SI <split> (custom interleave), P (play game), Q (restart), QQ (quit)\n");
    printf("Game Commands: C1:AH->C2 (move Ace of Hearts from column 1 to column 2)\n");
    printf("File Commands: SV (save game), LD filename (load saved game)\n");
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
        char read_from_console[100]; // Større buffer til at håndtere længere kommandoer

        // Læs hele linjen inklusive mellemrum
        printf("Input > ");
        fgets(read_from_console, sizeof(read_from_console), stdin);

        // Fjern newline-tegnet fra slutningen af strengen
        size_t len = strlen(read_from_console);
        if (len > 0 && read_from_console[len-1] == '\n') {
            read_from_console[len-1] = '\0';
        }

        // Gem den sidste kommando
        strcpy(last_command, read_from_console);

        // Check for QQ command to quit the program
        if (strcmp(read_from_console, "QQ") == 0 || strcmp(read_from_console, "qq") == 0) {
            strcpy(message, "Quitting the game...");
            printf("\n%s\n", message);
            cleanup_resources(deck, seven_rows, four_pockets);
            return 0;
        }

        // Check for SV command to save the game
        if (strcmp(read_from_console, "SV") == 0 || strcmp(read_from_console, "sv") == 0) {
            // Only allow saving when the game has started
            if (game_state == STATE_GAME_STARTED) {
                char filename[100];
                printf("Enter filename to save game: ");

                // Læs filnavnet med fgets
                fgets(filename, sizeof(filename), stdin);

                // Fjern newline-tegnet fra slutningen af strengen
                size_t len = strlen(filename);
                if (len > 0 && filename[len-1] == '\n') {
                    filename[len-1] = '\0';
                }

                if (save_game_to_file(filename, deck, seven_rows, four_pockets)) {
                    strcpy(message, "Game saved successfully");
                } else {
                    strcpy(message, "Error saving game");
                }

                // Display the updated game state
                system("cls");
                print_seven_rows(seven_rows, four_pockets);
                display_status_line(last_command, message);
            } else {
                strcpy(message, "Cannot save game in current state");
                system("cls");
                display_status_line(last_command, message);
            }
            continue;
        }

        // Check for LD command with a filename to load a saved game
        if ((strncmp(read_from_console, "LD ", 3) == 0 || strncmp(read_from_console, "ld ", 3) == 0) ||
            (strncmp(read_from_console, "LD", 2) == 0 && strlen(read_from_console) > 2) ||
            (strncmp(read_from_console, "ld", 2) == 0 && strlen(read_from_console) > 2)) {

            char* filename;
            if (read_from_console[2] == ' ') {
                filename = read_from_console + 3; // Skip "LD " to get the filename
            } else {
                filename = read_from_console + 2; // Skip "LD" to get the filename
            }

            // Clean up any existing game
            cleanup_resources(deck, seven_rows, four_pockets);

            // Reset pointers
            deck = NULL;
            for (int i = 0; i < 7; i++) {
                seven_rows[i] = NULL;
            }
            for (int i = 0; i < 4; i++) {
                four_pockets[i] = NULL;
            }

            if (load_game_from_file(filename, &deck, seven_rows, four_pockets)) {
                // Hvis filnavnet er "deck", skal vi behandle det som et deck, ikke et spil
                if (strcmp(filename, "deck") == 0) {
                    // Vi vil kun have deck'et, ikke spiltilstanden
                    // Ryd eventuel indlæst spiltilstand
                    for (int i = 0; i < 7; i++) {
                        free_card_list(seven_rows[i]);
                        seven_rows[i] = NULL;
                    }
                    for (int i = 0; i < 4; i++) {
                        free_card_list(four_pockets[i]);
                        four_pockets[i] = NULL;
                    }

                    // Sørg for at alle kort i deck'et er skjulte
                    Card* current = deck;
                    while (current != NULL) {
                        current->is_hidden = true;
                        current = current->next;
                    }

                    // Opdater spiltilstand
                    game_state = STATE_DECK_LOADED;

                    // Ryd skærmen
                    system("cls");

                    // Vis deck'et med skjulte kort i et gitterformat
                    printf("Loaded deck from file (all cards hidden):\n");
                    print_deck_grid(deck, false); // false = don't show faces
                    printf("\nMessage: OK\n");

                    strcpy(message, "OK - Type SW to show deck");
                } else {
                    // Normal game load
                    // Update game state
                    game_state = STATE_GAME_STARTED;

                    strcpy(message, "Game loaded successfully");

                    // Display the loaded game state
                    system("cls");
                    print_seven_rows(seven_rows, four_pockets);
                }

                display_status_line(last_command, message);
            } else {
                strcpy(message, "Error loading game");

                // Reset to initial state
                game_state = STATE_INITIAL;

                // Display status
                system("cls");
                display_status_line(last_command, message);
            }
            continue;
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
            // In initial state, accept LD command (without filename)
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
            }
            // "LD deck" kommandoen håndteres nu i den generelle "LD filnavn" kommando
            else {
                // Invalid command for this state
                strcpy(message, "Invalid command. Type LD to load deck or LD filename to load a saved game.");
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
                strcpy(message, "OK - Type P to start game");
                display_status_line(last_command, message);
            } else {
                // Invalid command for this state
                strcpy(message, "Invalid command. Type SW to show deck.");
                system("cls");

                // Vis altid deck'et, selv ved ugyldig kommando
                printf("Current deck (all cards hidden):\n");
                print_deck_grid(deck, false); // false = don't show faces
                printf("\nMessage: %s\n", message);

                display_status_line(last_command, message);
            }
        }
        else if (game_state == STATE_DECK_SHOWN) {
            // In deck shown state, accept OK command, SD (Save Deck) command, or SI (Interleave Shuffle) command
            if (strcmp(read_from_console, "SI") == 0 || strcmp(read_from_console, "si") == 0) {
                // Interleave shuffle with default split (26 cards)
                interleave_shuffle(&deck, 26);

                // Clear screen
                system("cls");

                // Print the deck with all cards hidden in a grid format
                printf("Current deck after interleave shuffle (all cards hidden):\n");
                print_deck_grid(deck, false); // false = don't show faces
                printf("\nMessage: OK\n");

                // Update game state to reflect that cards are now hidden
                game_state = STATE_DECK_LOADED;

                // Display status line
                strcpy(message, "Deck interleaved with default split (26 cards). Type SW to show deck.");
                display_status_line(last_command, message);
            }
            else if (strncmp(read_from_console, "SI ", 3) == 0 || strncmp(read_from_console, "si ", 3) == 0) {
                // Interleave shuffle with custom split
                int split = atoi(read_from_console + 3);
                interleave_shuffle(&deck, split);

                // Clear screen
                system("cls");

                // Print the deck with all cards hidden in a grid format
                printf("Current deck after interleave shuffle (all cards hidden):\n");
                print_deck_grid(deck, false); // false = don't show faces
                printf("\nMessage: OK\n");

                // Update game state to reflect that cards are now hidden
                game_state = STATE_DECK_LOADED;

                // Display status line
                char split_message[100];
                sprintf(split_message, "Deck interleaved with custom split (%d cards). Type SW to show deck.", split);
                strcpy(message, split_message);
                display_status_line(last_command, message);
            }
            else if (strcmp(read_from_console, "SD") == 0 || strcmp(read_from_console, "sd") == 0) {
                // Save the current deck to a file
                char filename[100];
                printf("Enter filename to save deck (default: deck): ");

                // Læs filnavnet med fgets
                fgets(filename, sizeof(filename), stdin);

                // Fjern newline-tegnet fra slutningen af strengen
                size_t len = strlen(filename);
                if (len > 0 && filename[len-1] == '\n') {
                    filename[len-1] = '\0';
                }

                // Hvis filnavnet er tomt, brug "deck" som standard
                if (strlen(filename) == 0) {
                    strcpy(filename, "deck");
                }

                // Gem deck'et i det nye format (et kort per linje)
                FILE* file = fopen(filename, "w");
                if (!file) {
                    strcpy(message, "Error opening file for writing");
                } else {
                    Card* current = deck;
                    // Ingen fejlhåndtering nødvendig her

                    while (current != NULL) {
                        // Konverter værdi til bogstav/tal
                        char value_str[3];
                        if (current->value == 1) {
                            strcpy(value_str, "A");
                        } else if (current->value == 11) {
                            strcpy(value_str, "J");
                        } else if (current->value == 12) {
                            strcpy(value_str, "Q");
                        } else if (current->value == 13) {
                            strcpy(value_str, "K");
                        } else {
                            sprintf(value_str, "%d", current->value);
                        }

                        // Konverter kulør til bogstav
                        char suit_char;
                        switch (current->suit) {
                            case 1: suit_char = 'H'; break; // Hearts
                            case 2: suit_char = 'D'; break; // Diamonds
                            case 3: suit_char = 'C'; break; // Clubs
                            case 4: suit_char = 'S'; break; // Spades
                            default: suit_char = '?'; break;
                        }

                        // Skriv kortet til filen
                        fprintf(file, "%s%c\n", value_str, suit_char);

                        current = current->next;
                    }

                    fclose(file);
                    strcpy(message, "Deck saved successfully");
                }

                // Clear screen
                system("cls");

                // Print the deck with all cards face-up in a grid format
                printf("Current deck (all cards face-up):\n");
                print_deck_grid(deck, true); // true = show faces
                printf("\nMessage: %s\n", message);

                // Display status line
                display_status_line(last_command, message);
            }
            else if (strcmp(read_from_console, "P") == 0 || strcmp(read_from_console, "p") == 0) {
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
                strcpy(message, "Invalid command. Type P to start game.");
                system("cls");

                // Vis altid deck'et, selv ved ugyldig kommando
                printf("Current deck (all cards face-up):\n");
                print_deck_grid(deck, true); // true = show faces
                printf("\nMessage: %s\n", message);

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
