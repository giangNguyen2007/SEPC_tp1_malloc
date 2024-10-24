/******************************************************
 * Copyright Grégory Mounié 2018-2022                 *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include "mem.h"
#include "mem_internals.h"

unsigned long knuth_mmix_one_round(unsigned long in)
{
    return in * 6364136223846793005UL % 1442695040888963407UL;
}

void *mark_memarea_and_get_user_ptr(void *block_ptr, unsigned long size, MemKind k)
{
	/* ecrire votre code ici */
	unsigned long  magic_number = knuth_mmix_one_round((unsigned long)block_ptr & ~(0b11UL));

	unsigned long *head_ptr = (unsigned long*)block_ptr;
	unsigned long *tail_ptr = (unsigned long*)(block_ptr + size - 16);

	// set size
	*head_ptr = size;
	*(tail_ptr + 1) = size;

	unsigned long kind_indice;

	switch (k) {
		case SMALL_KIND:
			kind_indice = 0;
		break;
		case MEDIUM_KIND:
			kind_indice = 1;
		break;
		case LARGE_KIND:
			kind_indice = 2;
		break;
	}

	unsigned long magic_number_modified = magic_number & ~(0b11UL);
	//printf(" magic number last two bits = %lu ", magic_number & (0b11UL));
	//printf(" kind indice = %d ", kind_indice);
	magic_number_modified = magic_number_modified | kind_indice;
	//printf(" modified magic number last two bits = %lu ", magic_number_modified & (0b11UL));

	// set magic number
	*(head_ptr + 1) = magic_number_modified;
	*tail_ptr = magic_number_modified;

	return head_ptr + 2;
}

// *user_ptr : point to first byte of user section, 16 bytes from block head
Alloc mark_check_and_get_alloc(void *user_ptr)
{

	unsigned long *head_ptr = (unsigned long *)(user_ptr - 16);
	unsigned long block_size = *head_ptr;
	unsigned long magic_number = *(head_ptr + 1);

	unsigned long *tail_ptr = (unsigned long *)(user_ptr + block_size - 16*2);
	unsigned long block_size_end = *(tail_ptr + 1);
	unsigned long magic_number_end = *(tail_ptr);

	assert(block_size == block_size_end);
	assert(magic_number == magic_number_end);

	MemKind k;
	unsigned long kind_indice = magic_number & 0b11UL;
	switch (kind_indice) {
		case 0:
			k = SMALL_KIND;
		break;
		case 1:
			k = MEDIUM_KIND;
		break;
		case 2:
			k = LARGE_KIND;
		break;
	}

	Alloc a = { .ptr = (void *)(head_ptr), .kind = k, .size = block_size};
	return a;
}


unsigned long mem_realloc_small() {
    assert(arena.chunkpool == 0);
    unsigned long size = (FIRST_ALLOC_SMALL << arena.small_next_exponant);
    arena.chunkpool = mmap(0,
			   size,
			   PROT_READ | PROT_WRITE | PROT_EXEC,
			   MAP_PRIVATE | MAP_ANONYMOUS,
			   -1,
			   0);
    if (arena.chunkpool == MAP_FAILED)
	handle_fatalError("small realloc");
    arena.small_next_exponant++;
    return size;
}

unsigned long mem_realloc_medium() {
    uint32_t indice = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
    assert(arena.TZL[indice] == 0);
    unsigned long size = (FIRST_ALLOC_MEDIUM << arena.medium_next_exponant);
    assert( size == (1UL << indice));
    arena.TZL[indice] = mmap(0,
			     size*2, // twice the size to allign
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1,
			     0);
    if (arena.TZL[indice] == MAP_FAILED)
	handle_fatalError("medium realloc");
    // align allocation to a multiple of the size
    // for buddy algo
    arena.TZL[indice] += (size - (((intptr_t)arena.TZL[indice]) % size));
    arena.medium_next_exponant++;
    return size; // lie on allocation size, but never free
}


// used for test in buddy algo
unsigned int nb_TZL_entries() {
    int nb = 0;
    for(int i=0; i < TZL_SIZE; i++)
	if ( arena.TZL[i] )
	    nb ++;

    return nb;
}
