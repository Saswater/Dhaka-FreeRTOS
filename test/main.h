#ifndef MAIN_H
#define MAIN_H
// Libraries imported by FreeRTOS itself
#include <stddef.h>
#include <stdint.h>

// Debugging
#include <stdio.h>
#include <stdlib.h>

// malloc's a pointer of the specified type
#define MALLOC_T(TYPE)  ((TYPE*)malloc(sizeof(TYPE)))
#define DBG(...)        if (DEBUG) printf(__VA_ARGS__)
#define ERR(...)        {                                       \
                            printf(__VA_ARGS__);                \
                            printf("Line: %d, File: %s\n",      \
                                __LINE__, __FILE__);            \
                            exit(-1);                           \
                        }
#define CHK_PTR(PTR)    if (!PTR) ERR("malloc failed on line %d, file %s\n", __LINE__, __FILE__)

#define INVALID_KEY     (uint64_t)0
#define FREEABLE        (uint8_t)1
#define DEBUG           1

#endif