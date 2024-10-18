/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <sys/mman.h>
#include <stdint.h>
#include <assert.h>
#include "mem.h"
#include "mem_internals.h"



unsigned int puiss2(unsigned long size) {
    unsigned int p=0;
    size = size -1; // allocation start in 0
    while(size) {  // get the largest bit
	p++;
	size >>= 1;
    }
    if (size > (1 << p))
	p++;
    return p;
}



// extract first block from level [i], return NULL if level is empty;
void *extract_head_from_level(int i){
    //printf("search block in level %d \n", i);
    if (arena.TZL[i] == NULL){
        return NULL;
    } else {
        void *res =  arena.TZL[i];
        // point the linkedlist head to the next block
        arena.TZL[i] = *(void **)res;
        return res;
    }
}

// extract i th block from level, head index = 1
// return NULL if index not valid
void *extract_from_level(int level, int target_idx) {
    if (arena.TZL[level] == NULL) {
        return NULL;
    }
    if (target_idx == 1) {
        return extract_head_from_level(level);
    }
    void *cur = arena.TZL[level];
    void *prev = NULL;
    int current_idx = 1;

    // go to next block only if current not null
    while( cur != NULL) {
        prev = cur;
        cur = *(void **)cur;
        current_idx++;
        if (current_idx == target_idx) {
            // extract the block pointed by current from the linked list
            *(void **)prev = *(void **)cur;
            return cur;
        }
    }
    printf(" target index %d  not valid \n", target_idx);
    return NULL;
}

// go through the linkedlist and check for buddy, return buddy index, 1 if head, 0 if not found
int scan_level_for_buddy(int level, void *adr) {
    int buddy_index = 0;
    void *cur = arena.TZL[level];
    while(cur != NULL){
        buddy_index++;
        if ((unsigned long)adr == ((unsigned long)cur ^ (1 << level))) {
            printf(" buddy found at index %d \n", buddy_index);
            return buddy_index;
        }
        cur = *(void **)cur;
    }
    return 0;
}

void return_block_to_level(int level, void *adr){
    printf(" return block %d to level %d \n", adr, level);
    if (arena.TZL[level] == NULL){
        arena.TZL[level] = adr;
        return;
    }

    // scan level for buddy
    int buddy_index = scan_level_for_buddy(level, adr);
    // if found, extract buddy from level
    void *buddy = extract_from_level(level, buddy_index);
    // merge two buddy blocks, then return the merged block to next level
    if (buddy != NULL) {
        assert( (unsigned long)adr == ((unsigned long)buddy ^ (1 << level)) );

        // erase the size marked on two buddy blocks
        *((unsigned long *)adr + 1)= 0;
        *((unsigned long *)buddy + 1)= 0;
        // erase the addresses marked on two blocks
        *(void **)adr= 0;
        *(void **)buddy= 0;

        if (adr > buddy)
            adr = buddy;

        // mark new size
        *((unsigned long *)adr + 1)= (1 << level + 1);

        return_block_to_level(level + 1, adr);
    } else {
        //if no buddy found, insert the block to the linkedList's head
        *(void **)adr = arena.TZL[level];
        arena.TZL[level] = adr;
    }
    return;
}

void print_linked_list(void **adr){
    while(adr != NULL){
        unsigned long s = *((unsigned long *)adr + 1);
        printf(" %d  size = %d -->", adr, s);
        adr = *adr;
    }
    printf("\n");
}

void print_TZL(int idx){
    for(int i = 0; i <= idx; i++){
        printf("index = %d \n", i);
        print_linked_list(arena.TZL[i]);
    }
}

 
void fill_empty_level(int level){

    assert(arena.TZL[level] == NULL && level < TZL_SIZE);

    int current_max_idx = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;

    if (level == current_max_idx)
    {
        printf(" +++++ Call mem_realloc_medium at level %d \n", level);
        mem_realloc_medium();
        return;
    }
    if (arena.TZL[level + 1] == NULL)
    {
        fill_empty_level(level + 1);
    }
    // level i+1 not empty -> extract the first block of size 2^(i+1) from the linked list
    // split the block in two halfes of size 2^(i), link them, then return the pointer to the first half
    void *new_block =  extract_head_from_level(level + 1);  // new_block has size 2^(i+1)

    // calculate the middle of the extracted block
    void *middle = new_block + (1 << (level));
    // point the first half to 2nd half, forming 2 linked blocks of size 2^i
    *(void **)new_block = middle;

    // mark the block size
    // *((unsigned long *)middle + 1)= (1 << (level));
    // *((unsigned long *)new_block + 1) = (1 << (level));

    // add the two new block to the head of linkedlist at TZL[i]
    arena.TZL[level] = new_block;
}

void *emalloc_medium(unsigned long request_size)
{
    unsigned long total_request_size = request_size + 32;
    arena.TZL[0] = NULL;
    assert(total_request_size < LARGEALLOC);
    assert(total_request_size > SMALLALLOC);

    // calculate the closest indice
    int idx_size = puiss2(total_request_size + 32);
    printf(" >>> Allocation REQUEST at level %d \n", idx_size);

    if (arena.TZL[idx_size] == NULL) {
        fill_empty_level(idx_size);
    }
    void *ptr = extract_head_from_level(idx_size);

    unsigned long allocated_block_size = (1 << idx_size);
    return mark_memarea_and_get_user_ptr(ptr, allocated_block_size, MEDIUM_KIND);;
}

void efree_medium(Alloc a) {
    /* ecrire votre code ici */

    //assert(a.size < LARGEALLOC);
    //assert(a.size > SMALLALLOC);
    // calculate the closest indice
    int idx_size = puiss2(a.size);
    printf(" >>> return block to level %d \n", idx_size);
    return_block_to_level(idx_size, a.ptr);

}


// int main(void) {
//     mem_realloc_medium();
//     print_TZL(10);
//
//     recursive_get_block_from_next_level(5);
//     void *b5 = extract_from_level(5);
//     printf("=========================================== \n");
//     print_TZL(10);
//
//     void *b10 = extract_from_level(8);
//     printf("=========================================== \n");
//     print_TZL(10);
//     return_block_to_level(5, b5);
//     printf("=========================================== \n");
//     //print_TZL(10);

    
    // recursive_get_block_from_next_level(6);
    // void *c = recursive_get_block_from_next_level(6);
    // add_head_linkedlist(6, c);
    // print_TZL(10);
    // printf("==================Check buddy ==================== \n");
    // printf(" buddy of 6 = %d\n", (unsigned long)arena.TZL[6]^(1 << 6));

