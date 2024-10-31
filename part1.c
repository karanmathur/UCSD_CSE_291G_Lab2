#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#include <sys/mman.h>
#define BUFF_SIZE (32*1024)/8

static inline void lfence() {
    asm volatile("lfence");
}

static inline uint64_t measure_one_block_access_time(uint64_t addr)
{
    uint64_t cycles;

    asm volatile("mov %1, %%r8\n\t"
    "mfence\n\t"
    "lfence\n\t"
    "rdtscp\n\t"
    "mov %%eax, %%edi\n\t"
    "mov (%%r8), %%r8\n\t"
    "rdtscp\n\t"
    "sub %%edi, %%eax\n\t"
    : "=a"(cycles) /*output*/
    : "r"(addr)    /*input*/
    : "r8", "edi"); /*reserved register*/

    return cycles;
}

static inline void clflush(void *v) {
    asm volatile ("clflush 0(%0)": : "r" (v):);
}

int main(int argc, char **argv)
{

	    // Allocate a buffer using huge page
    // See the handout for details about hugepage management
    void *buf= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE |
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);

    if (buf == (void*) - 1) {
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }
    // The first access to a page triggers overhead associated with
    // page allocation, TLB insertion, etc.
    // Thus, we use a dummy write here to trigger page allocation
    // so later access will not suffer from such overhead.
    *((char *)buf) = 1; // dummy write to trigger page allocation
    volatile uint64_t *eviction_buffer = (uint64_t *)malloc(BUFF_SIZE);
    volatile uint64_t *eviction_buffer2 = (uint64_t *)malloc(BUFF_SIZE);
    uint8_t tmp; 
    int l=0;
    // fill L1 with eviction set 

    //for (int i =0; i<BUFF_SIZE/64;i++){
    //    tmp = eviction_buffer[i*64]; 
    //}
    

    // measuring latency for dram  
    for(l=0; l<BUFF_SIZE/8; l++){
    //while (l < BUFF_SIZE) {
    for(int z=0; z < 1000; z++){
    for (int j=0 ; j < (BUFF_SIZE/8) + 64; j++){
        clflush(eviction_buffer+(j*8));
    }
    
    lfence();

    // Flush 
    for (int j=0 ; j < (BUFF_SIZE/8) + 64; j++){
        clflush(eviction_buffer+(j*8));
    }

    lfence();
    // Victim load 
     
    tmp = eviction_buffer2[l*8];
    lfence(); 
    
  // Attacker Reload 
    for (int i =0; i<(BUFF_SIZE/8) + 64;i++){
        tmp = measure_one_block_access_time((uint64_t)(eviction_buffer+(i*8)));
        //printf("%d\n",tmp);
        //lfence();
        eviction_buffer[i*8] = eviction_buffer[i*8] + tmp;
        lfence();
    }
}
    for(int q = 0; q < (BUFF_SIZE/8) + 64; q++){
	if((eviction_buffer[q*8]/1000) < 100) printf("%d --> %d\n",q,l);
	eviction_buffer[q*8] = 0; 
    }

}
/*
    for (int i =0 ; i<BUFF_SIZE/8; i++){
    	printf("%d\n",eviction_buffer[i*8]/1000);
    }
*/


    free((uint8_t *)eviction_buffer);

    return 0; 
}
