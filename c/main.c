#include <stdio.h>
#include <string.h>

#include "chess.h"

/**
 * Reads input from stdin and stores it in the given buffer.
 *
 * @return The length of the input (not including the null-terminator).
 */
static size_t read_input(char *buf, size_t max);

int main(void)
{
    char input[100];
    struct game *game = game_new();

    for (;;) {
        game_print_board(game);
        if (game->white_turn)
            printf("White to move: ");
        else
            printf("Black to move: ");
        fflush(stdout);
        read_input(input, sizeof input);
        if (!strcmp(input, "quit")) {
            puts("Goodbye!");
            break;
        } else {
            game_move(game, input);
        }
    }

    game_destroy(game);
    return 0;
}

static size_t read_input(char *buf, size_t max)
{
    size_t len = 0;
    char c;

    /* Make sure the condition allows the null-terminator at the end */
    while (--max > 0 && (c = getchar()) != '\n' && c != EOF)
        buf[len++] = c;
    buf[len] = '\0';
    /* If the input was too long, flush the input until the next line */
    if (max == 0 && c != EOF)
        while ((c = getchar()) != '\n' && c != EOF)
            ;

    return len;
}
