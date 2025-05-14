// #include "main.h"
#include "llist.c"

// Types
// all the metadata
typedef struct ptr_meta_s {
    uint64_t key;
	uintptr_t base_addr;
	uintptr_t bound;
	uint64_t* lock_addr;
	uint8_t freeable;	// maybeee?  
} ptr_meta;

typedef struct lmap_node_s {
    ptr_meta* metadata;  // value
    uintptr_t addr;      // key
    struct lmap_node_s* next;
} lmap_node;


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
        if (!curr_node->addr)
            printf("Current node: empty.\n");
        else
            printf("Current node: %p\n", (void*)curr_node->addr);
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
    uint8_t freeable
) {
    // ptr_meta* mtdt = (ptr_meta*)malloc(sizeof(ptr_meta));
    ptr_meta* mtdt = MALLOC_T(ptr_meta);                                                /*MALLOC*/
    chk_ptr((void*)mtdt);
    lmap_node* new_node = MALLOC_T(lmap_node);                                          /*MALLOC*/
    chk_ptr((void*)new_node);

    mtdt->base_addr = base;
    mtdt->bound = bound;
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


// Deletes the first node with a matching addr, including its LOCKED ADDRESS's entry in the llist.
void lmap_del(
    lmap_node** lmap_start,
    addr_node** llist_start,
    uintptr_t addr
) {
    lmap_node* curr_node = *lmap_start;
    lmap_node* prev_node = (lmap_node*)NULL;

    while (curr_node) {
        if (curr_node->addr == addr) {
            // match found!
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

            return;
        }

        prev_node = curr_node;
        curr_node = curr_node->next;
    }

    // no match found
    err("No match found when trying to delete an addr_node\n");
}


