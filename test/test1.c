#include "main.h"
#include "lmap.c"


// GLOBALS
lmap_node* lmap_start = (lmap_node*)NULL;

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
    DBG("Key counter: %llu\nLMap start: %p\n\n", key_cnt, (void*)lmap_start);
}


int add_to_lists(void) {
    print_vars();
    // 1. ALLOCATION
    // for each pointer allocation, lmap_add with the required info
    // happens after malloc for both SoftBound and CETS
    test_s* foo = MALLOC_T(test_s);
    // just 1 line of instrumentation!!
    lmap_add(&lmap_start, (uintptr_t)foo, (uintptr_t)foo, (uintptr_t)foo + sizeof(test_s), FREEABLE);
    // print_vars();

    // accessing foo->y is okay for now, since it doesn't threaten to go outside struct bounds?
    //      once we see any pointer assignment, that's when we add another entry to lmap
    foo->y = MALLOC_T(int);
    lmap_add(&lmap_start, (uintptr_t)(foo->y), (uintptr_t)(foo->y), (uintptr_t)(foo->y) + sizeof(int), FREEABLE);
    // print_vars();

    // arrays my be{love,hate}d
    // we won't consider dynamically allocated strings here, since they're a different beast in terms of memory access
    const char pld[] = "12345";
    lmap_add(&lmap_start, (uintptr_t)(pld), (uintptr_t)(pld), (uintptr_t)(pld) + sizeof(pld), FREEABLE);

    // 2. POINTER ARITHMETIC
    // pointer copying/arithmetic propagates the metadata
    //      while unmodified pointer copying doesn't necessitate duplication of lmap entries,
    //      in my case, it makes it a lot more consistent and easier to script into C if I treat them the same
    int* copy = foo->y;
    lmap_add(&lmap_start, (uintptr_t)copy, (uintptr_t)(foo->y), (uintptr_t)(foo->y) + sizeof(int), NON_FREEABLE);
    // we only have to free ptrs that were directly returned by malloc!

    // pointer arithmetic!
    int* new = foo->y + 1;
    // the addr (key for lmap) is ALWAYS the new pointer's value, but the base is the pointer that this is based off
    //      NOTE: POSSIBLE ISSUE! can't use a pointer within a struct to access other elts of the struct. maybe okay?
    lmap_add(&lmap_start, (uintptr_t)new, (uintptr_t)(foo->y), (uintptr_t)(foo->y) + sizeof(int), NON_FREEABLE);
    // Q. if pointer arithmetic adds a legal entry to lmap, what stops an attacker from creating an out-of-bounds entry?
    // A. the lmap entry inherits all base/bound info from the original ptr; checking happens on access!

    // 3. POINTER ACCESS
    // look up the ptr, perform the SoftBound AND CETS check
    //      NOTE: addr should be the pointer getting dereferenced! but sizeof should check the underlying type
    // enable this to see a temporal error (dangling ptr)
    // lmap_del(&lmap_start, (uintptr_t)(foo->y));
    chk_ptr_access(lmap_start, (uintptr_t)(copy), sizeof(*(copy)));
    int y = *(copy);
    // enable for a spatial "error" (going out of bounds)
    // chk_ptr_access(lmap_start, (uintptr_t)(new), sizeof(*(new)));
    // int z = *(new);
    // (void) z;
    chk_ptr_access(lmap_start, (uintptr_t)(foo), sizeof(*foo));
    test_s bar = *foo;
    // unrelated: arrays/buffers have different behaviour under sizeof (total data length), even though they're still ptrs
    // the below doesn't work, because &pld[2] would need to be its own entry in lmap first through instrumentation
    // chk_ptr_access(lmap_start, (uintptr_t)(&pld[2]), sizeof(pld[2]));
    char* pld_elt = (char*)pld + 2;
    lmap_add(&lmap_start, (uintptr_t)pld_elt, (uintptr_t)(pld), (uintptr_t)(pld) + sizeof(char), NON_FREEABLE);
    chk_ptr_access(lmap_start, (uintptr_t)(pld_elt), sizeof(*pld_elt));
    char c = pld[2];
    (void) bar;
    (void) y;
    (void) c;

    // now, frees and deletes go hand in hand for each malloc
    // deleting two entries of the same ptr address... but is okay, because duplicates are allowed in lmap
    // NOTE: the program needs to ensure that everything has been freed
    lmap_del(&lmap_start, (uintptr_t)new);
    // ERR("a\n\n\n");
    lmap_del(&lmap_start, (uintptr_t)copy);
    lmap_del(&lmap_start, (uintptr_t)(foo->y));
    // free(foo->y);
    print_vars();
    // NOTE: to free a struct, NEED TO FREE STRUCT MEMBERS BEFORE STRUCT ITSELF!!!

    lmap_del(&lmap_start, (uintptr_t)foo);
    // free(foo);
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
