#ifndef LMAP_C
#define LMAP_C

#include "main.h"

// Types
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

// uint64_t* lock_persist;
// GLOBALS
uint64_t key_cnt = INVALID_KEY + 1;
// always points to the start of the list
// lmap_node* lmap_start = (lmap_node*)NULL;
// lmap_node lmap_start = {{0}, (uintptr_t)NULL, (lmap_node*)NULL};

// Prints all the nodes.
void lmap_print(lmap_node*);
// Insert a node at the beginning of the lmap.
// lmap_node* lmap_add(uintptr_t, uintptr_t, uintptr_t, uint8_t);


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
            // lock_persist = curr_node->metadata->lock_addr;
            // DBG("Lockaddr key:%llu\n\n\n", *lock_persist);
        }
        curr_node = curr_node->next;
    }
    
    return;
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
    // avoid duplicates, since the same addr should have the same key
    //      actually, should be okay! we can just copy over the details
    // if (lmap_find(*lmap_start, addr)) {
    //     DBG("Address %p already exists in lmap - not adding again!\n", (void*)addr);
    //     return NULL;
    // }
    
    ptr_meta* mtdt = MALLOC_T(ptr_meta);                                                /*MALLOC*/
    CHK_PTR((void*)mtdt);
    lmap_node* new_node = MALLOC_T(lmap_node);                                          /*MALLOC*/
    CHK_PTR((void*)new_node);
    
    // the following is based on the assumption that if two addrs (key) are the same in the lmap,
    //      then their metadata->base_addrs will also be the same (vice versa not necessarily true)
    //      could add checks for this?
    // lmap_node* base_node = lmap_find(*lmap_start, addr);
    // if base =/= addr (due to ptr arithmetic), then base could also already exist
    //      if so, propagate metadata!
    lmap_node* base_node = lmap_find(*lmap_start, base);
    // if (!base_node)
    //     base_node = lmap_find(*lmap_start, base);
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
            // del_lockaddr(llist_start, curr_node->metadata->lock_addr);
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


#endif