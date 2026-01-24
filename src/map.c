#include "map.h"
#include "error_handling.h"

#define to_hash(res, key, buckets)                                            \
    _Generic(                                                                 \
        (key), i32: to_hash_i32, const char*: to_hash_str, default: kill_me)( \
        res, key, buckets)

// TODO: Support str hashes with more than 5 characters

static constexpr hash kNineHash           = '9' - '0' + 1;
static constexpr hash kZHash              = '9' - '0' + 1 + 'z' - 'a' + 1;
static constexpr char kHashableSpecSyms[] = {'-', '_'};

static Error to_hash_i32(
    hash*     res,
    const i32 key,
    const i32 buckets) {
    {
        bool pre_ok  = true;
        pre_ok      &= res != NULL;
        if (!pre_ok) {
            return (Error){.ec = PRE, .lvl = WARN};
        }
    }
    *res = key % buckets;
    return (Error){0};
}

static inline Error to_hash_symbol(
    hash* res,
    char  sym) {
    {
        bool pre_ok  = true;
        pre_ok      &= res != NULL;
        if (!pre_ok) {
            return (Error){.ec = PRE, .lvl = WARN};
        }
    }
    if (sym >= '0' && sym <= '9') {
        *res = sym - '0' + 1;
        return (Error){0};
    } else if (sym >= 'a' && sym <= 'z') {
        *res = kNineHash + sym - 'a' + 1;
        return (Error){0};
    }

    for (i32 i = 0; i < sizeof(kHashableSpecSyms); ++i) {
        if (sym == kHashableSpecSyms[i]) {
            *res = kZHash + i + 1;
            return (Error){0};
        }
    }
    return (Error){
        .lvl = WARN, .ec = UNEXP, .desc = "Unexpected symbol met when hashing"};
}

static Error from_hash_symbol(
    char* res,
    hash  hashed) {
    {
        bool pre_ok  = true;
        pre_ok      &= hashed > 0;
        pre_ok      &= res != NULL;
        if (!pre_ok) {
            return (Error){.ec = PRE, .lvl = WARN};
        }
    }
    if (hashed <= kNineHash) {
        *res = '9' - kNineHash + hashed;
        return (Error){0};
    } else if (hashed <= kZHash) {
        *res = 'z' - kZHash + hashed;
        return (Error){0};
    } else {
        const hash spec_sym_offset = hashed - kZHash - 1;
        if (spec_sym_offset >= sizeof(spec_sym_offset)) {
            return (Error){.lvl  = WARN,
                           .ec   = UNEXP,
                           .desc = "Unexpected hash for a symbol"};
        }
        *res = kHashableSpecSyms[spec_sym_offset];
    }
    return (Error){0};
}

Error from_hash_str(
    char* res_str,
    i32*  res_str_sz,
    hash  hashed) {
    {
        bool pre_ok  = true;
        pre_ok      &= hashed > 0;
        pre_ok      &= res_str != NULL;
        pre_ok      &= res_str_sz != NULL;
        if (!pre_ok) {
            return (Error){.ec = PRE, .lvl = WARN};
        }
    }
    i32 sz;
    sz = 0;
    for (hash pow = 1; hashed >= pow; pow *= kDftHashStrPolyBase) {
        sz++;
    }
    *res_str_sz = sz;
    res_str[sz] = '\0';
    for (i32 i = 0; i < sz; ++i) {
        hash  hash_sym;
        char  sym;
        hash_sym = hashed % kDftHashStrPolyBase;
        RETHROW_IF(from_hash_symbol(&sym, hash_sym));
        res_str[i] = sym;
        hashed /= kDftHashStrPolyBase;
    }
    return (Error){0};
}

Error to_hash_str(
    hash*       res,
    const char* key,
    const i32   ) {
    {
        bool pre_ok  = true;
        pre_ok      &= res != NULL;
        if (!pre_ok) {
            return (Error){.ec = PRE, .lvl = WARN};
        }
    }
    hash  poly_sum, deg;
    Error e;
    poly_sum = 0;
    deg      = 1;
    for (i32 i = 0; key[i] != '\0' && i < kDftHashStrPolyBase; ++i) {
        hash converted;
        e = to_hash_symbol(&converted, key[i]);
        RETHROW_IF(e);
        poly_sum += converted * deg;
        deg      *= kDftHashStrPolyBase;
    }
    *res = poly_sum;
    return (Error){0};
}
