#include <stdbool.h>
#include <stdio.h>

#include "geohash.h"
#include "rtree.h"
#include "timer.h"
#include <stdlib.h>

struct city {
    char *name;
    double lat;
    double lon;
};

/* Number of matches found in the search */
int match_count = 0;

bool city_iter(const double *min, const double *max, const void *item,
               void *udata) {
    match_count++;
    return true;
}

/**
 * Generate a random double between low and high
 */
double drand(double low, double high) {
    return ((double)rand() * (high - low)) / (double)RAND_MAX + low;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr,
                "Usage: %s insertions\n"
                "Ex: %s 1000000\n",
                argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    long insertions = atol(argv[1]);

    struct spatial_range sr;
    geohash_decodeN(&sr, "9x");
    // create a new rtree where each item is a `struct city`.

    struct rtree *tr = rtree_new();

    for (int i = 0; i < insertions; ++i) {
        struct city *p = malloc(sizeof(struct city));
        double rand_x = drand(sr.west, sr.east);
        double rand_y = drand(sr.south, sr.north);
        p->lat = rand_y;
        p->lon = rand_x;
        p->name = "9x";
        double min[] = {rand_x, rand_y};
        double max[] = {rand_x, rand_y};
        rtree_insert(tr, min, max, p);
    }

    match_count = 0;
    double q_min[] = {sr.west, sr.south};
    double q_max[] = {sr.east, sr.north};
    double start = timer_now();
    rtree_search(tr, q_min, q_max, city_iter, NULL);
    printf("%f\n", timer_now() - start);
    printf("Found: %d matches during rtree search\n", match_count);

    rtree_free(tr);
}
