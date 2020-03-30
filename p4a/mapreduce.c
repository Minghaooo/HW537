//include files
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "mapreduce.h"

#define TABLE_SIZE 10 * 100
#define HashPar 10
//data structures
/**
 * hash
 * 0    - key1 - key2 - key3 -----
 *         v1     v1
 *         v2     v2
 *         v3
 * 
 * 1    - key1 - key2 - key3 -----
 *         v1     v1
 *         v2     v2
 *         v3
 * 2
 */
//
// a) store mappers output to be accessed by combiner (Arraylist)
// b) store combiners output to be accessed by reducers.

// hashtable's chain list


struct KeyHead_t {
    //char* key;
   struct  KeyNode_t *Knxt;
    //ValueNode *Vnxt;
    //struct Node *last;
};

struct KeyNode_t {
    char* key;
    struct KeyNode_t *Knxt;
    struct ValueNode_t *Vnxt;
    struct ValueNode_t *Vend;
    //struct Node *last;
};

struct ValueNode_t
{
    char *value;
    struct ValueNode_t *V_nxt;
};

// hash table
struct HashMap_t
{
     struct KeyHead_t* hashlist[HashPar];
};

typedef struct HashMap_t HashMap;
typedef struct ValueNode_t ValueNode;
typedef struct KeyHead_t KeyHead;
typedef struct KeyNode_t KeyNode;


void KeyNode_init(KeyNode *node){
    node->key = NULL;
    node->Knxt = NULL;
    node->Vnxt = NULL;
    node->Vend = NULL;

};

 void ValueNode_init(ValueNode* vnode){
    vnode->value = NULL;
    vnode->V_nxt = NULL;
    
    ValueNode a;

}

// first go through the list and find the key, then insert the value
void KeyNode_insert(KeyHead *khead, char* key, char* value){
    //init 
    KeyNode* Kcurrent;
    ValueNode* Vcurrent;

    // find the key;
    // if the list is empty
    if(khead->Knxt == NULL){
        KeyNode *Kpair = malloc(sizeof(KeyNode));
        ValueNode *Vpair = malloc(sizeof(ValueNode));
        KeyNode_init(Kpair);
        ValueNode_init(Vpair);
        Kpair->key = key;
        Vpair->value = value;

        Kpair->Vnxt = Vpair;
        Kpair->Vend = Vpair;

        khead->Knxt = Kpair;

    }
    else 
    {
        Kcurrent=khead->Knxt;
        while (Kcurrent->Knxt != NULL)
        {
            // if the key exists
            if (strcmp(Kcurrent->key,key)==0) {
                ValueNode *Vpair = malloc(sizeof(ValueNode));
                ValueNode_init(Vpair);
                Vpair->value = value;

                // inset the value to the end of the pair
                Kcurrent->Vend->V_nxt = Vpair;
                Kcurrent->Vend = Vpair;
                break;
            }
            else{
                Kcurrent = Kcurrent->Knxt;
            }
        }
        // if key is new:
        KeyNode *Kpair = malloc(sizeof(KeyNode));
        ValueNode *Vpair = malloc(sizeof(ValueNode));
        KeyNode_init(Kpair);
        ValueNode_init(Vpair);
        Kpair->key = key;
        Vpair->value = value;
        Kpair->Vnxt = Vpair;

        Kcurrent->Knxt = Kpair;
        Kcurrent->Vend = Vpair;
    }
}


void free_Vnode(KeyNode* knode){
    
    ValueNode* vnode;
    vnode = knode->Vnxt;

    if(vnode->V_nxt ==NULL){
        knode->Vend = NULL;
    }

    knode->Vnxt = knode->Vnxt->V_nxt;
    free(vnode);

}




//get_nxt function used to iterate over values that need to be merged

/**
 * @brief propogate data from a mapper to its respective combiner
 * take k/v pairs from single mapper 
 * and temporarily store them so that combine()can retrive and merge its values for each key
 */
void MR_EmitToCombiner(char *key, char *value){
    unsigned long hsn = MR_DefaultHashPartition(key, 10);
    //printf("this is in the EmitToCombiner \n ");
    printf("(%s,%s) hash:%ld \n",key, value, hsn);
    


}


/**
 * @brief take k/v pairs from the many different combiners 
 * store them in a way that reducers can access them
 * @param key 
 * @param value 
 */
void MR_EmitToReducer(char *key, char *value){
printf("this is emit to reducer");
}


/**
 * @brief hash function 
 * 
 * @param key 
 * @param num_partitions 
 * @return unsigned long 
 */
unsigned long MR_DefaultHashPartition(char *key, int num_partitions){
    unsigned long hash = 5381;
    int c;
    while ((c = *key++)!= '\0'){
        hash = hash *33+c;
        //printf("c :%d, hash %ld\n",c,hash );
    }

    return hash%num_partitions;
}

//MR-run,
/*
TODO:
 step0: create data sturcture to hold intermediate data
 step1: launch some threads to run map function
 step2: launch reduced threads to proocess intermediate data
*/

/**
 * @brief this is the function called by the map threads
 * we will invoke the mappper here
 * after mapper is done, invoke the combiner here.
 * @param mapper_args 
 * @return void* 
 */
void *mapper_wrapper(void * mapper_args)
{
    printf("this is mapper wrapper %s \n",(char*) mapper_args);
    //map(mapper_args);
}

void MR_Run(int argc, char *argv[],
            Mapper map, int num_mappers,
            Reducer reduce, int num_reducers,
            Combiner combine,
            Partitioner partition){

pthread_t map_threads[num_mappers];//unsigned long int 

//char * mapper_args = "hello";

    for (int i=0;i<num_mappers;i++){
      //  mapper_args = argv[i+1];
     //   pthread_create(&map_threads[i],NULL,mapper_wrapper,argv[i+1]);
      pthread_create(&map_threads[i], NULL, (void*)map, argv[i + 1]);
    }
 



// wait for threads to join;
    for (int i = 0; i < num_mappers ; i++)
    {
        pthread_join(map_threads[i],NULL);
    }
/*
    for (int i = 0; i < num_mappers; i++)
    {
        //  mapper_args = argv[i+1];
        //   pthread_create(&map_threads[i],NULL,mapper_wrapper,argv[i+1]);
        pthread_create(&map_threads[i], NULL, (void *)map, argv[i + 1]);
    }

*/
}