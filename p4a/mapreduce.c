//include files
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "mapreduce.h"

#define TABLE_SIZE 10 * 100
#define HashPar 100



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
struct mapper_par_t
{
    Mapper Mapfun;
    char *arg;
};
struct reducer_t
{  
    Reducer reducefun;
    ReduceStateGetter getstate;
    ReduceGetter getfun;
    char *arg;
    int partition_number;
    int pid;
   // char *key, get_state, get_next, int partition_number
};



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
  //  struct ValueNode_t *Vend;
    int num;
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
    pid_t ProcessID;
};

typedef struct HashMap_t HashMap;
typedef struct ValueNode_t ValueNode;
typedef struct KeyHead_t KeyHead;
typedef struct KeyNode_t KeyNode;
typedef struct KeyMem_t keymen;
typedef struct mapper_par_t map_par;
typedef struct reducer_t    reduce_par;

static HashMap *MAPMEM[100];
static HashMap *REDUCERMEN[100];
static HashMap *RESULTMEM;
static map_par *map_paramer;
static reduce_par reduce_param[100];

static Reducer reducefun;
static Combiner MRcomb;
static ReduceGetter reducegetterfun;
static ReduceStateGetter RStateGetterfun;
static Mapper mapfun;



//TODO: init data structure

void KeyNode_init(KeyNode *node)
{
    node->key = NULL;
    node->Knxt = NULL;
    node->Vnxt = NULL;
   // node->Vend = NULL;
    node->num = 0;
};

 void ValueNode_init(ValueNode* vnode){
    vnode->value = NULL;
    vnode->V_nxt = NULL;

  //  ValueNode a;

}

void HashMap_init(HashMap *hmp)
{   
    //KeyHead head[HashPar];
    for(int i=0; i<HashPar; i++)
    {
        //head[i].Knxt = NULL;
        //hmp->hashlist[i] = &head[i];
        KeyHead *head_test;
        head_test = malloc(sizeof(KeyHead));
        head_test->Knxt = NULL;
        hmp->hashlist[i] = head_test;
    }

    hmp->ProcessID =0;

}

char * searchvalue(KeyHead *khead, char* key){

    KeyNode * Kcurrent;
    KeyNode *Klast;
    KeyNode *Kfree;
    ValueNode * Vcurrent;
    char *value = malloc(20);

    printf("search begin\n");
    
    Klast = NULL;

    if(khead->Knxt == NULL)
    return NULL;

    Kcurrent = khead->Knxt;


    if (Kcurrent->Knxt == NULL){
        if (strcmp(Kcurrent->key, key)==0){
            strcpy(value, Kcurrent->Vnxt->value);
            khead->Knxt = Kcurrent->Knxt;
            free(Kcurrent);
            return value;
        }
        return NULL;
    }

while (Kcurrent->Knxt != NULL){
      printf("search begin 01\n");
     // Klast = Kcurrent;

      if (strcmp(Kcurrent->key, key) == 0)
      {
          printf("search begin 02\n");
          strcpy(value, Kcurrent->Vnxt->value);
          if(Klast == NULL){
              khead->Knxt =Kcurrent->Knxt;
              Kfree = Kcurrent;
              free(Kfree);
            
          }
          else
          {
              Klast->Knxt = Kcurrent->Knxt;
              Kfree = Kcurrent;
               free(Kfree);
          }
          
         
          return value;
    }
    printf("search begin 03\n");
    Klast = Kcurrent;

    Kcurrent = Kcurrent->Knxt;
}

   // printf("search begin 06\n");

    return NULL;
}

// first go through the list and find the key, then insert the value
KeyNode* KeyNode_insert(KeyHead *khead, char* key, char* value){
    

   printf("=====insert %s=======\n", key);
    KeyNode* Kcurrent;
    ValueNode* Vcurrent;
    char* abc = malloc(30);
    char* num = malloc(10);
    strcpy(abc, key);
    strcpy(num, value);
    //printf("abc is %s\n\n", abc);
    

    // find the key;
    // if the list is empty
    if(khead->Knxt == NULL){
     //   printf("000\n");
     //  printf("insert to the first, key = %s, val = %s \n", key, value);
        KeyNode *Kpair = malloc(sizeof(KeyNode));
        ValueNode *Vpair = malloc(sizeof(ValueNode));
        Vpair->value = malloc(sizeof(char*)*10);

        KeyNode_init(Kpair);
        ValueNode_init(Vpair);
        Kpair->key = abc;
        Vpair->value = num;

        Kpair->Vnxt = Vpair;
      //  Kpair->Vend = Vpair;
        khead->Knxt = Kpair;
       // printf("the insetred is %s, %s\n", Kpair->key, Kpair->Vnxt->value);
      //  printf("the key is : %s, value is %s\n", MAPMEM[0]->hashlist[2]->Knxt->key, MAPMEM[0]->hashlist[2]->Knxt->Vnxt->value);

        return Kpair;

    }
    else 
    {
        Kcurrent=khead->Knxt;

        if (strcmp(Kcurrent->key, key) == 0)  // ÏàÍ¬µÄkey
        {
         //   printf("have the same key %s, %s\n", Kcurrent->key, Kcurrent->Vnxt->value);
          

            ValueNode *Vpair = malloc(sizeof(ValueNode));
            ValueNode_init(Vpair);
            Vpair->value = num;
            Vpair->V_nxt = Kcurrent->Vnxt;
            Kcurrent->Vnxt = Vpair;
            return Kcurrent;
        }

        while (Kcurrent->Knxt != NULL) // no same key
        {
           // printf("key exist\n");
            if (strcmp(Kcurrent->key,key)==0) {
            

                ValueNode *Vpair = malloc(sizeof(ValueNode));
                ValueNode_init(Vpair);
                Vpair->value = num;
                Vpair->V_nxt = Kcurrent->Vnxt;
                Kcurrent->Vnxt = Vpair;

                return Kcurrent;
                break;
            }
            else{
                Kcurrent = Kcurrent->Knxt;
            }
        }

       

        KeyNode *Kpair = malloc(sizeof(KeyNode));
        ValueNode *Vpair = malloc(sizeof(ValueNode));
        KeyNode_init(Kpair);
        ValueNode_init(Vpair);
        Kpair->key = abc;
        Vpair->value = num;
        Kpair->Vnxt = Vpair;
       // printf("key exist2%s, %s\n", Kpair->key, Kpair->Vnxt->value);

        Kcurrent->Knxt = Kpair;
      //  printf("key node is %s, %s\n", Kcurrent->Knxt->key, Kcurrent->Vnxt->value);
        //   Kcurrent->Vend = Vpair;
        return Kpair;
    }
}

void free_Vnode(KeyNode* knode){
    
    ValueNode* vnode;
    vnode = knode->Vnxt;

    knode->Vnxt = knode->Vnxt->V_nxt;
    free(vnode);
}


// combine getter
extern  char* GetterComb(char* key){
    //printf("----getrun----- \n");
    KeyNode * key_sel;
    key_sel = (KeyNode *)key;

    ValueNode *vsel;
    char * revalue;
    revalue = malloc(sizeof(ValueNode));


    if (key_sel->Vnxt == NULL){
        return NULL;
    }
    else
    {
     //   printf("* not empty \n");
        vsel = key_sel->Vnxt;
        if (key_sel->Vnxt->V_nxt == NULL){
            key_sel->Vnxt = NULL;
 
        }else
        {
            key_sel->Vnxt = vsel->V_nxt;
        }
        
        revalue =  vsel->value;
        free (vsel);
        return revalue;
    }

}


extern char* ReduceGetter_help(char *key, int partition_number){

 //   printf("int the getter\n");
    unsigned long hsn = MR_DefaultHashPartition(key, 10);
    char* value;
 //   printf("search start\n");

    value = searchvalue(RESULTMEM->hashlist[hsn], key);
    printf(" getter return %s\n", value);

    return value;
    
}

// reduce getter
extern char *ReduceStateGetter_help(char *key, int partition_number){

   unsigned long hsn = MR_DefaultHashPartition(key, 10);

   char *value;

   value = searchvalue(REDUCERMEN[0]->hashlist[hsn], key);
   printf("state return %s\n", value);

   return value;

   /*
   value = searchvalue(REDUCERMEN[0]->hashlist[hsn], key);

   return value;

   value = (char *)malloc(10 * sizeof(char));
   sprintf(value, "%d", 0);
   KeyNode *Keyhelper = KeyNode_insert(REDUCERMEN[0]->hashlist[hsn], key, value);

   if (Keyhelper == NULL){
       printf("null");
       return NULL;
   }
else
{
    ValueNode *temp = Keyhelper->Vnxt;

    Keyhelper->Vnxt = temp->V_nxt;

    free(temp);

  //  printf("state return (%s,%s) \n", Keyhelper->key, Keyhelper->Vnxt->value);
    return Keyhelper->Vnxt->value;
    */



  
}

//get_nxt function used to iterate over values that need to be merged

/**
 * @brief propogate data from a mapper to its respective combiner
 * take k/v pairs from single mapper 
 * and temporarily store them so that combine()can retrive and merge its values for each key
 */
void MR_EmitToCombiner(char *key, char *value){
    unsigned long hsn = MR_DefaultHashPartition(key, 10);
     // printf("key is %s,hash is %ld\n",key, hsn);
    KeyNode *keytree = KeyNode_insert(MAPMEM[0]->hashlist[hsn], key, value);

}


/**
 * @brief take k/v pairs from the many different combiners 
 * store them in a way that reducers can access them
 * @param key 
 * @param value 
 */
void MR_EmitToReducer(char *key, char *value){
    KeyNode *key_sel;
    key_sel = (KeyNode *)key;
    unsigned long hsn = MR_DefaultHashPartition(key_sel->key, 10);
    printf("this is emit to reducer: (%s,%s), hash: %ld \n", key_sel->key, value, hsn) ;
    KeyNode_insert(REDUCERMEN[0]->hashlist[hsn], key_sel->key, value);
    //printf("this is emit to reducer: (%s, %s), hash: %ld \n", REDUCERMEN[0]->hashlist[hsn]->Knxt->key, REDUCERMEN[0]->hashlist[hsn]->Knxt->Vnxt->value, hsn);
    
      printf("----emit to reduce finish------\n\n");

}


//get merget result using REduce sate getter and sotre the partial result 
// using ME_emitReducerState
void MR_EmitReducerState(char *key, char *state, int partition_number){
    unsigned long hsn = MR_DefaultHashPartition(key, 10);

    KeyNode *keyhelp =  KeyNode_insert(RESULTMEM->hashlist[hsn], key, state);
/*
    if(keyhelp->Vnxt->V_nxt != NULL){
        ValueNode * vhelp;
        vhelp = keyhelp->Vnxt->V_nxt;

        while (vhelp->V_nxt != NULL){
            ValueNode *abc;
            abc = vhelp;
            vhelp = abc->V_nxt;
            free(abc);
        }
    }
    */
    
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
    }
    return hash%num_partitions;
}

void * mapper_wrap(){

    mapfun(map_paramer->arg);

    KeyNode *keycurrent;
    KeyNode *key_comb;
    Combiner combineself;
    combineself = MRcomb;

    for (int i =0; i<11;i++)  // int partition number
    {
        printf("i = %d\n", i);
        if (MAPMEM[0]->hashlist[i]->Knxt != NULL){

            key_comb = MAPMEM[0]->hashlist[i]->Knxt;
            combineself((void *)key_comb, GetterComb);
            while (key_comb->Knxt != NULL)
            {
                key_comb = key_comb->Knxt;

                combineself((void *)key_comb, GetterComb);
                printf("combine finish (%s)\n",key_comb->key );

            }
        }

    }
    printf("----------finish mapper--------\n");
}

void *reducer_wrap( void *arg)
{
    reduce_par* argv = arg;

   // printf("before anything\n");

    KeyNode *Kcurrent;
   char *key;

   pthread_t a = pthread_self();
   ReduceStateGetter get_state;
   ReduceGetter get_next;
   get_state = ReduceStateGetter_help;

   //printf("001 before anything\n");

   //printf("          in the MEM the key is %s, value is %s\n", REDUCERMEN[0]->hashlist[2]->Knxt->key,
   //       REDUCERMEN[0]->hashlist[2]->Knxt->Vnxt->value);

   for (int i = 0; i < 11; i++)
   {
      
       if (REDUCERMEN[0]->hashlist[i]->Knxt != NULL)
       {
           printf("\n Warp hash: %d\n", i);

           Kcurrent = REDUCERMEN[0]->hashlist[i]->Knxt;
           key = Kcurrent->key;
         //  printf("002 before anything %s\n", key);
           reducefun(key, ReduceStateGetter_help, ReduceGetter_help, 10);

           while ((Kcurrent->Knxt != NULL))
           {
               key = Kcurrent->key;
            //   printf("002 before anything %s\n", key);
               reducefun(key, ReduceStateGetter_help, ReduceGetter_help, 10);
               Kcurrent = Kcurrent->Knxt;
           }
       }

    }

   

   // (*reducefun)(key,get_state,get_next, 10);
  
   // search REDUCERMEN
   //TODO:
   /**
     * 1. go to sleep 
     * 2. get signaled
     * 3. find the ready key
     * 4. remove it to final structure
     * 5. jump to step 1
     * 
     * 6. all finih: 
     * 7. 
     * 
     * 
     * 
     */

   // 

   // printf("reducer_warps %ld \n",a);//argv->pid);
    // Reduce(char *key, get_state,get_next, int partition_number);
    
   
}
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
void MR_Run(int argc, char *argv[],
            Mapper map, int num_mappers,
            Reducer reduce, int num_reducers,
            Combiner combine,
            Partitioner partition){

          //  reduce(NULL,NULL, NULL,10);

    MRcomb = combine;
    reducefun = reduce;
    mapfun = map;

    pthread_t map_threads[num_mappers];//unsigned long int
    pthread_t reduce_threads[num_reducers];//unsigned long int

    map_paramer = malloc(sizeof(map_par));

        for (int i=0; i<num_mappers;i++){
            MAPMEM[i] = malloc(sizeof(HashMap));   // the place to save the received data
            HashMap_init(MAPMEM[i]);
        }
        for (int i = 0; i < num_mappers;i++)
        {
           REDUCERMEN[i] = malloc(sizeof(HashMap)); // reduce to map 
           HashMap_init(REDUCERMEN[i]);
        }
        RESULTMEM = malloc(sizeof(HashMap)); // the final stage
        HashMap_init(RESULTMEM);

        map_paramer->Mapfun = map;
        for (int i = 0; i < num_mappers; i++) {
            map_paramer->arg = argv[i+1];
            pthread_create(&map_threads[i], NULL, mapper_wrap, NULL);
        }

    // wait for threads to join;
        
         for (int i = 0; i < num_mappers ; i++){
             pthread_join(map_threads[i],NULL);
         }

      //   printf("          in the MEM the key is %s, value is %s\n", REDUCERMEN[0]->hashlist[8]->Knxt->Knxt->key,
      //          REDUCERMEN[0]->hashlist[8]->Knxt->Knxt->Vnxt->value);

         //reduce_param
         //  map_paramer->Mapfun =(void *) reduce;

         //printf("(%s, %s),(%s, %s)",REDUCERMEN[0]->hashlist[])

             for (int i = 0; i < num_mappers; i++)
         {
             reduce_param[i].pid = i;
                 ///map_paramer->arg = argv[i + 1];
                 pthread_create(&reduce_threads[i], NULL, reducer_wrap,(void*) &(reduce_param[i]));
                // printf("main %ld",reduce_threads[i] );
         }

         for (int i = 0; i < num_mappers; i++)
         {
             pthread_join(reduce_threads[i], NULL);
         }

    for (int i = 0; i < num_mappers; i++)
    {
       free (MAPMEM[i]); // the place to save the received data
    }

  //  printf("     read from map: %s \n", MAPMEM[0]->hashlist[0]->Knxt->key);
   // printf("     read from value: %s \n", MAPMEM[0]->hashlist[0]->Knxt->Vend->value);

    //printf("read from map: %d \n", MAPMEM[0]->hashlist[0]->Knxt->sum);

    /*
    for (int i = 0; i < num_mappers; i++)
    {
        //  mapper_args = argv[i+1];
        //   pthread_create(&map_threads[i],NULL,mapper_wrapper,argv[i+1]);
        pthread_create(&map_threads[i], NULL, (void *)map, argv[i + 1]);
    }

*/

}