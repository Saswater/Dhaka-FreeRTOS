#ifndef LMAP_C
#define LMAP_C

#include "main.h"

#define LMAP_ADD(ADDR,BASE,SIZE,ENUM_FREE)                      \
    lmap_add(&lmap_start, (uintptr_t)ADDR, (uintptr_t)BASE,     \
            (uintptr_t)BASE + (uintptr_t)SIZE, ENUM_FREE);      

#define LMAP_DEL(PTR)                                           \
    lmap_del(&lmap_start, (uintptr_t)PTR)


// GLOBALS
uint64_t key_cnt = INVALID_KEY + 1;


// FUNCTIONS

// Can be modified, e.g. using a hash func or srand
uint64_t new_key() {
    key_cnt++;
    return key_cnt;
}

// Creates a new lock, initialises it with the key, and returns its address.
uint64_t* new_lock(uint64_t key) {
    // alloc a new lock lock_addr
    uint64_t* lock_addr = MALLOC_T(uint64_t);                                            /*MALLOC*/
    CHK_PTR(lock_addr);
    *lock_addr = key;
    return lock_addr;
}

void lmap_print(lmap_node* start) {
    lmap_node* curr_node = start;
    while (curr_node) {
        if (!curr_node->addr) {
            DBG("Current lmap node: empty.\n");
        } else {
            DBG("\nLMap:\nCurrent ptr addr: %p\nMetadata key: %llu\nbase: %p, bound: %p\nlock addr: %p\nfreeable: %u\n",
                (void*)curr_node->addr, curr_node->metadata->key, (void*)curr_node->metadata->base_addr,
                (void*)curr_node->metadata->bound_addr, (void*)curr_node->metadata->lock_addr, curr_node->metadata->freeable);
        }
        curr_node = curr_node->next;
    }
}

lmap_node* lmap_find(lmap_node* start, uintptr_t addr) {
    lmap_node* curr_node = start;

    while (curr_node) {
        if (curr_node->addr == addr)
            return curr_node;
        curr_node = curr_node->next;
    }

    return NULL;
}

lmap_node* lmap_add(
    lmap_node** lmap_start,
    uintptr_t addr,
    uintptr_t base,
    uintptr_t bound,
    PtrFreeable freeable
) {
    if (!addr) {
        DBG("Will not map a NULL ptr!");
        return NULL;
    }
    
    ptr_meta* mtdt = MALLOC_T(ptr_meta);                                                /*MALLOC*/
    CHK_PTR((void*)mtdt);
    lmap_node* new_node = MALLOC_T(lmap_node);                                          /*MALLOC*/
    CHK_PTR((void*)new_node);
    
    // the following is based on the assumption that if two addrs (key) are the same in the lmap,
    //      then their metadata->base_addrs will also be the same (vice versa not necessarily true)
    //      could add checks for this?
    lmap_node* base_node = lmap_find(*lmap_start, base);
    if (base_node) {
        DBG("Base %p already exists in lmap - copying metadata!\n", (void*)addr);
        *mtdt = *(base_node->metadata);
    } else {
        mtdt->base_addr  = base;
        mtdt->bound_addr = bound;
        // generate new key and lockaddr pair ONLY if neither the ptr nor the baseaddr already exist in lmap
        mtdt->key       = new_key();
        mtdt->lock_addr = new_lock(mtdt->key);
    }

    mtdt->freeable = freeable;
    new_node->addr = addr;
    new_node->metadata = mtdt;

    // append the new node to the beginning of the llmap by setting its next to the old start
    new_node->next = *lmap_start;
    // and change the global start!
    *lmap_start = new_node;
    return new_node;
}

// Deletes the first/all node(s) with matching addrs, along with all associated information like the lock.
//  Frees the ptr, if FREEABLE.
void lmap_del(
    lmap_node** lmap_start,
    uintptr_t addr
) {
    lmap_node* curr_node = *lmap_start;
    lmap_node* prev_node = (lmap_node*)NULL;
    uint_fast8_t found = 0;
    uint_fast8_t ptr_freed = 0;

    while (curr_node) {
        if (curr_node->addr == addr) {
            // match found!
            found = 1;
            if ((curr_node->metadata->freeable == FREEABLE) && (!ptr_freed)) {
                free((void*)addr);
                ptr_freed = 1;
            }
            // need to fill in gap!
            //      if start (no prev node), NEED TO CHANGE GLOBAL START
            //      if mid or end, then change prev_node->next to curr_node->next
            if (prev_node)  // NOT start node
                prev_node->next = curr_node->next;
            else
                *lmap_start = curr_node->next;
            
            // since we're getting rid of this entry, we should reset and free the associated lock, too
            //      NOTE: possible to do this w/o having the entire dedicated llist structure
            *(curr_node->metadata->lock_addr) = INVALID_KEY;
            free((void*)(curr_node->metadata->lock_addr));

            // free the metadata struct
            free((void*)(curr_node->metadata));
            // free the node
            free((void*)curr_node);

            // NOTE: uncomment if you only want the first entry to be deleted
            return;
        } else {
            // only move prev_node forward if curr_node hasn't been deleted
            prev_node = curr_node;
        }
        
        if (prev_node)
            curr_node = prev_node->next;
        else
            curr_node = *lmap_start;
    }

    if (!found)
        ERR("No match found for %p when trying to delete an lmap_node\n", (void*)addr);
}



#endif