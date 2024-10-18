

// split a block of size 2^(i+1) in two blocks of size 2^i, link them, then return the pointer to the first half
void *split_half(void *adr, int size){

    // calculte
    void *middle = adr + size/2;
    // set the first 8 bytes of the first haft to point the 
    void **ptr = (void **)(adr);
    *ptr = middle; 
    return adr;
}

// add new memory block of size 2^i to the head of TZL[i] linkedlist
void add_tail_linkedlist(int i, void *adr){
    if (arena.TZL[i] == NULL){
        arena.TZL[i] = adr;
        return;
    }
    // if linkedlist is not empty, go to the end of linked list
    void **cur = &(arena.TZL[i]);
    void *v = *cur;
    while (v != NULL){
        v = v + (1 << i) - 8;
        cur = (void **)v;
        v = *cur;
    }
    // link the last block's end to new block
    *cur = adr;
    return;
}


int main2(void) {

  // bt1k_reset();

  void *mem_zone = malloc(100);

  mark_memarea_and_get_user_ptr(mem_zone, 80, SMALL_KIND);
  printf("====== START READING === \n"); 
  Alloc a = mark_check_and_get_alloc(mem_zone);
  printf(" block adresse = %p \n", a.ptr);
  return 1;
}

int test_2(void) {
    printf("area = %d \n", arena.medium_next_exponant);
    int i = 10;
    void *b1 = mmap(0,
			     16, // twice the size to allign
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1,
			     0);
    void *b2 = mmap(0,
			     16, // twice the size to allign
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1,
			     0);
    void *b3 = mmap(0,
			     16, // twice the size to allign
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1,
			     0);
    void *b4 = mmap(0,
			     16, // twice the size to allign
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1,
			     0);
    printf("head b1 = %d \n", b1);
   
    printf("head b1 = %d \n", b1);
    printf("head b2 = %d \n", b2);
    printf("head b3 = %d \n", b3);
    printf("head b4 = %d \n", b4);
    add_head_linkedlist(4, b1);
    add_head_linkedlist(4, b2);
    add_head_linkedlist(4, b3);
    add_head_linkedlist(4, b4);

    print_linked_list(arena.TZL[4]);
    

}