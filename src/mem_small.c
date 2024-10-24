/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include "mem.h"
#include "mem_internals.h"

// chain the inital block into linked list of 96 bytes block each
void linked_list_init(){

    void *cursor = arena.chunkpool;

    for (int i = 1; i < (1 << (7 + arena.small_next_exponant - 1)); i++){
        //printf("adresse of block %d = %d \n", i, cursor);
        //printf(" %d \n", i);
        *(void **)cursor = cursor + CHUNKSIZE;
        cursor = *(void **)cursor;
    }
    //printf("adresse of last block = %d \n", cursor);
    //printf("adresse of last allocated byte = %d \n", arena.chunkpool + (FIRST_ALLOC_SMALL << (arena.small_next_exponant - 1)));
    printf("total linked blocks = %d \n", (cursor - arena.chunkpool) / CHUNKSIZE);

    return;
}


// count the number of remaining blocks in chunkpool
int count_chunk_pool(){
    void *cursor = arena.chunkpool;
    int count = 0;

    while (cursor != NULL){
        count++;
        cursor = *(void **)cursor;
    }
    return count;
}

void *extract_head_chunk_pool(){
    if (arena.chunkpool != NULL) {
        void *cursor = arena.chunkpool;
        // point arena.chunkpool to the next block
        arena.chunkpool = *(void **)cursor;
        return cursor;
    } else {
        return NULL;
    }
}


void *emalloc_small(unsigned long size)
{
    // call mem_realloc_small if chunkpool is empty
    if (arena.chunkpool == NULL){
        mem_realloc_small();
        linked_list_init();
    }

    // extract the first block from the linkedList
    void *ptr = extract_head_chunk_pool();
    //printf(" chunk pool count = %d  \n", count_chunk_pool());

    return mark_memarea_and_get_user_ptr(ptr, CHUNKSIZE, SMALL_KIND);

}

void efree_small(Alloc a) {
    if (a.ptr != NULL) {
        *(void **)a.ptr = arena.chunkpool;
        arena.chunkpool = a.ptr;
    }
}

// int main(void) {
//
//     mem_realloc_small();
//
//     printf("initial chunk pool = %d \n", arena.chunkpool);
//     linked_list_init();
//
//     // int total_block_count = 1 << (7 + arena.small_next_exponant - 1);
//     //
//     // for (int i = 1; i < total_block_count - 5; i++){
//     //     emalloc_small(50);
//     // }
//
//     printf("total blocks in chunk pool initial = %d \n", count_chunk_pool());
//
//     void *myBlock = emalloc_small(60);
//     void *myBlock2 = emalloc_small(60);
//     Alloc a1 = mark_check_and_get_alloc(myBlock);
//     Alloc a2 = mark_check_and_get_alloc(myBlock2);
//
//     printf("total blocks in chunk pool after allocation = %d \n", count_chunk_pool());
//     // printf(" Alloc size = %lu \n", a.size);
//     // printf(" Alloc adresse = %d \n", a.ptr);
//     // printf(" Alloc kind = %d \n", a.kind);
//     // printf(" myBlock adresse = %d \n", myBlock);
//     efree_small(a1);
//     efree_small(a2);
//
//     printf("total blocks in chunk pool after returning = %d \n", count_chunk_pool());
//
//
//     return 1;
// }