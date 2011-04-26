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

#ifndef SNPS_SOLVER_H
#define SNPS_SOLVER_H

/* datatypes */
typedef struct {
    unsigned char rows;
    unsigned char columns;
    unsigned char size;
    unsigned char *from;
    unsigned char *to;
} snps_game_t;

typedef struct {
    unsigned char **boards;
    unsigned size;
    unsigned length;
} snps_route_t;

typedef void (*snps_stats_f)(unsigned states, unsigned depth);

/* coordinate conversion */
#define TRANSLATE_2D_TO_1D(row, column, rows) ((row) * (rows) + (column))
#define TRANSLATE_1D_TO_ROW(position, columns) ((position) / (columns))
#define TRANSLATE_1D_TO_COLUMN(position, columns) ((position) % (columns))

/* allocate a new game which may be solved */
extern snps_game_t *snps_game_new(unsigned rows, unsigned columns,
    const unsigned char *from, const unsigned char *to);
/* test if a game is solvable using the parity of the start and the end
   state, 0 means solvable */
extern int snps_game_solvable(snps_game_t *game);
/* free a game instance */
extern void snps_game_free(snps_game_t *game);

/* a simple breadth first solving algorithm which utilises pruning to find
   the optimal route */
extern snps_route_t *snps_solve_optimal(snps_game_t *game, snps_stats_f stats);
/* a faster approach using a heuristic and a sorted list to find a route,
   don't have to be the optimal route! */
extern snps_route_t *snps_solve_fast(snps_game_t *game, snps_stats_f stats);

/* free a route instance */
extern void snps_route_free(snps_route_t *route);

#endif

/* vim: set expandtab shiftwidth=4 softtabstop=4 textwidth=79: */
