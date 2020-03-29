//include files
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "mapreduce.h"
//data structures
// a) store mappers output to be accessed by combiner
// b) store combiners output to be accessed by reducers.


/**
 * @brief propogate data from a mapper to its respective combiner
 * take k/v pairs from single mapper 
 * and temporarily store them so that combine()can retrive and merge its values for each key
 */
void MR_EmitToCombiner(char *key, char *value){

}


/**
 * @brief take k/v pairs from the many different combiners 
 * store them in a way that reducers can access them
 * @param key 
 * @param value 
 */
void MR_EmitToReducer(char *key, char *value){

}



unsigned long MR_DefaultHashPartition(char *key, int num_partitions){
    unsigned long hash = 5381;
    int c;
    while ((c = *key++)!= '\0')
        hash = hash *33+c;

    return hash%num_partitions;
}

//MR-run,
/*
TODO step0: create data sturcture to hold intermediate data
TODO step1: launch some threads to run map function
TODO step2: launch reduced threads to proocess intermediate data
*/
void MR_Run(int argc, char *argv[],
            Mapper map, int num_mappers,
            Reducer reduce, int num_reducers,
            Combiner combine,
            Partitioner partition){


/*
    pthread_t map_pthreads[num_mappers];
    for (int i=0; i<num_mappers;++i){
       // pthread_create(&map_pthreads[i] );
    }


*/
}