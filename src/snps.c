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

#include "snps-solver.h"

#include <stdio.h>
#include <sys/time.h>

static void show_board(unsigned char *board, unsigned size);
static void show_stats(unsigned states, unsigned depth);

int main(int argc, char *argv[])
{
    unsigned char from[] = {0, 8, 1, 2, 4, 7, 3, 6, 5};
    unsigned char to[] = {1, 2, 3, 4, 5, 6, 7, 8, 0};

    snps_game_t *game = snps_game_new(3, 3, from, to);

    struct timeval ts, te;
    
    gettimeofday(&ts, NULL);
    //snps_route_t *route = snps_solve_optimal(game, show_stats);
    snps_route_t *route = snps_solve_fast(game, show_stats);
    gettimeofday(&te, NULL);

    if (route == NULL) {
        printf("Game not solvable!\n");
    } else {
        float diff = (float) (te.tv_sec - ts.tv_sec) + (te.tv_usec - ts.tv_usec) / 1000000.0;

        printf("\n\nGame solved in %.2f seconds, solution needs %d moves:\n\n",
            diff, route->length - 1);
        
        for (int i = 0; i < route->length; ++i)
            show_board(route->boards[i], route->size);
    }

    snps_route_free(route);
    snps_game_free(game);

    return 0;
}

static void show_board(unsigned char *board, unsigned size)
{
    for (int i = 0; i < size; ++i)
        printf("%i ", board[i]);
    printf("\n");
}

static void show_stats(unsigned states, unsigned depth)
{
    printf("Checking state number %i on level %i ...\r", states, depth);
}

/* vim: set expandtab shiftwidth=4 softtabstop=4 textwidth=79: */
