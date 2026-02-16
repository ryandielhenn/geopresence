#include "geode.h"
#include "timer.h"
#include <stdio.h>
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
    double start = timer_now();
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

    double end = timer_now();

    printf("%f\n", end - start);
    fclose(fp);
    return 0;
}
