#ifndef MAIN_H
#define MAIN_H
// Libraries imported by FreeRTOS itself
#include <stddef.h>
#include <stdint.h>

// Debugging
#include <stdio.h>
#include <stdlib.h>

#define INVALID_KEY     (uint64_t)0
#define DEBUG           1

// malloc's a pointer of the specified type
#define MALLOC_T(TYPE)          ((TYPE*)malloc(sizeof(TYPE)))
#define DBG(...)                if (DEBUG) printf(__VA_ARGS__)
#define ERR(...)                {                                   \
    printf(__VA_ARGS__);                                            \
    printf("Line: %d, File: %s\n",                                  \
    __LINE__, __FILE__);                                            \
    exit(-1);                                                       \
}
// validates malloc's to avoid NULLs
#define CHK_PTR(PTR)                                                \
    if (!PTR)                                                       \
        ERR("malloc failed on line %d, file %s\n",                  \
            __LINE__, __FILE__)                                     \

typedef enum {
    FREEABLE,
    NON_FREEABLE
} PtrFreeable;

// all the metadata
typedef struct ptr_meta_s {
    uint64_t key;
	uintptr_t base_addr;
	uintptr_t bound_addr;
	uint64_t* lock_addr;
	PtrFreeable freeable;  
} ptr_meta;

typedef struct lmap_node_s {
    ptr_meta* metadata;  // value
    uintptr_t addr;      // key
    struct lmap_node_s* next;
} lmap_node;


extern lmap_node* lmap_start;


#endif