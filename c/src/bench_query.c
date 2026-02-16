#include "geode.h"
#include "timer.h"
#include "geohash.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TEST_PRECISION 16
#define TEST_HASH_SZ 2

int main(int argc, char** argv) {

    if (argc != 2) {
        fprintf(stderr,
                "Usage: %s insertions\n"
                "Ex: %s 1000000\n",
                argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    long insertions = atol(argv[1]);

    struct geode *g = geode_create("9x", TEST_PRECISION, TEST_HASH_SZ);
    srand(time(NULL));

    for (int i = 0; i < insertions; ++i) {
        int rand_x = (rand() / ((double) RAND_MAX + 1)) * (g->width + 1);
        int rand_y = (rand() / ((double) RAND_MAX + 1)) * (g->height + 1);
        geode_add_xy(g, rand_x, rand_y);
    }

    struct spatial_range *points = (struct spatial_range*) calloc(3, sizeof(struct spatial_range));
	points[0].latitude = 44.919;
	points[0].longitude = -112.242;

	points[1].latitude = 43.111;
	points[1].longitude = -105.414;

	points[2].latitude = 41.271;
	points[2].longitude = -111.421;

    double start = timer_now();
    polygon_intersects_geode(g, points, 3);
    double end = timer_now();
    printf("%f\n", end - start);

    return 0;
}
