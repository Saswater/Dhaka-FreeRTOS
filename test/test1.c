#include "main.h"
#include "llist.c"
#include "lmap.c"


// GLOBALS
lmap_node* lmap_start = (lmap_node*)NULL;
addr_node* llist_start = (addr_node*)NULL;

typedef struct {
    int* y;
    int x;
} test_s;


// int main(void) {
//     test_s* test_struct = MALLOC_T(test_s);
    
//     test_struct->y = MALLOC_T(int);
//     *(test_struct->y) = 2498;
//     test_struct->x = 13;

//     DBG("x: %d, y: %d\n", test_struct->x, *(test_struct->y));

//     free(test_struct->y);
//     free(test_struct);

//     DBG("Done!\n");
//     return 0;
// }

void print_vars() {
    lmap_print(lmap_start);
    llist_print(llist_start);
    DBG("Key counter: %llu\nLMap start: %p, LList start: %p\n\n", key_cnt, (void*)lmap_start, (void*)llist_start);
}


int add_to_lists(void) {
    print_vars();
    // for each pointer allocation, lmap_add with the required info
    // happens after malloc for both SoftBound and CETS
    test_s* foo = MALLOC_T(test_s);
    // just 1 line of instrumentation!!
    lmap_add(&lmap_start, &llist_start, (uintptr_t)foo, (uintptr_t)foo, (uintptr_t)foo + sizeof(test_s), FREEABLE);
    print_vars();

    foo->y = MALLOC_T(int);
    lmap_add(&lmap_start, &llist_start, (uintptr_t)(foo->y), (uintptr_t)(foo->y), (uintptr_t)(foo->y) + sizeof(int), FREEABLE);
    print_vars();

    // pointer copying/arithmetic propagates the metadata
    //      while unmodified pointer copying doesn't necessitate duplication of lmap entries,
    //      in my case, it makes it a lot more consistent and easier to script into C if I treat them the same
    int* copy = foo->y;
    lmap_add(&lmap_start, &llist_start, (uintptr_t)copy, (uintptr_t)(foo->y), (uintptr_t)(foo->y) + sizeof(int), NON_FREEABLE);
    // we only have to free ptrs that were directly returned by malloc!

    // pointer arithmetic!
    int* new = foo->y + 1;
    // the addr (key for lmap) is ALWAYS the new pointer's value, but the base is the pointer that this is based off
    //      NOTE: POSSIBLE ISSUE! can't use a pointer within a struct to access other elts of the struct. maybe okay?
    lmap_add(&lmap_start, &llist_start, (uintptr_t)new, (uintptr_t)(foo->y), (uintptr_t)(foo->y) + sizeof(int), NON_FREEABLE);

    // now, frees and deletes go hand in hand for each malloc
    // deleting two entries of the same ptr address... but is okay, because duplicates are allowed in lmap
    // NOTE: the program needs to ensure that everything has been freed
    lmap_del(&lmap_start, &llist_start, (uintptr_t)new);
    lmap_del(&lmap_start, &llist_start, (uintptr_t)copy);       // FIX WITH FIND~!
    lmap_del(&lmap_start, &llist_start, (uintptr_t)(foo->y));
    free(foo->y);
    print_vars();
    // NOTE: to free a struct, NEED TO FREE STRUCT MEMBERS BEFORE STRUCT ITSELF!!!

    lmap_del(&lmap_start, &llist_start, (uintptr_t)foo);
    free(foo);
    print_vars();

    // DBG("Lockaddr key:%llu\n\n\n", *lock_persist);

    return 0;
}


int validation(void) {
    // now, adding pointer validation for ONE (1) pointer during free
    //      then, add validation for access
    //      then, propagating metadata for arithmetic?

    return 0;
}


int errors(void) {
    // here... need to start testing against errors
    //      spatial safety (data only attack example!)
    //      temporal (dangling ptrs)
    //      double free
    //      copying ptrs and trying to do stuff

    return 0;
}


int main(void) {return add_to_lists();}
