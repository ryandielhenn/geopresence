#include "geode.h"
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#define TEST_PRECISION 16
#define TEST_HASH_SZ 2

struct geode *instances = NULL;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr,
                "Usage: %s geohash_file.txt\n"
                "Ex: %s ../datasets/geohashes.txt\n",
                argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    printf("Opening %s\n", argv[1]);
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    char line[128];
    while (fgets(line, 128, fp) != NULL) {
        char prefix[TEST_HASH_SZ + 1] = { '\0' };
        memcpy(prefix, line, TEST_HASH_SZ);
        struct geode *instance;

        HASH_FIND_STR(instances, prefix, instance);
        if (instance == NULL) {
            instance = geode_create(line, TEST_PRECISION, TEST_HASH_SZ);
            HASH_ADD_STR(instances, prefix, instance);
        }
        geode_add_geohash(instance, line);
    }

	struct geode *g;
    struct geode *sg;
    for(g=instances; g != NULL; g=g->hh.next) {
		const uint64_t estimate = HLL_estimate(g->hll);
		printf("-----------------------------\n");
		printf("Hash: %s\nTotal: %" PRIu64 "\nUnique: %" PRIu64 "\n", g->prefix, g->total, estimate);
        printf("Load factor: %f\n", geode_load_factor(g));
        for (int i = 0; i < g->num_sgs; i++) {
            sg = g->sgs[i];
		    const uint64_t estimate = HLL_estimate(sg->hll);
            printf("\t-----------------------------\n");
            printf("\tHash: %s\n\tTotal: %" PRIu64 "\n\tUnique: %" PRIu64 "\n", sg->prefix, sg->total, estimate);
            printf("\tLoad factor: %f\n", geode_load_factor(sg));
        }
    }

    fclose(fp);
    return 0;
}
