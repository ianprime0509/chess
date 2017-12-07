#include <stdio.h>

#include "chess.h"

int main(void)
{
    struct game *game = game_create();

    game_print_board(game);

    return 0;
}
