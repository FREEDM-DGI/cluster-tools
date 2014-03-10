#ifndef _DATA_TYPES_H
#define _DATA_TYPES_H

#include <stdint.h>

/*
 * documents that it is realized that the variable does not
 * need to be initiaized, but the compiler will warn if we do
 * not set it equal to something
 */
#define uninitialized_var(x)x=0

typedef uint64_t u64;
typedef int64_t s64;

typedef uint32_t u32;
typedef int32_t s32;

// this relies on the fact that right shifting will replace the left
// most bits to zero (gcc specific?)
#define S64_MAX ((s64)(~0ULL>>1))

#endif // _DATA_TYPES_H 
