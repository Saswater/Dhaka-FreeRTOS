#ifndef LMAP_C
#define LMAP_C

#include "main.h"
#include "llist.c"

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
uint64_t key_cnt = 0;
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


lmap_node* lmap_add(
    lmap_node** lmap_start,
    addr_node** llist_start,
    uintptr_t addr,
    uintptr_t base,
    uintptr_t bound,
    PtrFreeable freeable
) {
    if (!addr) {
        DBG("Will not map a NULL ptr - returning!");
        return NULL;
    }

    // ptr_meta* mtdt = (ptr_meta*)malloc(sizeof(ptr_meta));
    ptr_meta* mtdt = MALLOC_T(ptr_meta);                                                /*MALLOC*/
    CHK_PTR((void*)mtdt);
    lmap_node* new_node = MALLOC_T(lmap_node);                                          /*MALLOC*/
    CHK_PTR((void*)new_node);

    mtdt->base_addr = base;
    mtdt->bound_addr = bound;
    mtdt->freeable = freeable;
    mtdt->key = new_key();
    // this changes the global llist_start
    mtdt->lock_addr = add_addr(llist_start, mtdt->key)->lock_addr;
    new_node->addr = addr;
    new_node->metadata = mtdt;

    // append the new node to the beginning of the llmap by setting its next to the old start
    new_node->next = *lmap_start;
    // and change the global start!
    *lmap_start = new_node;
    return new_node;
}


// Deletes all nodes with matching addrs, including their LOCKED ADDRESSes' entries in the llist.
void lmap_del(
    lmap_node** lmap_start,
    addr_node** llist_start,
    uintptr_t addr
) {
    lmap_node* curr_node = *lmap_start;
    lmap_node* prev_node = (lmap_node*)NULL;
    uint_fast8_t found = 0;

    while (curr_node) {
        if (curr_node->addr == addr) {
            // match found!
            found = 1;
            // need to fill in gap!
            //      if start (no prev node), NEED TO CHANGE GLOBAL START
            //      if mid or end, then change prev_node->next to curr_node->next
            if (prev_node)  // NOT start node
                prev_node->next = curr_node->next;
            else
                *lmap_start = curr_node->next;
            
            // since we're getting rid of this entry, we should reset and free the associated lock, too
            del_lockaddr(llist_start, curr_node->metadata->lock_addr);

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