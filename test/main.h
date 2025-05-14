// Libraries imported by FreeRTOS itself
#include <stddef.h>
#include <stdint.h>

// Debugging
#include <stdio.h>
#include <stdlib.h>

// malloc's a pointer of the specified type
#define MALLOC_T(TYPE)  ((TYPE*)malloc(sizeof(TYPE)))
#define INVALID_KEY     (uint64_t)0




extern void err(char* msg) {
    // could also add __LINE__ or __FILE__ if needed
    printf(msg);
    exit(-1);
}


extern void chk_ptr(void* ptr) {
    if (!ptr)
        err("malloc failed\n");
}
