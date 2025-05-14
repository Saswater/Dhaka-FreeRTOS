#ifndef LLIST_C
#define LLIST_C

#include "main.h"


// linked list to store lock addresses
typedef struct addr_node_s {
    uint64_t* lock_addr;
    struct addr_node_s* next;
} addr_node;


// GLOBALS
// addr_node* llist_start = (addr_node*)NULL;


void llist_print(addr_node* start) {
    addr_node* curr_node = start;
    
    while (curr_node) {
        if (!curr_node->lock_addr) {
            DBG("Current llist node: empty.\n");
        } else {
            DBG("\nLList:\nCurrent lock addr: %p\nLock value: %llu\n\n",
                (void*)curr_node->lock_addr, *(curr_node->lock_addr));
        }
        curr_node = curr_node->next;
    }
    
    return;
}


// Add a new lockaddr node to the beginning of the llist.
addr_node* add_addr(addr_node** start, uint64_t key) {
    addr_node* new_node = MALLOC_T(addr_node);                                      /*MALLOC*/
    CHK_PTR((void*)new_node);

    // alloc a new lock lock_addr
    // uint64_t* new_lock = (uint64_t*)malloc(sizeof(uint64_t));
    new_node->lock_addr = MALLOC_T(uint64_t);                                            /*MALLOC*/
    CHK_PTR(new_node->lock_addr);
    *(new_node->lock_addr) = key;

    // append the new node to the beginning of the llist by setting its next to the old start
    new_node->next = *start;
    // NOTE: NEED TO ALSO CHANGE THE GLOBAL START!!!
    *start = new_node;
    return new_node;
}


// Deletes every node with matching lockaddrs, sets lock to invalid, and frees the related data.
//  For now, doesn't return any status code on failure - simply exits. Could change.
void del_lockaddr(addr_node** start, uint64_t* lock_addr) {
    addr_node* curr_node = *start;
    addr_node* prev_node = (addr_node*)NULL;
    uint_fast8_t found = 0;

    while (curr_node) {
        if (curr_node->lock_addr == lock_addr) {
            // match found!
            found = 1;
            // need to fill in gap!
            //      if start (no prev node), NEED TO CHANGE GLOBAL START
            //      if mid or end, then change prev_node->next to curr_node->next
            if (prev_node) {// NOT start node
                prev_node->next = curr_node->next;
            } else {
                *start = curr_node->next;
            }

            // SET THE LOCKADDR TO INVALID!!!!
            *(curr_node->lock_addr) = INVALID_KEY;
            // free the lockaddr
            free((void*)(curr_node->lock_addr));
            // free the struct
            free((void*)curr_node);

            // NOTE: add this back if you only want the first one deleted
            // return;
        } else {
            // if we didn't delete current node, it will be the next previous node
            prev_node = curr_node;
        }

        // at this point, prev_node will point to the node immediately before the upcoming node either way
        // ...unless the very first element of the list was popped, since prev_node is still NULL
        if (prev_node)
            curr_node = prev_node->next;
        else
            curr_node = *start;
    }

    // no match found
    // ERR("No match for lockaddr %p when trying to delete an addr_node\n", (void*)lock_addr);
    
    // no match may be found legally; e.g. if multiple deletes happen in lmap.c
    //      so maybe just notify no match was found?
    if (!found)
        DBG("No match for lockaddr %p when trying to delete an addr_node\n", (void*)lock_addr);
}



#endif