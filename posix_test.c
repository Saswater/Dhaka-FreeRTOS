#include "main.h"
#include "linked_map.c"
#include "dhaka.c"

// GLOBALS
lmap_node* lmap_start = (lmap_node*)NULL;

typedef struct {
    int* y;
    int x;
} test_s;


void print_vars() {
    lmap_print(lmap_start);
    DBG("Key counter: %llu\nLMap start: %p\n\n", key_cnt, (void*)lmap_start);
}


int examples(void) {
    // Pointer definitions
    test_s* foo = MALLOC_T(test_s);
    foo->y      = MALLOC_T(int);
    const char pld[] = "12345";
    int* copy = foo->y;
    
    // Pointer accesses
    int y = *copy;
    test_s bar = *foo;
    char c = pld[2];

    // below should be a spatial memory violation
    //int z = *(foo->y + 1);
    
    // and this a temporal memory violation
    //free(foo->y);
    //int y = *copy;    
    
    // Pointer frees
    free(foo->y);
    free(foo);

    (void) c;
    (void) bar;
    (void) y;

    DBG("Got through!\n");

    return 0;
}


int examples_hardened(void) {
    print_vars();
    // 1. ALLOCATION
    test_s* foo = dhalloc(sizeof(test_s));
    // print_vars();

    // accessing foo->y is okay for now, since it doesn't threaten to go outside struct bounds?
    //      once we see any pointer assignment, that's when we add another entry to lmap
    foo->y = dhalloc(sizeof(int));
    // print_vars();

    // arrays my be{love,hate}d
    // we won't consider dynamically allocated strings here, since they're a different beast in terms of memory access
    const char pld[] = "12345";
    LMAP_ADD(pld, pld, sizeof(pld), FREEABLE);

    // 2. POINTER ARITHMETIC
    // pointer copying/arithmetic propagates the metadata
    //      while unmodified pointer copying doesn't necessitate duplication of lmap entries,
    //      in my case, it makes it a lot more consistent if I treat them the same
    int* copy = foo->y;
    LMAP_ADD(copy, foo->y, sizeof(int), NON_FREEABLE);
    // we only have to free ptrs that were directly returned by malloc!

    // pointer arithmetic!
    int* new = foo->y + 1;
    // the addr (key for lmap) is ALWAYS the new pointer's value, but the base is the pointer that this is based off
    //      NOTE: POSSIBLE ISSUE! can't use a pointer within a struct to access other elts of the struct. maybe okay?
    LMAP_ADD(new, foo->y, sizeof(int), NON_FREEABLE);
    // Q. if pointer arithmetic adds a legal entry to lmap, what stops an attacker from creating an out-of-bounds entry?
    // A. the lmap entry inherits all base/bound info from the original ptr; checking happens on access!

    // 3. POINTER ACCESS
    // look up the ptr, perform the SoftBound AND CETS check
    //      NOTE: addr should be the pointer getting dereferenced! but sizeof should check the underlying type
    ///////////////
    ////enable this to see a temporal error (dangling ptr)
    //lmap_del(&lmap_start, (uintptr_t)(foo->y));
    ///////////////
    CHK_ACCESS(copy);
    int y = *copy;
    
    ///////////////
    ////enable for a spatial "error" (going out of bounds)
    //chk_ptr_access(lmap_start, (uintptr_t)(new), sizeof(*(new)));
    //int z = *(new);
    //(void) z;
    ///////////////
    CHK_ACCESS(foo);
    test_s bar = *foo;

    // if the 2 below is changed to sth greater than 6, spatial error
    CHK_ARRAY(char, pld, 2);
    // the original line is at the end, since this actually accesses the memory
    char c = pld[2];

    // now, frees and deletes go hand in hand for each malloc
    // deleting two entries of the same ptr address... but is okay, because duplicates are allowed in lmap
    // NOTE: macro needs to ensure every lmap entry gets deleted
    LMAP_DEL(new);
    LMAP_DEL(copy);
    LMAP_DEL(foo->y);
    LMAP_DEL(foo);
    // lmap_del(&lmap_start, (uintptr_t)new);                                                                          /*+*/
    // lmap_del(&lmap_start, (uintptr_t)copy);                                                                         /*+*/
    // lmap_del(&lmap_start, (uintptr_t)(foo->y));                                                                     /*+*/
    print_vars();
    // NOTE: to free a struct, NEED TO FREE STRUCT MEMBERS BEFORE STRUCT ITSELF!!!

    // lmap_del(&lmap_start, (uintptr_t)foo);                                                                          /*+*/
    print_vars();

    (void) bar;
    (void) y;
    (void) c;

    return 0;
}


int main(void) {return examples_hardened();}
