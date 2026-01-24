#ifndef MAP_H
#define MAP_H

#include "arena.h"
#include "auxiliary_utilities.h"

constexpr i32 kDftBucketsCount    = 32;
constexpr i32 kDftHashStrPolyBase = 51;
constexpr i32 kMaxHashableStrSz   = 16;

typedef i64 hash;

typedef struct Map_s {
    Arena mem;
    i32   sz;
    i32   buckets;
    i32   value_sz;
} Map;

#define construct_map(key, value) construct_map_f(sizeof(key), sizeof(value))

Error construct_map_f(
    Map** map,
    i32   key_sz,
    i32   value_sz);

Error from_hash_str(
    char* res_str,
    i32*  res_str_sz,
    hash  hashed);

Error to_hash_str(
    hash*       res,
    const char* key,
    const i32   buckets);

#endif /* MAP_H */
