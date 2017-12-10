/**
 * @file
 * The backend of the chess program.
 */
#ifndef CHESS_CHESS_H
#define CHESS_CHESS_H

#include <stdbool.h>

/**
 * The number of ranks (rows) on the board.
 */
#define BOARD_RANKS 8
/**
 * The number of files (columns) on the board.
 */
#define BOARD_FILES 8

/**
 * The type of a piece.
 */
enum piece_type {
    PIECE_PAWN = 'P',
    PIECE_KNIGHT = 'N',
    PIECE_BISHOP = 'B',
    PIECE_ROOK = 'R',
    PIECE_QUEEN = 'Q',
    PIECE_KING = 'K'
};

/**
 * A game piece.
 */
struct piece {
    /**
     * The type of the piece.
     */
    enum piece_type type;
    /**
     * Whether the piece belongs to white (if false, it belongs to black).
     */
    bool is_white;
    /**
     * Whether the piece has been moved in the current game.
     */
    bool has_moved;
};

/**
 * The state of the game.
 */
struct game {
    /**
     * The pieces on the board.
     *
     * We use pointers so that NULL can represent there being no piece on that
     * space. The indexing used is intended to be consistent with algebraic
     * notation, so that the square b7 would be board[1][6] (note the
     * 0-indexing).
     */
    struct piece *board[BOARD_FILES][BOARD_RANKS];
    /**
     * Pointer to a pawn eligible to be taken en passant.
     *
     * This should be set to point to a pawn that moves two spaces forward, and
     * should be unset (to NULL) on the next turn. Keeping a pointer like this
     * seemed to me to be the most efficient and natural way to handle en
     * passant.
     */
    struct piece *en_passant;
    /**
     * Whether it is white's turn.
     */
    bool white_turn;
};

/**
 * Creates a piece with the given type and color.
 */
struct piece *piece_new(enum piece_type type, bool is_white);

/**
 * Initializes a new game with the standard starting board.
 */
struct game *game_new(void);
/**
 * Destroys the given game, freeing all its memory.
 */
void game_destroy(struct game *game);
/**
 * Processes the given move (in algebraic notation) and performs it if possible.
 *
 * If the move is impossible (illegal), an error message will be printed to
 * stdout with a description of the cause.
 *
 * @return An error code (e.g. if the move was illegal or if the move was in an
 * invalid format).
 */
int game_move(struct game *game, const char *move);
/**
 * Pretty-prints the current game board to stdout.
 */
void game_print_board(struct game *game);

#endif
