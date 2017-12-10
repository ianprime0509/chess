#include "chess.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Represents a position on the board.
 */
struct position {
    int rank, file;
};

/**
 * Represents a single move.
 */
struct move {
    /**
     * The current square of the piece to be moved.
     */
    struct position start;
    /**
     * The destination square of the piece to be moved.
     */
    struct position end;
    /**
     * If the piece to be moved is a pawn which should be promoted, the piece to
     * which the pawn should be promoted.
     */
    enum piece_type promotion;
};

/**
 * Attempts to deduce the starting square of a move.
 *
 * The parameters passed to this function give the necessary context to deduce
 * the starting square. There is one peculiarity: the rank and file of the
 * output variable start *must* be initialized to either a pre-determined
 * starting rank or file (in which case they will be untouched by this
 * function), or to -1 (in which case this function will attempt to deduce
 * their values and update them accordingly). It's a little strange, but that's
 * the best way I could think of to do it.
 *
 * @param game The current game to be used as context.
 * @param piece The type of the piece to be moved.
 * @param end The desired destination square (must be a valid position).
 * @param[out] start The starting square (each member must be either a valid
 * rank/file or -1 if unknown).
 * @return An error code.
 */
static int game_deduce_start(const struct game *game, enum piece_type type,
                             struct position end, struct position *start);
/**
 * Returns whether the specified position is reachable from the given starting
 * position.
 *
 * The result returned will be relative to the current (given) game. If there
 * is no piece in the given starting position, the return value will be false.
 * Otherwise, the proposed move will be checked according to the basic movement
 * rules (which directions a piece can move in making sure that e.g. a rook
 * doesn't move through another piece), but will *not* check to ensure that, if
 * there is a piece in the given destination square, it can be captured with
 * the given piece.
 *
 * Special mention should be made of the behavior of this function when the
 * piece in the start position is a pawn: this function will return true if
 * e.g. the start position is e4 (for a white pawn) and the end position is f5,
 * even if there is no piece for the pawn to take in f5. That is, pawns are
 * considered able to move diagonally, even though they can't really do this;
 * if this function is being used to check move legality, the caller must
 * ensure that there is a piece to be captured by the pawn (whether normal
 * capture or en passant) to make the move possible. This behavior, while
 * strange, is desirable since it allows this function to be used when seeing
 * if the destination square is under attack by the piece on the starting
 * square (indeed, this function is used as part of the function that sees if
 * the king is in check).
 *
 * This function is not concerned with the more limited problem of checking
 * whether a move is legal, so it is up to the caller (in that situation) to
 * ensure that a move would not put the king in check or something like that.
 *
 * @param game The current game to be used as context.
 * @param start The starting position (must be a valid position).
 * @param end The ending position (must be a valid position).
 * @return Whether end is reachable by the piece at start.
 */
static bool game_is_reachable(const struct game *game, struct position start,
                              struct position end);
/**
 * Translates a move in algebraic notation to a more convenient form.
 *
 * This will print an error message to stdout if the given move is invalid, with
 * a brief reason why. In the case of an invalid move, the output parameter will
 * be unchanged. The move will be checked according to the method used in
 * game_is_reachable, with the same caveats (e.g. you must check to ensure that
 * the player's king will not be put in check before making the move).
 *
 * Currently, the behavior of this function is designed to be as "tolerant" as
 * possible: it will skip leading whitespace, the 'x' indicating capture will be
 * accepted even if there's nothing to capture, and trailing characters after
 * the move is processed (such as '+' or '#') will be ignored. In a future
 * version, I might decide to emit warnings for these mistakes, but it's
 * unlikely since I'd like to keep things simple.
 *
 * @param game The current game (for context).
 * @param alg The move in algebraic notation.
 * @param[out] move The translated move.
 *
 * @return An error code.
 */
static int game_translate(const struct game *game, const char *alg,
                          struct move *move);

struct piece *piece_new(enum piece_type type, bool is_white)
{
    struct piece *piece = malloc(sizeof *piece);

    piece->type = type;
    piece->is_white = is_white;
    piece->has_moved = false;

    return piece;
}

void game_destroy(struct game *game)
{
    for (int i = 0; i < BOARD_FILES; i++)
        for (int j = 0; j < BOARD_RANKS; j++)
            free(game->board[i][j]);
    free(game);
}

struct game *game_new(void)
{
    struct game *game = malloc(sizeof *game);

    /* Create back rank from left to right */
    game->board[0][0] = piece_new(PIECE_ROOK, true);
    game->board[0][7] = piece_new(PIECE_ROOK, false);
    game->board[1][0] = piece_new(PIECE_KNIGHT, true);
    game->board[1][7] = piece_new(PIECE_KNIGHT, false);
    game->board[2][0] = piece_new(PIECE_BISHOP, true);
    game->board[2][7] = piece_new(PIECE_BISHOP, false);
    game->board[3][0] = piece_new(PIECE_QUEEN, true);
    game->board[3][7] = piece_new(PIECE_QUEEN, false);
    game->board[4][0] = piece_new(PIECE_KING, true);
    game->board[4][7] = piece_new(PIECE_KING, false);
    game->board[5][0] = piece_new(PIECE_BISHOP, true);
    game->board[5][7] = piece_new(PIECE_BISHOP, false);
    game->board[6][0] = piece_new(PIECE_KNIGHT, true);
    game->board[6][7] = piece_new(PIECE_KNIGHT, false);
    game->board[7][0] = piece_new(PIECE_ROOK, true);
    game->board[7][7] = piece_new(PIECE_ROOK, false);
    /* Create pawns */
    for (int i = 0; i < 8; i++) {
        game->board[i][1] = piece_new(PIECE_PAWN, true);
        game->board[i][6] = piece_new(PIECE_PAWN, false);
    }

    game->en_passant = NULL;
    game->white_turn = true;

    return game;
}

int game_move(struct game *game, const char *move)
{
    struct move translated;

    printf("Moving %s\n", move);
    if (game_translate(game, move, &translated) != 0)
        return 1;
    printf("Translated: %c%c to %c%c\n", translated.start.rank + 'a',
           translated.start.file + '1', translated.end.rank + 'a',
           translated.end.file + '1');
    free(game->board[translated.end.rank][translated.end.file]);
    game->board[translated.end.rank][translated.end.file] =
        game->board[translated.start.rank][translated.start.file];
    game->board[translated.start.rank][translated.start.file] = NULL;
    game->en_passant = NULL;
    game->white_turn = !game->white_turn;

    return 0;
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

static int game_deduce_start(const struct game *game, enum piece_type type,
                             struct position end, struct position *start)
{
    /* Number of possibilities found */
    int found = 0;
    struct position new_start;
    struct position test_start = *start;

    if (start->rank != -1 && start->file != -1) {
        struct piece *piece = game->board[start->rank][start->file];
        if (piece != NULL && piece->type == type &&
            piece->is_white == game->white_turn &&
            game_is_reachable(game, *start, end)) {
            found++;
            new_start = *start;
        }
    } else if (start->rank != -1) {
        /* Search in given rank */
        new_start.rank = start->rank;
        for (int f = 0; f < BOARD_FILES; f++) {
            struct piece *piece = game->board[start->rank][f];
            test_start.file = f;
            if (piece != NULL && piece->type == type &&
                piece->is_white == game->white_turn &&
                game_is_reachable(game, test_start, end)) {
                found++;
                new_start.file = f;
            }
        }
    } else if (start->file != -1) {
        /* Search in given file */
        new_start.file = start->file;
        for (int r = 0; r < BOARD_RANKS; r++) {
            struct piece *piece = game->board[r][start->file];
            test_start.rank = r;
            if (piece != NULL && piece->type == type &&
                piece->is_white == game->white_turn &&
                game_is_reachable(game, test_start, end)) {
                found++;
                new_start.rank = r;
            }
        }
    } else {
        /* We need to search both ranks and files */
        for (int r = 0; r < BOARD_RANKS; r++)
            for (int f = 0; f < BOARD_FILES; f++) {
                struct piece *piece = game->board[r][f];
                test_start.rank = r;
                test_start.file = f;
                if (piece != NULL && piece->type == type &&
                    piece->is_white == game->white_turn &&
                    game_is_reachable(game, test_start, end)) {
                    found++;
                    new_start = test_start;
                }
            }
    }

    if (found == 0) {
        printf("No pieces found to perform specified move\n");
        return 1;
    } else if (found == 1) {
        *start = new_start;
        return 0;
    } else {
        printf("Ambiguous move; found %d possibilities\n", found);
        return 1;
    }
}

static bool game_is_reachable(const struct game *game, struct position start,
                              struct position end)
{
    const struct piece *piece = game->board[start.rank][start.file];
    int rank_diff = abs(end.rank - start.rank);
    int file_diff = abs(end.file - start.file);
    int rank_inc = rank_diff == 0 ? 0 : (end.rank - start.rank) / rank_diff;
    int file_inc = file_diff == 0 ? 0 : (end.file - start.file) / file_diff;

    if (!piece)
        return false;
    switch (piece->type) {
    case PIECE_PAWN: {
        /* To simplify things when dealing with white vs black */
        int inc = piece->is_white ? 1 : -1;
        if (rank_diff <= 1 && end.file == start.file + inc)
            return true;
        else if (end.rank == start.rank && end.file == start.file + 2 * inc)
            return !piece->has_moved &&
                   game->board[start.rank][start.file + inc] == NULL;
        else
            return false;
    }
    case PIECE_KNIGHT:
        return (rank_diff == 2 && file_diff == 1) ||
               (rank_diff == 1 && file_diff == 2);
    case PIECE_BISHOP:
        if (rank_diff != file_diff)
            return false;
        break;
    case PIECE_ROOK:
        if (rank_diff != 0 && file_diff != 0)
            return false;
        break;
    case PIECE_QUEEN:
        if (rank_diff != file_diff && rank_diff != 0 && file_diff != 0)
            return false;
        break;
    case PIECE_KING:
        return rank_diff <= 1 && file_diff <= 1;
    }
    /*
     * This is the common part of the "reachable" check, which checks if
     * there
     * are any pieces between the start square and the end square
     * (exclusive).
     * The reason why this is outside the switch statement is because it is
     * the
     * same code for bishops, rooks and queens, so it would be pointless to
     * repeat it in all three cases (the other three cases always return
     * early,
     * so they never reach this point).
     *
     * The requisite checks for direction (e.g. bishops can only move
     * diagonally) are performed in the switch statement before reaching
     * here.
     */
    for (int r = start.rank + rank_inc, f = start.file + file_inc;
         r != end.rank || f != end.file; r += rank_inc, f += file_inc)
        if (game->board[r][f] != NULL)
            return false;
    return true;
}

static int game_translate(const struct game *game, const char *alg,
                          struct move *move)
{
    enum piece_type type;
    /* Keep the initial rank and file at -1 in case none was specified */
    struct position start = {.rank = -1, .file = -1};
    struct position end = {.rank = -1, .file = -1};
    enum piece_type promotion;

    /* Skip leading whitespace */
    while (isspace(*alg))
        alg++;
    /* Look for piece indicator */
    switch (*alg) {
    case 'N':
        type = PIECE_KNIGHT;
        alg++;
        break;
    case 'B':
        type = PIECE_BISHOP;
        alg++;
        break;
    case 'R':
        type = PIECE_ROOK;
        alg++;
        break;
    case 'Q':
        type = PIECE_QUEEN;
        alg++;
        break;
    case 'K':
        type = PIECE_KING;
        alg++;
        break;
    default:
        type = PIECE_PAWN;
        break;
    }
    /* Skip capture indication if present */
    if (*alg == 'x')
        alg++;
    /* First, check to see if there's a starting file specified */
    if ('1' <= *alg && *alg <= '8')
        start.file = *alg++ - '1';
    /* Try to parse the destination rank */
    if ('a' <= *alg && *alg <= 'h') {
        end.rank = *alg++ - 'a';
    } else {
        fprintf(stderr, "Expected rank of destination square\n");
        return 1;
    }
    /* What we just got might have been the starting rank */
    if ('a' <= *alg && *alg <= 'h') {
        start.rank = end.rank;
        end.rank = *alg++ - 'a';
    }
    /* Try to parse the destination file */
    if ('1' <= *alg && *alg <= '8') {
        end.file = *alg++ - '1';
    } else {
        fprintf(stderr, "Expected file of destination square\n");
        return 1;
    }
    /* If there's a rank, the previous rank and file were the starting
     * square */
    if ('a' <= *alg && *alg <= 'h') {
        start.rank = end.rank;
        end.rank = *alg++ - 'a';
        if ('1' <= *alg && *alg <= '8') {
            start.file = end.file;
            end.file = *alg - '1';
        } else {
            fprintf(stderr, "Expected file of destination square\n");
            return 1;
        }
    }
    /* Check for a promotion */
    if (type == PIECE_PAWN && ((game->white_turn && end.rank == 7) ||
                               (!game->white_turn && end.rank == 0))) {
        if (*alg == '=')
            alg++;
        switch (*alg) {
        case 'N':
            promotion = PIECE_KNIGHT;
            break;
        case 'B':
            promotion = PIECE_BISHOP;
            break;
        case 'R':
            promotion = PIECE_ROOK;
            break;
        case 'Q':
            promotion = PIECE_QUEEN;
            break;
        default:
            fprintf(stderr, "Invalid promotion specified\n");
            return 1;
        }
    }
    /* Per the docs, we ignore everything that's left */

    /* Now we need to deduce the end rank and file if they weren't given */
    printf("Trying to find %c to move to %c%c (start %d, %d)\n", type,
           end.rank + 'a', end.file + '1', start.rank, start.file);
    if (game_deduce_start(game, type, end, &start) != 0)
        return 1;

    assert(start.rank >= 0 && start.rank < BOARD_RANKS);
    assert(start.file >= 0 && start.file < BOARD_FILES);
    move->start = start;
    move->end = end;

    return 0;
}
