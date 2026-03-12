#ifndef _GEODE_H_
#define _GEODE_H_
#include "c.h"
#include "geohash.h"
#include <roaring.h>
#include "uthash.h"

#define MAX_PREFIX_SZ 10

struct geode {
    char prefix[MAX_PREFIX_SZ + 1];
    int prefix_sz;
    struct spatial_range base_range;
    unsigned int width;
    unsigned int height;
    double x_deg;
    double y_deg;
    double x_px;
    double y_px;
    roaring_bitmap_t *bmp;
    hll_t *hll;
    int opt_error;
    uint64_t total;
    struct geode *sgs[32];
    int num_sgs;
    UT_hash_handle hh;
};

typedef struct {
	int x, y;
}
geodePoint, *geodePointPtr;

struct query_result {
    uint32_t *locations;
    uint64_t sz;
};

void geode_add_geohash(struct geode *g, const char *geohash);
void geode_add_sprange(struct geode *g, const struct spatial_range *sr);
void geode_add_xy(struct geode *g, const int x, const int y);

double geode_load_factor(struct geode *g);
geodePoint geode_sprange_to_point(struct geode *g, const struct spatial_range *sr);
void print_geocoord(GeoCoord *gc);
void print_geode(struct geode *gc);

/**
 * Initializes a new GEODE data structure using the provided base Geohash.
 *
 * This function allocates memory; the resulting GEODE struct should be freed
 * using geode_free().
 *
 * @param base_geohash Geohash to use as the bounding box for this GEODE, e.g.,
 * a base Geohash of 'dj' creates a new GEODE that is centered on the southern
 * USA near Alabama, Georgia, North Florida.
 *
 * @return The initialized GEODE.
 */
struct geode *geode_create(const char *base_geohash, unsigned int precision, unsigned int hash_sz);
void geode_free(struct geode *g);

int geode_xy_to_idx(struct geode *g, int x, int y);
bool rectangle_intersects_geode(struct geode *g, GeoCoord *query, int geode_num);
bool polygon_intersects_geode(struct geode *g, const struct spatial_range *c, int n);
struct query_result* polygon_query_geode(struct geode *g, const struct spatial_range *c, int n);

#endif
