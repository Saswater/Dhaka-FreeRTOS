#ifndef DHAKA
#define DHAKA

#include "dhaka.h"
#include "linked_map.c"

#define CHK_ACCESS(PTR)        chk_ptr_access(lmap_start, (uintptr_t)PTR, sizeof(*PTR))

#define CHK_ARRAY(TYPE, ARR, INDEX)             {   \
    TYPE* elt = (TYPE*)ARR + INDEX;                 \
    LMAP_ADD(elt, ARR, sizeof(*elt), NON_FREEABLE); \
    CHK_ACCESS(elt);                                \
    LMAP_DEL(elt);                                  \
}

// NOTE: access_sz is the size of the actual access that's happening
void chk_ptr_access(
    lmap_node* lmap_start,
    uintptr_t addr,
    size_t access_sz
) {
    lmap_node* found_node = lmap_find(lmap_start, addr);
    if (!found_node) {
        // ptr doesn't exist - bad instrumentation, or intentional corruption
        ERR("Pointer %p wasn't found in lmap!\n", (void*)addr);
    }

    // Shyamoli
    if ((addr < found_node->metadata->base_addr) || (addr + (uintptr_t)access_sz > found_node->metadata->bound_addr)) {
        // can change this handler easily
        ERR("Spatial memory violation detected by Shyamoli: %p is out of the bounds %p to %p!\n",
            (void*)addr, (void*)(found_node->metadata->base_addr), (void*)(found_node->metadata->bound_addr));
    }

    // Chhayanaut
    if (found_node->metadata->key != *(found_node->metadata->lock_addr)) {
        ERR("Temporal memory violation detected by Chhayanaut: %p's key (%llu) and lock (%llu) don't match!\n",
            (void*)addr, found_node->metadata->key, *(found_node->metadata->lock_addr));
    }
}


// Dhaka Malloc!
void* dhalloc(size_t size) {
    // if error, use void**
    void* ptr = malloc(size);
    LMAP_ADD(ptr, ptr, size, FREEABLE);
    return ptr;
}


#endif