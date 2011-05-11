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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <libsnps/snps.h>

static void show_stats(unsigned states_c, unsigned states_e, unsigned depth);
static void show_boards(snps_game_t *game, snps_route_t *route);
static void show_board_border(snps_game_t *game, const char *l, const char *m,
    const char *r, unsigned char_width);

/* a simple example how to use the functions provided by snps-solver.h */
int main(int argc, char *argv[])
{
    unsigned rows, cols, algorithm;
    char buffer1[128], buffer2[128], *p1, *p2;

    printf("Board dimensions as COLSxROWS:\n");
    fgets(buffer1, 128, stdin);
    sscanf(buffer1, "%ux%u", &cols, &rows);

    unsigned char from[rows * cols];
    unsigned char to[rows * cols];

    printf("\nStart board as a whitespace seperated list:\n");
    fgets(buffer1, 128, stdin);
    printf("\nEnd board as a whitespace seperated list:\n");
    fgets(buffer2, 128, stdin);

    p1 = buffer1;
    p2 = buffer2;

    for (int i = 0; i < rows * cols; ++i) {
        char *p1_n = p1;
        char *p2_n = p2;

        while (*p1_n != '\0' && *p1_n != '\n' && *p1_n != ' ')
            ++p1_n;
        while (*p2_n != '\0' && *p2_n != '\n' && *p2_n != ' ')
            ++p2_n;

        *p1_n = '\0';
        *p2_n = '\0';

        from[i] = atoi(p1);
        to[i] = atoi(p2);

        p1 = p1_n + 1;
        p2 = p2_n + 1;
    }

    printf("\nSearch algorithm (0: optimal path, 1: fast search):\n");
    fgets(buffer1, 128, stdin);
    sscanf(buffer1, "%u", &algorithm);
    
    snps_game_t *game = snps_game_new(rows, cols, from, to);
    snps_route_t *route = NULL;

    struct timeval ts, te;
    gettimeofday(&ts, NULL);

    if (algorithm == 0) {
        printf("\nUsing a breadth first search with pruning to find the "
            "optimal route:\n");
        route = snps_solve_optimal(game, show_stats);
    } else {
        printf("\nUsing a heuristic based tree search to find a path fast:\n");
        route = snps_solve_fast(game, show_stats);
    }

    gettimeofday(&te, NULL);

    if (route == NULL) {
        printf("Game not solvable!\n");
    } else {
        float diff = (float) (te.tv_sec - ts.tv_sec) +
            (te.tv_usec - ts.tv_usec) / 1000000.0;

        printf("\n\nGame solved in %.4f seconds, path has %d "
            "moves:\n%s\n\nDo you want to see all states? [y/N] ",
            diff, route->length - 1, route->moves);

        fgets(buffer1, 128, stdin);
        if (buffer1[0] == 'y' || buffer1[0] == 'Y')
            show_boards(game, route);

        snps_route_free(route);
    }

    snps_game_free(game);

    return 0;
}

static void show_stats(unsigned states_c, unsigned states_e, unsigned depth)
{
    printf("Checking state number %i/%i on level %i ...\r", states_c, states_e,
        depth);
}

static void show_boards(snps_game_t *game, snps_route_t *route)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    int char_width = (int) log10(route->size - 1) + 1;
    int board_width = (char_width + 2 + 1) * game->columns + 2;
    int terminal_width = w.ws_col;
    int board_parallel = terminal_width / board_width;

    printf("\n");

    for (int i = 0; i < route->length; i += board_parallel) {
        int max_j = (route->length < (i + board_parallel)) ? route->length
            : (i + board_parallel);

        for (int row = 0; row < game->rows; ++row) {
            if (row == 0) {
                for (int j = i; j < max_j; ++j) {
                    char delimiter = (j == max_j - 1) ? '\n' : ' ';
                    show_board_border(game, "┌", "┬", "┐", char_width);
                    putchar(delimiter);
                }
            }

            for (int j = i; j < max_j; ++j) {
                char delimiter = (j == max_j - 1) ? '\n' : ' ';
                int p = -1;
                if (j + 1 != route->length) {
                    p = 0;
                    while (route->boards[j + 1][p] != 0)
                        ++p;
                }
                printf("│");
                for (int column = 0; column < game->columns; ++column) {
                    int p1 = TRANSLATE_2D_TO_1D(row, column, game->rows);
                    putchar(' ');
                    if (route->boards[j][p1] == 0)
                        for (int i = 0; i < char_width; ++i)
                            putchar(' ');
                    else if (p1 == p)
                        printf("\x1b[31;40m%*i\x1b[0m",
                            char_width,route->boards[j][p1]);
                    else
                        printf("%*i", char_width,route->boards[j][p1]);
                    printf(" │");
                    if (column == game->columns - 1)
                        putchar(delimiter);
                }
            }

            for (int j = i; j < max_j; ++j) {
                char delimiter = (j == max_j - 1) ? '\n' : ' ';
                if (row == (game->rows - 1))
                    show_board_border(game, "└", "┴", "┘", char_width);
                else
                    show_board_border(game, "├", "┼", "┤", char_width);
                putchar(delimiter);
            }
        }
    }
}

static void show_board_border(snps_game_t *game, const char *l, const char *m,
    const char *r, unsigned char_width)
{
    printf("%s", l);
    for (int column = 0; column < game->columns; ++column) {
        for (int i = 0; i < char_width + 2; ++i)
            printf("─");
        printf("%s", (column == game->columns -1) ? r : m);
    }
}

/* vim: set expandtab shiftwidth=4 softtabstop=4 textwidth=79: */
