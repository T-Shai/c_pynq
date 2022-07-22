#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

/*
    Dirty allocation using static memory address
*/
int mem_alloc1(unsigned int** send_buff, unsigned int** recv_buff, size_t len);

/*
    Dynamic allocation using malloc from libc
*/
int mem_alloc2(unsigned int** send_buff, unsigned int** recv_buff, size_t len);

/*
    Dynamic allocation using cma_alloc from libxlnk_cma
*/
int mem_alloc3(unsigned int** send_buff, unsigned int** recv_buff, size_t len);

#endif /* MEM_ALLOC_H */