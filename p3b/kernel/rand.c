//
// Created by liuyin14 on 2020/2/29.
//


#include "rand.h"
//#include <stdint.h>
//#include <stdio.h>
//#include <stdlib.h>


unsigned int seed = 1;
//int count = 0;



struct xorshift32_state {
    unsigned int a;
};


static struct xorshift32_state jbn;

//jbn = (struct xorshift32_state *)malloc(sizeof(struct xorshift32_state));


/* The state word must be initialized to non-zero */
unsigned int xorshift32(struct xorshift32_state *state)
{
    /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
    unsigned int x = state->a;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return state->a = x;
}



void xv6_srand (unsigned int seed){
    jbn.a = seed;
//    count = 0;
}

int xv6_rand (void){
    unsigned int uu =  xorshift32(&jbn);
    return uu % XV6_RAND_MAX;
}