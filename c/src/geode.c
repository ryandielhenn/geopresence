#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "c.h"
#include "log.h"
#include "geode.h"
#include "roaring.c"
#include "uthash.h"
#include "geohash.h"
#include "bitmap_graphics.h"

static void query_transform(roaring_bitmap_t *r, struct geode *g, const struct spatial_range *coords, int n);

/**
 * Create a geode with a base hash and precision of bitmap
 *
 * @param base_geohash - starting hash
 * @param precision  - precision of bitmap for the geode
 */
struct geode *geode_create(const char *base_geohash, unsigned int precision, unsigned int hash_sz)
{
    struct geode *g = malloc(sizeof(struct geode));
    if (g == NULL) {
        perror("malloc");
        return NULL;
    }


    memcpy(g->prefix, base_geohash, hash_sz);
    g->prefix[hash_sz] = '\0';
    g->prefix_sz = hash_sz;
    g->num_sgs = 0;

    geohash_decodeN(&g->base_range, g->prefix);
    g->hll = HLL_create(9, &g->opt_error);

    /*
     * height, width calculation:
     * width = 2^(floor(precision / 2))
     * height = 2^(ceil(precision / 2))
     */
    int w = precision / 2;
    int h = precision / 2;
    if (precision % 2 != 0) {
        h += 1;
    }

    g->width = (1 << w); /* = 2^w */
    g->height = (1 << h); /* = 2^h */

    g->bmp = roaring_bitmap_create_with_capacity(g->width * g->height);

    /* Determine the number of degrees in the x and y directions for the base
     * spatial range this GEODE represents */
    g->x_deg = fabs(g->base_range.west - g->base_range.east);
    g->y_deg = fabs(g->base_range.north - g->base_range.south);

    /* Determine the number of degrees represented by each grid pixel */
    g->x_px = g->x_deg / (double) g->width;
    g->y_px = g->y_deg / (double) g->width;

    g->total = 0;

    LOG("New GEODE: %s (%d x %d), dpp: (%f, %f)\n",
            g->prefix,
            g->width,
            g->height,
            g->x_px,
            g->y_px);

    return g;
}

double geode_load_factor(struct geode *g) {
    return (1 - ((double) HLL_estimate(g->hll) / (double) g->total));
}

uint64_t hash(int i) {
    uint64_t x = (uint64_t)(unsigned int)i;
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}


/**
 * Add a data point to a geode. Search for and create sub-geodes when geode is appropriately dense.
 *
 * @param g       - geode to add data to
 * @param geohash - geohash to be added to the geode
 */
void geode_add_geohash(struct geode *g, const char *geohash) {
    struct spatial_range sr;
    struct geode *gx = g;

    double lf = geode_load_factor(g);
   
    /* Check if geode is dense */
    if (lf > .6) {
        LOG("Load factor breached -> %s -> %f -> %s\n", g->prefix, lf, geohash);
        char sub_hash[g->prefix_sz + 2];
        memcpy(sub_hash, geohash, g->prefix_sz + 1);
        sub_hash[g->prefix_sz + 2] = '\0';
        /* Find the sub geode to place new data */
        for (int i = 0; i < g->num_sgs; i++) {
            if (strncmp(g->sgs[i]->prefix, sub_hash, g->prefix_sz + 1) == 0) {
                LOG("Found sub geode -> %s\n", g->sgs[i]->prefix);
                /* Add data point to sub geode and return */
                geode_add_geohash(g->sgs[i], geohash);
                return;
            }
        }
        /* If sub geode wasn't found and it will be at most 10 characters in length, create it */
        if (g->prefix_sz < 10) {
            g->sgs[g->num_sgs] = geode_create(geohash, 16, g->prefix_sz + 1);
            LOG("Created sub geode -> %s -> sz %d\n", g->sgs[g->num_sgs]->prefix, g->sgs[g->num_sgs]->prefix_sz);
            gx = g->sgs[g->num_sgs];
            g->num_sgs++;
        }
    }
    /* Add data point to the geode */
    geohash_decodeN(&sr, geohash);
    geodePoint p = geode_sprange_to_point(gx, &sr);
    geode_add_xy(gx, p.x, p.y);
}

/**
 * Add an (x,y) point to the geode 
 *
 * @param g - geode to add to
 * @param x - x coord
 * @param y - y coord
 */
void geode_add_xy(struct geode *g, const int x, const int y) {
    unsigned int idx = geode_xy_to_idx(g, x, y);
    HLL_update(g->hll, hash(idx));
    g->total++;
    roaring_bitmap_add(g->bmp, idx);
}


/**
 * Converts X, Y coordinates to a particular index within the underlying
 * bitmap implementation.  Essentially this converts a 2D index to a 1D
 * index.
 *
 * @param x The x coordinate to convert
 * @param y The y coorddinate to convert
 *
 * @return A single integer representing the bitmap location of the X, Y
 * coordinates.
 */
int geode_xy_to_idx(struct geode *g, const int x, const int y)
{
    return y * g->width + x;
}

/**
 * Prints a geocoord
 */
void print_geocoord(GeoCoord *gc)
{
    printf("(%f, %f) in [%f, %f, %f, %f]\n", gc->longitude, gc->latitude, gc->north, gc->east, gc->south, gc->west);
}

/** 
 * Prints geode as above
 */
void print_geode(struct geode *gc)
{
    printf("%s (%f, %f) in [%f, %f, %f, %f]\n", gc->prefix, gc->base_range.longitude, gc->base_range.latitude, gc->base_range.north, gc->base_range.east, gc->base_range.south, gc->base_range.west);
}

/**
 * Converts a coordinate pair (defined with latitude, longitude in decimal
 * degrees) to an x, y location in the grid.
 *
 * @param coords the Coordinates to convert.
 *
 * @return Corresponding x, y location in the grid.
 */
geodePoint geode_sprange_to_point(
        struct geode *g, const struct spatial_range *sr)
{

    /* Assuming (x, y) coordinates for the geoavailability grids, latitude
     * will decrease as y increases, and longitude will increase as x
     * increases. This is reflected in how we compute the differences
     * between the base points and the coordinates in question. */
    float xDiff = sr->longitude - g->base_range.west;
    float yDiff = g->base_range.north - sr->latitude;

    geodePoint point;
    point.x = (int) (xDiff / g->x_px);
    point.y = (int) (yDiff / g->y_px);
    return point;
}

/**
 * Function: polygon_intersects_geode
 * Purpose:  check ifany index locations that are set in g are contained in the bounds of coords
 * Params:   g      - the geode to query
 *           coords - the polygon represented as lat/lon pairs
 *           n      - number of lat/lon pairs
 */
bool polygon_intersects_geode(struct geode *g, const struct spatial_range *coords, int n) {
    roaring_bitmap_t *r = roaring_bitmap_create_with_capacity(g->width * g->height);
    query_transform(r, g, coords, n);
    return roaring_bitmap_intersect(g->bmp, r);
}

/**
 * Function: polygon_query_geode
 * Purpose:  get the index locations that are set in g and contained in the bounds of coords
 * Params:   g      - the geode to query
 *           coords - the polygon represented as lat/lon pairs
 *           n      - number of lat/lon pairs
 *
 * Return:   locations_and_sz - array of matching locations and the size of the array.
 *                              the size is determined with roaring_bitmap_get_cardinality
 */
struct query_result* polygon_query_geode(struct geode *g, const struct spatial_range *coords, int n) {
    roaring_bitmap_t *r = roaring_bitmap_create_with_capacity(g->width * g->height);
    query_transform(r, g, coords, n);
   
    /* Convert the intersecting grid cells of the geode and the query to an array of matching index locations */
    roaring_bitmap_and_inplace(r, g->bmp);
    uint64_t sz = roaring_bitmap_get_cardinality(r);
    uint32_t *locations = calloc(sz, sizeof(uint32_t));
    roaring_bitmap_to_uint32_array(r, locations);

    /* Expose the number of index locations in the array */
    struct query_result *locations_and_sz = malloc(sizeof(struct query_result));
    locations_and_sz->locations = locations;
    locations_and_sz->sz = sz;

    return locations_and_sz;
}

/**
 * Function: query_transform
 *
 * Purpose:  Resolve the set of latitude/longitude coordinates in the query 
 *           with the coordinates of the geode. 
 *
 *           Convert each point to an xy location in the bitmap, 
 *           then draw the filled polygon with the set of xy pairs.
 *
 * @param    r - The bitmap to draw the polygon query into
 *           g - The geode that the bitmap should be drawn for. If any part of the polygon is
 *               outside this geode's spatial range, that part of the polygon will be clipped
 *           c - Set of latitude and longitude coordinates
 *           n - number of coordinates
 */
void query_transform(roaring_bitmap_t *r, struct geode *g, const struct spatial_range *c, int n) {
    geodePointPtr points = (geodePointPtr)calloc(n, sizeof(geodePoint));
    for (int i = 0; i < n; i++) {
        points[i] = geode_sprange_to_point(g, &c[i]);
    }
    bmp_filled_polygon(r, points, n, g->width, g->height);
    free(points);
}
