#include "chess.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

struct piece *piece_create(enum piece_type type, bool is_white)
{
    struct piece *piece = malloc(sizeof *piece);

    piece->type = type;
    piece->is_white = is_white;
    piece->has_moved = false;

    return piece;
}

struct game *game_create(void)
{
    struct game *game = malloc(sizeof *game);

    /* Create back rank from left to right */
    game->board[0][0] = piece_create(PIECE_ROOK, true);
    game->board[0][7] = piece_create(PIECE_ROOK, false);
    game->board[1][0] = piece_create(PIECE_KNIGHT, true);
    game->board[1][7] = piece_create(PIECE_KNIGHT, false);
    game->board[2][0] = piece_create(PIECE_BISHOP, true);
    game->board[2][7] = piece_create(PIECE_BISHOP, false);
    game->board[3][0] = piece_create(PIECE_QUEEN, true);
    game->board[3][7] = piece_create(PIECE_QUEEN, false);
    game->board[4][0] = piece_create(PIECE_KING, true);
    game->board[4][7] = piece_create(PIECE_KING, false);
    game->board[5][0] = piece_create(PIECE_BISHOP, true);
    game->board[5][7] = piece_create(PIECE_BISHOP, false);
    game->board[6][0] = piece_create(PIECE_KNIGHT, true);
    game->board[6][7] = piece_create(PIECE_KNIGHT, false);
    game->board[7][0] = piece_create(PIECE_ROOK, true);
    game->board[7][7] = piece_create(PIECE_ROOK, false);
    /* Create pawns */
    for (int i = 0; i < 8; i++) {
        game->board[i][1] = piece_create(PIECE_PAWN, true);
        game->board[i][6] = piece_create(PIECE_PAWN, false);
    }

    game->en_passant = NULL;

    return game;
}

void game_print_board(struct game *game)
{
    printf("  a b c d e f g h\n");
    for (int j = 7; j >= 0; j--) {
        printf("%d", j + 1);
        for (int i = 0; i < 8; i++)
            if (game->board[i][j])
                printf(" %c", game->board[i][j]->is_white
                                  ? (char)game->board[i][j]->type
                                  : tolower(game->board[i][j]->type));
            else
                printf(" *");
        putchar('\n');
    }
}
