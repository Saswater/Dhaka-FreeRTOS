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
    
    while (curr_node != (addr_node*)NULL) {
        if (curr_node->lock_addr == (uint64_t*)NULL)
            printf("Current node: empty.\n");
        else
            printf("Current node: %p\n", (void*)curr_node->lock_addr);
        curr_node = curr_node->next;
    }
    
    return;
}


// Add a new lockaddr node to the beginning of the llist.
addr_node* add_addr(addr_node** start, uint64_t key) {
    addr_node* new_node = MALLOC_T(addr_node);                                      /*MALLOC*/
    chk_ptr((void*)new_node);

    // alloc a new lock lock_addr
    // uint64_t* new_lock = (uint64_t*)malloc(sizeof(uint64_t));
    new_node->lock_addr = MALLOC_T(uint64_t);                                            /*MALLOC*/
    chk_ptr(new_node->lock_addr);
    *(new_node->lock_addr) = key;

    // append the new node to the beginning of the llist by setting its next to the old start
    new_node->next = *start;
    // NOTE: NEED TO ALSO CHANGE THE GLOBAL START!!!
    *start = new_node;
    return new_node;
}


// Deletes the first node with a matching lockaddr, sets lock to invalid, and frees the related data.
//  For now, doesn't return any status code on failure - simply exits. Could change.
void del_lockaddr(addr_node** start, uint64_t* lock_addr) {
    addr_node* curr_node = *start;
    addr_node* prev_node = (addr_node*)NULL;

    while (curr_node) {
        if (curr_node->lock_addr == lock_addr) {
            // match found!
            // need to fill in gap!
            //      if start (no prev node), NEED TO CHANGE GLOBAL START
            //      if mid or end, then change prev_node->next to curr_node->next
            if (prev_node)  // NOT start node
                prev_node->next = curr_node->next;
            else
                *start = curr_node->next;
            
            // SET THE LOCKADDR TO INVALID!!!!
            *(curr_node->lock_addr) = INVALID_KEY;
            // free the lockaddr
            free((void*)(curr_node->lock_addr));
            // free the struct
            free((void*)curr_node);

            return;
        }

        prev_node = curr_node;
        curr_node = curr_node->next;
    }

    // no match found
    err("No match found when trying to delete an addr_node\n");
}


int main() {return 0;}
