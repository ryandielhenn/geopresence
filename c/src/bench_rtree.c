#include <stdio.h>
#include <stdlib.h>
#include "geohash.h"
#include "rtree.h"
#include "timer.h"

struct city {
    char *name;
    double lat;
    double lon;
};

/* Number of matches found in the search */
int match_count;

bool city_iter(const double *rect, const void *item, void *udata) {
    match_count++;
    // This always prints 0.0 0.0 - the lat/lon values get lost somewhere?
    return true;
}

/**
 * Generate a random double between low and high
 */
double drand(double low, double high) {
    return ((double)rand() * ( high - low )) / (double)RAND_MAX + low;
}

int main(int argc, char** argv) {
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
    struct rtree *tr = rtree_new(sizeof(struct city), 2);

    for (int i = 0; i < insertions; ++i) {
        struct city *p = malloc(sizeof(struct city));
        double rand_x = drand(sr.west, sr.east);
        double rand_y = drand(sr.south, sr.north);
        p->lat = rand_y;
        p->lon = rand_x;
        p->name = "9x";
        //uncomment to print randomly generated lat/lon coordinates in 9x
        //printf("%f %f\n", rand_y, rand_x);
        rtree_insert(tr, (double[]){ rand_x, rand_y, rand_x, rand_y } , p);
    }

    // This finds none of what was inserted even though everything that was inserted
    // should be in the bounds of this rectangle
    match_count = 0;
    double start = timer_now();
    rtree_search(tr, (double[]){sr.west, sr.south, sr.east, sr.north}, city_iter, NULL);
    printf("%f\n", timer_now() - start);


    rtree_free(tr);
}
