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

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include <libsnps/snps.h>

static int statesc = 0, statese = 0;
static long long total_statesc = 0, total_statese = 0;
static double total_time = 0;
static long long total_length = 0;

static void show_stats(unsigned states_c, unsigned states_e, unsigned depth)
{
    statesc = states_c;
    statese = states_e;
}

int main(int argc, const char *argv[])
{
    char buffer[1024];
    unsigned char from[16];
    unsigned char to[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        0};
    struct timeval ts, te;
    int i = 0;

    while (fgets(buffer, 1024, stdin) != NULL) {
        for (int i = 0; i < 16; ++i)
            if (buffer[i] >= '0' && buffer[i] <= '9')
                from[i] = buffer[i] - '0';
            else
                from[i] = buffer[i] - 'A' + 10;
        
        snps_game_t *game = snps_game_new(4, 4, from, to);

        gettimeofday(&ts, NULL);
        snps_route_t *route = snps_solve_fast(game, show_stats);
        gettimeofday(&te, NULL);

        printf("%04i: ", ++i);

        if (route == NULL) {
            printf("Not solvable!\n");
        } else {
            float diff = (float) (te.tv_sec - ts.tv_sec) + (te.tv_usec -
                ts.tv_usec) / 1000000.0;

            printf("Solved: Path Length = %i ; Time = %.4fs ; "
                "States = %i/%i\n", route->length, diff, statesc, statese);

            total_time += diff;
            total_length += route->length;
            total_statesc += statesc;
            total_statese += statese;
        }

        if (route != NULL)
            snps_route_free(route);
        snps_game_free(game);
    }

    printf("\nMean: Path Length = %lli ; Time = %.4fs ; States = %lli/%lli\n",
        total_length/i, total_time/(double)i, total_statesc/i, total_statese/i);
    printf("Total Time = %.4fs\n", total_time);

    return 0;
}

