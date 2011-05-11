/**
 * snps -- sliding number puzzles solver
 *
 * Copyright (C) 2011 Oliver Mader <b52@reaktor42.de>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "snps.h"

#include <stdlib.h>
#include <string.h>

#include <glib.h>

/* data type */
typedef struct state {
    struct state *parent;
    unsigned char *board;
    unsigned char size;
    unsigned g, h, f;
} snps_state_t;

/* prototypes */
static snps_state_t *snps_game_start(snps_game_t *game, GHashTable *state_set);
static int snps_state_children_list(snps_state_t *parent,
    snps_game_t *game, GHashTable *state_set, GList **list);
static int snps_state_children_sequence(snps_state_t *parent,
    snps_game_t *game, GHashTable *state_set, GSequence *todo);
static void snps_state_children(snps_state_t *parent, snps_game_t *game,
    GHashTable *state_set, snps_state_t **ret);
static snps_state_t *snps_state_move(snps_state_t *parent, snps_game_t *game,
    GHashTable *state_set, unsigned char p1, unsigned char p2);
static unsigned snps_state_heuristic(snps_state_t *state, snps_game_t *game);
static guint snps_state_hash(gconstpointer a);
static gboolean snps_state_equals(gconstpointer a, gconstpointer b);
static gint snps_state_compare(gconstpointer a, gconstpointer b,
    gpointer user_data);
static void snps_state_free(gpointer data);
static snps_route_t *snps_route_new(snps_state_t *end, snps_game_t *game);

extern snps_game_t *snps_game_new(unsigned rows, unsigned columns,
    const unsigned char *from, const unsigned char *to)
{
    snps_game_t *game = g_slice_new(snps_game_t);
    game->rows = rows;
    game->columns = columns;
    game->size = rows * columns;
    game->from = g_slice_copy(game->size, from);
    game->to = g_slice_copy(game->size, to);

    return game;
}

extern void snps_game_free(snps_game_t *game)
{
    g_slice_free1(game->size, game->from);
    g_slice_free1(game->size, game->to);
    g_slice_free(snps_game_t, game);
}

extern snps_route_t *snps_solve_optimal(snps_game_t *game, snps_stats_f stats)
{
    GHashTable *state_set = g_hash_table_new_full(snps_state_hash,
        snps_state_equals, snps_state_free, NULL);
    GList *level = NULL, *next_level = NULL;

    snps_state_t *start = snps_game_start(game, state_set);
    level = g_list_prepend(level, start);

    int compared = 0;
    int expanded = 1;

    while (42) {
        for (GList *item = level; item != NULL; item = item->next) {
            snps_state_t *current = (snps_state_t *) item->data;

            if (stats != NULL)
                stats(++compared, expanded, current->g);

            if (memcmp(current->board, game->to, game->size) == 0) {
                snps_route_t *route = snps_route_new(current, game);

                g_list_free(level);
                g_list_free(next_level);
                g_hash_table_destroy(state_set);

                return route;
            }

            expanded += snps_state_children_list(current, game, state_set,
                &next_level);
        }

        g_list_free(level);
        level = next_level;
        next_level = NULL;
    }
}

extern snps_route_t *snps_solve_fast(snps_game_t *game, snps_stats_f stats)
{
    GHashTable *state_set = g_hash_table_new_full(snps_state_hash,
        snps_state_equals, snps_state_free, NULL);
    GSequence *todo = g_sequence_new(NULL);

    snps_state_t *start = snps_game_start(game, state_set);
    g_sequence_insert_sorted(todo, start, snps_state_compare, NULL);

    int compared = 0;
    int expanded = 1;

    while (42) {
        GSequenceIter *first = g_sequence_get_begin_iter(todo);
        snps_state_t *current = g_sequence_get(first);
        g_sequence_remove(first);

        if (stats != NULL);
            stats(++compared, expanded, current->g);

        if (memcmp(current->board, game->to, game->size) == 0) {
            snps_route_t *route = snps_route_new(current, game);

            g_sequence_free(todo);
            g_hash_table_destroy(state_set);

            return route;
        }

        expanded += snps_state_children_sequence(current, game, state_set,
            todo);
    }
}

extern void snps_route_free(snps_route_t *route)
{
    for (int i = 0; i < route->length; ++i)
        g_slice_free1(route->size, route->boards[i]);
    g_slice_free1(route->length * sizeof(char *), route->boards);
    g_slice_free1(route->length, route->moves);
    g_slice_free(snps_route_t, route);
}

/* create a first state using the start board */
static snps_state_t *snps_game_start(snps_game_t *game, GHashTable *state_set)
{
    snps_state_t *start = g_slice_new(snps_state_t);
    start->parent = NULL;
    start->size = game->size;
    start->board = g_slice_copy(game->size, game->from);
    start->g = start->h = start->f = 0;

    g_hash_table_insert(state_set, start, start);

    return start;
}

/* add all possible following states to a list */
static int snps_state_children_list(snps_state_t *parent,
    snps_game_t *game, GHashTable *state_set, GList **list)
{
    snps_state_t *children[4] = {NULL, NULL, NULL, NULL};
    snps_state_children(parent, game, state_set, children);

    int count = 0;

    for (int i = 0; i < 4; ++i)
        if (children[i] != NULL) {
            *list = g_list_prepend(*list, children[i]);
            ++count;
        }

    return count;
}

/* add all possible following states to a sequence */
static int snps_state_children_sequence(snps_state_t *parent,
    snps_game_t *game, GHashTable *state_set, GSequence *todo)
{
    snps_state_t *children[4] = {NULL, NULL, NULL, NULL};
    snps_state_children(parent, game, state_set, children);

    int count = 0;

    for (int i = 0; i < 4; ++i)
        if (children[i] != NULL) {
            g_sequence_insert_sorted(todo, children[i], snps_state_compare,
                NULL);
            ++count;
        }

    return count;
}

/* creates all possible following states */
static void snps_state_children(snps_state_t *parent, snps_game_t *game,
    GHashTable *state_set, snps_state_t **ret)
{
    int p = 0;
    while (parent->board[p] != 0)
        ++p;

    int row = TRANSLATE_1D_TO_ROW(p, game->columns);
    int column = TRANSLATE_1D_TO_COLUMN(p, game->columns);
    
    if (column > 0)
        ret[0] = snps_state_move(parent, game, state_set, p,
            TRANSLATE_2D_TO_1D(row, column - 1, game->rows));
    if (column < (game->columns - 1))
        ret[1] = snps_state_move(parent, game, state_set, p,
            TRANSLATE_2D_TO_1D(row, column + 1, game->rows));
    if (row > 0)
        ret[2] = snps_state_move(parent, game, state_set, p,
            TRANSLATE_2D_TO_1D(row - 1, column, game->rows));
    if (row < (game->rows - 1))
        ret[3] = snps_state_move(parent, game, state_set, p,
            TRANSLATE_2D_TO_1D(row + 1, column, game->rows));
}

/* tries to create a new state if it doesn't already exist */
static snps_state_t *snps_state_move(snps_state_t *parent, snps_game_t *game,
    GHashTable *state_set, unsigned char p1, unsigned char p2)
{
    unsigned char board[parent->size];
    memcpy(board, parent->board, parent->size);
    board[p1] = board[p2];
    board[p2] = 0;

    snps_state_t tmp = {
        .board = board,
        .size = parent->size,
    };

    if (g_hash_table_lookup(state_set, &tmp) != NULL)
        return NULL;

    snps_state_t *state = g_slice_new(snps_state_t);
    state->parent = parent;
    state->size = parent->size;
    state->board = g_slice_copy(parent->size, board);
    state->g = parent->g + 1;
    state->h = snps_state_heuristic(state, game);
    state->f = state->g + 2 * state->h;

    g_hash_table_insert(state_set, state, state);

    return state;
}

/* a simple heuristic to rate a state */
static unsigned snps_state_heuristic(snps_state_t *state, snps_game_t *game)
{
    unsigned h = 0;
    unsigned i_current, diff_columns, diff_rows;

    for (int i = 0; i < game->size; ++i) {
        if (game->to[i] == 0 || state->board[i] == game->to[i])
            continue;

        for (i_current = 0; state->board[i_current] != game->to[i];
            ++i_current)
            ;

        diff_columns = abs(TRANSLATE_1D_TO_COLUMN(i, game->columns) - 
            TRANSLATE_1D_TO_COLUMN(i_current, game->columns));
        diff_rows = abs(TRANSLATE_1D_TO_ROW(i, game->columns) - 
            TRANSLATE_1D_TO_ROW(i_current, game->columns));
        
        h += diff_rows + diff_columns;
    }

    return h;
}

/* creates a pseudo hash of a board using the SDBM algorithm */
static guint snps_state_hash(gconstpointer a)
{
    guint hash = 0;
    snps_state_t *state = (snps_state_t *) a;

    for (int i = 0; i < state->size; ++i)
        hash = (guint) state->board[i] + (hash << 6) + (hash << 16) - hash;

    return hash;
}

/* compare two boards on equality */
static gboolean snps_state_equals(gconstpointer a, gconstpointer b)
{
    snps_state_t *state_a = (snps_state_t *) a;
    snps_state_t *state_b = (snps_state_t *) b;

    if (memcmp(state_a->board, state_b->board, state_a->size) == 0)
        return TRUE;

    return FALSE;
}

/* compare two states */
static gint snps_state_compare(gconstpointer a, gconstpointer b,
    gpointer user_data)
{
    snps_state_t *state_a = (snps_state_t *) a;
    snps_state_t *state_b = (snps_state_t *) b;

    if (state_a->f > state_b->f)
        return 1;
    if (state_a->f < state_b->f)
        return -1;

    return 0;
}

/* free a state */
static void snps_state_free(gpointer data)
{
    snps_state_t *state = (snps_state_t *) data;

    g_slice_free1(state->size, state->board);
    g_slice_free(snps_state_t, state);
}

/* create a new route by reversing the final state */
static snps_route_t *snps_route_new(snps_state_t *end, snps_game_t *game)
{
    GList *list = NULL;
    for (snps_state_t *state = end; state != NULL; state = state->parent)
        list = g_list_prepend(list, state);

    snps_route_t *route = g_slice_new(snps_route_t);
    route->length = g_list_length(list);
    route->size = end->size;
    route->boards = g_slice_alloc(route->length * sizeof(char *));
    route->moves = g_slice_alloc(route->length);
    route->moves[route->length - 1] = '\0';

    int i = -1;
    for (GList *item = list; item != NULL; item = item->next) {
        route->boards[++i] = g_slice_copy(end->size,
            ((snps_state_t *) item->data)->board);

        if (item->next == NULL)
            continue;

        snps_state_t *from = (snps_state_t *) item->data;
        snps_state_t *to = (snps_state_t *) item->next->data;

        int from_p = 0;
        while (from->board[from_p] != 0)
            ++from_p;

        int to_p = 0;
        while (to->board[to_p] != 0)
            ++to_p;

        int from_p_row = TRANSLATE_1D_TO_ROW(from_p, game->columns);
        int from_p_column = TRANSLATE_1D_TO_COLUMN(from_p, game->columns);
        int to_p_row = TRANSLATE_1D_TO_ROW(to_p, game->columns);
        int to_p_column = TRANSLATE_1D_TO_COLUMN(to_p, game->columns);
        
        if (from_p_row > to_p_row)
            route->moves[i] = 'U';
        else if (from_p_row < to_p_row)
            route->moves[i] = 'D';
        else if (from_p_column > to_p_column)
            route->moves[i] = 'L';
        else
            route->moves[i] = 'R';
    }

    g_list_free(list);

    return route;
}

/* vim: set expandtab shiftwidth=4 softtabstop=4 textwidth=79: */
