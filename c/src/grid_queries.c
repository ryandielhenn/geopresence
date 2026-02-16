#include <stdio.h>
#include <stdlib.h>
#include "geode.h"
#include "geohash.h"
#include "grid_queries.h"

static void mbr_of_poly(const struct spatial_range *poly, struct spatial_range *mbr, int n);

/**
 * Returns a list of geohashes in the grid that are contained in the query.
 *
 * @param grid - hashmap of geohash to geode
 * @param coords - the polygon query as a list of latitude longitude points
 *
 * @return list of geohashes contained in the query
 */
char** geodes(struct geode *grid, const struct spatial_range *poly, int n) {
  struct spatial_range mbr;
  mbr_of_poly(poly, &mbr, n);

  /* Geodes in the system that bound this search */
  char *north_w = geohash_encode(mbr.north, mbr.west, PREFIX_SZ);
  char *north_e = geohash_encode(mbr.north, mbr.east, PREFIX_SZ);
  char *south_e = geohash_encode(mbr.south, mbr.east, PREFIX_SZ);
    printf("PREFIX_SZ = %d\n", PREFIX_SZ);
  
  /* All geodes within the query */
  char **hashes = calloc(1, sizeof(char *));

  /* Count of all geodes within the query bounds */
  int count = 0;

  /* Use the North Western geode of the query as a starting point */
  struct geode *g;

  /* Save start of current row*/
  char *start = north_w;
  char *curr = start;

  /* Keep track of spot in row and row size */
  int width = -1;
  int g_width = width;
  
  /* While there are more geodes */
  while (true) {
  
    /* If the geode exists in the system add it to the match list */
    HASH_FIND_STR(grid, curr, g);
    if (g != NULL) {
      hashes = count != 0 ? realloc(hashes, sizeof(char *) * (count + 1)) : hashes;
      hashes[count++] = curr;
    }
    
    /* If at the last geode then end */
    if (strcmp(curr, south_e) == 0) {
      break;
    }

    /* When at the north eastern geode of the query, save width (geode units) of the rows of the minimum bounding rectangle */
    if ((strcmp(curr, north_e) == 0)) {
      width = count;
      g_width = width;
      /* Carriage return and new line */
      curr = get_neighbor(start, SOUTH);
      /* Change start to new row and resume working east */
      start = curr;
    }

    /* If done with row reset row count and toggle south*/
    else if (width == 0) {
      width = g_width;
      /* Carriage return and new line */
      curr = get_neighbor(start, SOUTH);
      /* Change start to new row and resume working east */
      start = curr;
    } else {
      curr = get_neighbor(curr, EAST);
    }

    width--;
  }
  
  /* NULL termniate the list of hashes */
  hashes = realloc(hashes, sizeof(char *) * (count + 1));
  hashes[count] = NULL;

  return hashes;
}

/**
 * Function: mbr_of_poly
 * Purpose:  Find the minimum bounding rectangle of a polygon
 *
 * @param  poly - list of latitude longitude points
 * @param  mbr  - spatial range to add mbr data toggle
 * @param  n    - number of points in poly
 */
static void mbr_of_poly(const struct spatial_range *poly, struct spatial_range *mbr, int n) {
    mbr->east = MIN_LONG;
    mbr->west = MAX_LONG;
    mbr->south = MAX_LAT;
    mbr->north = MIN_LAT;

    for (int i = 0; i < n; i++) {
        if (poly[i].longitude < mbr->west) {
            mbr->west = poly[i].longitude;
        }
        if (poly[i].longitude > mbr->east) {
            mbr->east = poly[i].longitude;
        }
        if (poly[i].latitude > mbr->north) {
            mbr->north = poly[i].latitude;
        }
        if (poly[i].latitude < mbr->south) {
            mbr->south = poly[i].latitude;
        }
    }
} 
