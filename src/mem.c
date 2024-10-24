/*****************************************************
 * Copyright Grégory Mounié 2008-2018                *
 * This code is distributed under the GLPv3+ licence.*
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include "mem.h"
#include "mem_internals.h"

/** squelette du TP allocateur memoire */

void *tzl[TZL_SIZE];
MemArena arena = { .chunkpool = NULL, .medium_next_exponant = 0, .small_next_exponant = 0, .TZL = tzl};


/* ecrire votre code ici */


void *emalloc(unsigned long user_request_size)
{
    /*  ecrire votre code ici */
    if (user_request_size == 0)
	return NULL;

    if (user_request_size >= LARGEALLOC)
	return emalloc_large(user_request_size);
    else if (user_request_size <= SMALLALLOC)
	return emalloc_small(user_request_size); // allocation de taille CHUNKSIZE
    else
	return emalloc_medium(user_request_size);
}

void efree(void *ptr)
{
    /* ecrire votre code ici */

    Alloc a = mark_check_and_get_alloc(ptr);
    switch( a.kind ) {
	    case SMALL_KIND:
	    	//printf("free small kind \n");
		efree_small(a);
		break;
	    case MEDIUM_KIND:
	    	//printf("free medium kind \n");
		efree_medium(a);
		break;
	    case LARGE_KIND:
	    	//printf("free large kind \n");
		efree_large(a);
		break;
	    default:
		assert(0);
    }
}
