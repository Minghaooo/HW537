//include files
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "mapreduce.h"

#define MaxHashPar 100
#define MaxThreadSize 100



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

//pthread_mutex_lock(&hmp->HashMaplock);
//pthread_mutex_unlock(&hmp->HashMaplock);
//pthread_mutex_destroy(&hmp->HashMaplock);
struct mapper_par_t
{
    Mapper Mapfun;
    char *arg;
    int pid;
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
   pthread_mutex_t HeaeLock;

};

struct KeyNode_t {
    char* key;
    struct KeyNode_t *Knxt;
    struct ValueNode_t *Vnxt;
    int num;
};

struct ValueNode_t
{
    char *value;
    struct ValueNode_t *V_nxt;
};

// hash table
struct HashMap_t
{
     struct KeyHead_t* hashlist[MaxHashPar];
     int Pid_id;
     int finish;
     pthread_mutex_t HashMaplock;
};


typedef struct HashMap_t    HashMap;
typedef struct ValueNode_t  ValueNode;
typedef struct KeyHead_t    KeyHead;
typedef struct KeyNode_t    KeyNode;
typedef struct KeyMem_t     keymen;
typedef struct mapper_par_t map_par;
typedef struct reducer_t    reduce_par;

static HashMap *MAPMEM[MaxThreadSize];
static HashMap *REDUCERMEN[MaxThreadSize];
static HashMap *RESULTMEM;

static map_par              map_paramer[MaxThreadSize];
static reduce_par           reduce_param[MaxThreadSize];

static Reducer              reducefun;
static Mapper               mapfun;
static Combiner             MRcomb;
static Partitioner          MRpartition;
//static ReduceGetter         reducegetterfun;
//static ReduceStateGetter    RStateGetterfun;
static int                  PartitionNum;

static int MapperNum;
static int ReducerNum;
int find_map_num(char *key, HashMap **hsp, int hsn);

pthread_t Global_MAPthread[MaxThreadSize+1];
pthread_t Global_Reducethread[MaxThreadSize + 1];

int ReturnMynum(int type){
pthread_t my_pid = pthread_self();
if(type == 0){
    for (int i = 0; i<MapperNum; i++)
    { 
        //printf("   the process id saved is %ld\n", Global_MAPthread[i]);
        //if (pthread_equal(Global_MAPthread [i], my_pid)==0){
       if (my_pid == Global_MAPthread[i]){
            return i;
       }
    }
}
else

{
    for (int i = 0; Global_Reducethread [i + 1] == 0; i++)
    {
        if (my_pid == Global_Reducethread[i])
            return i;
    }
}
    return MaxThreadSize+100;
}

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
    for(int i=0; i<MaxHashPar; i++)
    {
        KeyHead *head_test;
        head_test = malloc(sizeof(KeyHead));
        head_test->Knxt = NULL; 
        pthread_mutex_init(&head_test->HeaeLock, NULL);
        hmp->hashlist[i] = head_test;

         
    }

        hmp->Pid_id = 99999;
        hmp->finish =0;
        pthread_mutex_init(&hmp->HashMaplock, NULL);
}



char *RetrunFirst(KeyHead *khead, char *key){
    KeyNode *Kcurrent;
    ValueNode *Vcurrent;
    char *value = malloc(20);

 //   printf("return first\n");
    pthread_mutex_lock(&(khead->HeaeLock));

    Kcurrent = khead->Knxt;

    if(khead->Knxt==NULL){
        pthread_mutex_unlock(&(khead->HeaeLock));
        return NULL;
    }


    if (strcmp(Kcurrent->key, key)==0){
        Vcurrent = Kcurrent->Vnxt;
  //      printf("place 1 \n");
        strcpy(value, Kcurrent->Vnxt->value);
        if (Vcurrent->V_nxt == NULL){
   //         printf("place 2 \n");
             khead->Knxt =NULL;
            free(Vcurrent);
            free(Kcurrent);
           
        }
        else{
 //           printf("place 3 \n");
            Kcurrent->Vnxt = Vcurrent->V_nxt;
            free(Vcurrent);
        }
        pthread_mutex_unlock(&(khead->HeaeLock));
        return value;
 //       printf("place 4 \n");
    }
//   printf("place 5 \n");
    pthread_mutex_unlock(&(khead->HeaeLock));
    return NULL;
}

char *RetrunLater(KeyHead *khead, char *key){
    KeyNode *Kcurrent;
    KeyNode *Klast;
    ValueNode *Vcurrent;

    pthread_mutex_lock(&(khead->HeaeLock));

    Kcurrent = khead->Knxt;
    char *value = malloc(20);

    if(khead->Knxt->Knxt == NULL){
    pthread_mutex_unlock(&(khead->HeaeLock));
    return NULL;
    }

    Klast = Kcurrent;
    Kcurrent = Kcurrent->Knxt;
    printf("hello 001\n");
    while(Kcurrent->Knxt!=NULL)
    {
        printf("hello 003\n");
       if( strcmp(Kcurrent->key, key) != 0){
           Klast = Kcurrent;
           Kcurrent = Kcurrent->Knxt;
       }
       else{
           strcpy(value, Kcurrent->Vnxt->value);
           if(Kcurrent->Vnxt->V_nxt == NULL){
               Vcurrent = Kcurrent->Vnxt;
               Klast->Knxt = NULL;
               free(Kcurrent);
               free(Vcurrent);
               pthread_mutex_unlock(&(khead->HeaeLock));
               return value;
           }
           
       }
    }
    printf("hello 002\n");
    if(Kcurrent->Knxt == NULL){
      

        if (strcmp(Kcurrent->key, key) ==0)
        {
            strcpy(value, Kcurrent->Vnxt->value);
            if (Kcurrent->Vnxt->V_nxt == NULL)
            {
                Vcurrent = Kcurrent->Vnxt;
                Klast->Knxt = NULL;
                free(Kcurrent);
                free(Vcurrent);
                pthread_mutex_unlock(&(khead->HeaeLock));
                return value;
            }
            else
            {
                Vcurrent = Kcurrent->Vnxt;
                Kcurrent->Vnxt = Vcurrent->V_nxt;
                free(Vcurrent);
                pthread_mutex_unlock(&(khead->HeaeLock));
                return value;
            }
            
        }
    }
    pthread_mutex_unlock(&(khead->HeaeLock));
    return NULL;
}

char *searchvalue(KeyHead * khead, char *key)
{
//   printf("the hello 000  \n");
    char * value;


    pthread_mutex_lock(&khead->HeaeLock);
    if(khead->Knxt==NULL){
 //       printf("return NULL");
    pthread_mutex_unlock(&khead->HeaeLock);
        return NULL;
    }


    pthread_mutex_unlock(&khead->HeaeLock);
 //   printf("the hello \n");
    value = RetrunFirst(khead, key);
 //   printf("the first%s\n", value);
    if (value != NULL)
        return value;
 

    value = RetrunLater(khead, key);
   // printf("the second %s\n", value);

    return value;
}
KeyNode *KeyNode_insert(KeyHead *khead, char *key, char *value)
{

    // printf("=====insert %s=======\n", key);
    KeyNode* Kcurrent;
    //ValueNode* Vcurrent;
    char* abc = malloc(30);
    char* num = malloc(10);
    strcpy(abc, key);
    strcpy(num, value);
    //printf("abc is %s\n\n", abc);
    pthread_mutex_lock(&khead->HeaeLock);

    if(khead->Knxt == NULL){
   
        KeyNode *Kpair = malloc(sizeof(KeyNode));
        ValueNode *Vpair = malloc(sizeof(ValueNode));
        Vpair->value = malloc(sizeof(char*)*10);
        KeyNode_init(Kpair);
        ValueNode_init(Vpair);
        Kpair->key = abc;
        Vpair->value = num;

        Kpair->Vnxt = Vpair;
     
        khead->Knxt = Kpair;
        pthread_mutex_unlock(&khead->HeaeLock);
        return Kpair;

    }
    else 
    {
        Kcurrent=khead->Knxt;

        if (strcmp(Kcurrent->key, key) == 0)  // ???????key
        {
         //   printf("have the same key %s, %s\n", Kcurrent->key, Kcurrent->Vnxt->value);
          

            ValueNode *Vpair = malloc(sizeof(ValueNode));
            ValueNode_init(Vpair);
            Vpair->value = num;
            Vpair->V_nxt = Kcurrent->Vnxt;
            Kcurrent->Vnxt = Vpair;
            pthread_mutex_unlock(&khead->HeaeLock);
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
                pthread_mutex_unlock(&khead->HeaeLock);

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
       pthread_mutex_unlock(&khead->HeaeLock);
        return Kpair;
    }
    pthread_mutex_unlock(&khead->HeaeLock);
    return NULL;
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
 unsigned long hsn = MRpartition(key, 10);
 char *value;

 //  pthread_mutex_lock(&RESULTMEM->hashlist[hsn]->HeaeLock);
 int a = find_map_num(key, REDUCERMEN, hsn);
 if (a == 99)
     return NULL;

 if (REDUCERMEN[a]->hashlist[hsn] == NULL)
 {
     // printf("hsn nULL\n");
    }

    value = searchvalue(REDUCERMEN[a]->hashlist[hsn], key);
  //  printf("search finish\n");
    return value;
    
}

int find_map_num(char *key, HashMap **hsp, int hsn ){
   KeyNode *Kcurrent;
   //printf("\n lxy in Warp pid: , %s,\n", REDUCERMEN[1]->hashlist[0]->Knxt->Vnxt->value);

   for (int i = 0; i < MapperNum; i++)
   {

       if (REDUCERMEN[i]->hashlist[hsn]->Knxt != NULL)
       {
    
           if( strcmp(key, REDUCERMEN[i]->hashlist[hsn]->Knxt->key)==0)
           {
               return i;
        

           }else
           {
              
               Kcurrent = REDUCERMEN[i]->hashlist[hsn]->Knxt ;
              // Kcurrent = Kcurrent->Knxt;
              if(Kcurrent ==NULL){
              
              }    

               while (Kcurrent->Knxt != NULL){


                   if (strcmp(key, Kcurrent->Knxt->key))
                    {
                       return i;
                    }
                    Kcurrent = Kcurrent->Knxt;
                }
            }
    }
   }
    return 99;
    
}

// reduce getter
extern char *ReduceStateGetter_help(char *key, int partition_number){
    
  //  printf("in reduce state getter to find , %s\n",key);

  unsigned long hsn = MRpartition(key, partition_number);
  // printf("in reduce state getter to find , %s, %ld\n", key, hsn);

    char *value;
    value = searchvalue(RESULTMEM->hashlist[hsn], key);


    return value;
}

//get_nxt function used to iterate over values that need to be merged

/**
 * @brief propogate data from a mapper to its respective combiner
 * take k/v pairs from single mapper 
 * and temporarily store them so that combine()can retrive and merge its values for each key
 */
void MR_EmitToCombiner(char *key, char *value){

    int i = ReturnMynum(0);
    unsigned long hsn = MRpartition(key, 10);
    KeyNode_insert(MAPMEM[i]->hashlist[hsn], key, value);

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
    int i = ReturnMynum(0);
    unsigned long hsn = MRpartition(key_sel->key, 10);
    //printf("this is emit to reducer: (%s,%s), hash: %ld \n", key_sel->key, value, hsn) ;
    KeyNode_insert(REDUCERMEN[i]->hashlist[hsn], key_sel->key, value);
    //printf("this is emit to reducer: (%s, %s), hash: %ld \n", REDUCERMEN[0]->hashlist[hsn]->Knxt->key, REDUCERMEN[0]->hashlist[hsn]->Knxt->Vnxt->value, hsn);
  // printf("----emit to reduce finish------\n\n");

}


//get merget result using REduce sate getter and sotre the partial result 
// using ME_emitReducerState
void MR_EmitReducerState(char *key, char *state, int partition_number){
    unsigned long hsn = MRpartition(key, 10);
  //  printf("emit redce state\n");

    //pthread_mutex_lock(&RESULTMEM->hashlist[hsn]->HeaeLock);
     KeyNode_insert(RESULTMEM->hashlist[hsn], key, state);
   // pthread_mutex_unlock(&RESULTMEM->hashlist[hsn]->HeaeLock);
 //   printf(" -- place state \n");
}

/**
 * @brief hash function 
 * 
 * @param key 
 * @param num_partitions 
 * @return unsigned long 
 */

void *mapper_wrap(void *arg)
{

    
    map_par * map_arg;
    map_arg = arg;

   // int p = ReturnMynum(0);
  //  printf("the thread is  %ld\n", Global_MAPthread[1]);
 //   printf("the file received is %s, in thread %d\n", map_arg->arg, p);
    //pthread_t a = pthread_self();

   // printf("%ld\n", a);
    mapfun(map_arg->arg);
    //KeyNode *keycurrent;

    if(MRcomb != NULL){
    KeyNode *key_comb;
    Combiner combineself;
    combineself = MRcomb;


    for (int i =0; i<11;i++)  // int partition number
    {
       
        if (MAPMEM[map_arg->pid]->hashlist[i]->Knxt != NULL){
       //     printf("i = %d\n", i);

            key_comb = MAPMEM[map_arg->pid]->hashlist[i]->Knxt;
            combineself((void *)key_comb, GetterComb);
            while (key_comb->Knxt != NULL)
            {
                key_comb = key_comb->Knxt;

                combineself((void *)key_comb, GetterComb);
       //         printf("combine finish (%s)\n",key_comb->key );

            }
        }

    }
    }
  //  MAPMEM[map_arg->pid]->finish =1;

 //   printf("----------finish mapper--------\n");
return NULL;
}

//pthread_mutex_lock(&hmp->HashMaplock);
//pthread_mutex_unlock(&hmp->HashMaplock);
//pthread_mutex_destroy(&hmp->HashMaplock);
/**
 * search different 
 * 
 * @param arg 
 * @return void* 
 */

unsigned long MR_DefaultHashPartition(char *key, int num_partitions)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
    {
        hash = hash * 33 + c;
    }
    return hash % num_partitions;
}

void *reducer_stage0(void *arg){
    reduce_par *argv = arg;

    //  printf("----reduce start-------\n");
   // printf("  -------  reduce start %d\n", argv->pid);
    KeyNode *Kcurrent;
    KeyNode *Klast;
    char *key;

    int hashnum;

    //pthread_t a = pthread_self();
    //ReduceStateGetter get_state;
    //ReduceGetter get_next;
   // get_state = ReduceStateGetter_help;

    for (int i = 0; i < MapperNum; i++)
    {
        for (int j = 0; j < PartitionNum; j++)
        {
            hashnum = j % ReducerNum;

              

            if (hashnum == argv->pid)
            {
                //   fflush(stdout);
                //  pthread_mutex_unlock(&REDUCERMEN[i]->hashlist[j]->HeaeLock);
                pthread_mutex_lock(&REDUCERMEN[i]->hashlist[j]->HeaeLock);

                if (REDUCERMEN[i]->hashlist[j]->Knxt != NULL)
                {
                    Kcurrent = REDUCERMEN[i]->hashlist[j]->Knxt;
                    Klast = REDUCERMEN[i]->hashlist[j]->Knxt;
                    key = Kcurrent->key;
                    //printf("j num is %d, pid is, %d, key\n", hashnum, argv->pid);
                    pthread_mutex_unlock(&REDUCERMEN[i]->hashlist[j]->HeaeLock);
                    reducefun(key, ReduceStateGetter_help, ReduceGetter_help, PartitionNum);
                    pthread_mutex_lock(&REDUCERMEN[i]->hashlist[j]->HeaeLock);

                    if (REDUCERMEN[i]->hashlist[j]->Knxt!=NULL){
                    while ((Kcurrent->Knxt != NULL))
                    {
                        
                        key = Kcurrent->key;
                        pthread_mutex_unlock(&REDUCERMEN[i]->hashlist[j]->HeaeLock);
                        reducefun(key, ReduceStateGetter_help, ReduceGetter_help, 10);
                        pthread_mutex_lock(&REDUCERMEN[i]->hashlist[j]->HeaeLock);
                         if (REDUCERMEN[i]->hashlist[j]->Knxt!=NULL){
                            Klast= Kcurrent;
                         }
                         else
                         {
                             break;
                         }
                         
                         if(Klast->Knxt!= NULL)
                         Kcurrent = Kcurrent->Knxt;
                         else
                         {
                             break;
                         }
                         
                    }
                    }
                    pthread_mutex_unlock(&REDUCERMEN[i]->hashlist[j]->HeaeLock);
                }
                pthread_mutex_unlock(&REDUCERMEN[i]->hashlist[j]->HeaeLock);
            }
        }
    }
    return NULL;
}
void reducer_stage2(void *arg){
    reduce_par *argv = arg;

    //  printf("----reduce start-------\n");
//   printf("  -------  reduce start %d\n", argv->pid);
    KeyNode *Kcurrent;
    char *key;

    int hashnum;

    //pthread_t a = pthread_self();
    //ReduceStateGetter get_state;
    //ReduceGetter get_next;
   // get_state = ReduceStateGetter_help;

    for (int i = 0; i < MapperNum; i++){
        for (int j = 0; j < PartitionNum; j++)
        {
            hashnum = (j % ReducerNum) ;

             
            if (hashnum == argv->pid)
            {
               // printf("hasn num is %d, pid is %d, j is %d\n", hashnum, argv->pid, j);

                //   fflush(stdout);
                //  pthread_mutex_unlock(&REDUCERMEN[i]->hashlist[j]->HeaeLock);
                pthread_mutex_lock(&RESULTMEM->hashlist[j]->HeaeLock);

                if (RESULTMEM->hashlist[j]->Knxt != NULL)
                {
                    //printf("hello\n");
                    Kcurrent = RESULTMEM->hashlist[j]->Knxt;
                    key = Kcurrent->key;
                 //   printf("key is %s\n", key);
                    pthread_mutex_unlock(&RESULTMEM->hashlist[j]->HeaeLock);
                    reducefun(key, ReduceStateGetter_help, ReduceGetter_help, PartitionNum);
                    pthread_mutex_lock(&RESULTMEM->hashlist[j]->HeaeLock);

                    while ((Kcurrent->Knxt != NULL))
                    {
                        key = Kcurrent->key;
                        pthread_mutex_unlock(&RESULTMEM->hashlist[j]->HeaeLock);
                        reducefun(key, ReduceStateGetter_help, ReduceGetter_help, 10);
                     //   printf("key is %s\n", key);
                        pthread_mutex_lock(&RESULTMEM->hashlist[j]->HeaeLock);
                        Kcurrent = Kcurrent->Knxt;
                    }
                    pthread_mutex_unlock(&RESULTMEM->hashlist[j]->HeaeLock);
                }
              //  printf("hello 2\n");
                pthread_mutex_unlock(&RESULTMEM->hashlist[j]->HeaeLock);
            }
        }
        }

}

void *reducer_wrap( void *arg)
{
  reducer_stage0(arg);
  reducer_stage2(arg);
  return NULL;
}
  
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

    PartitionNum = 10;
    //  reduce(NULL,NULL, NULL,10);
    MapperNum = num_mappers;
    ReducerNum = num_reducers;

    MRcomb = combine;
    reducefun = reduce;
    mapfun = map;
    MRpartition = partition;

    //initial MAMMEM and REDUCEMEN
        for (int i=0; i<num_mappers;i++){

            MAPMEM[i] = malloc(sizeof(HashMap));   // the place to save the received data
            HashMap_init(MAPMEM[i]);

            REDUCERMEN[i] = malloc(sizeof(HashMap)); // reduce to map
            HashMap_init(REDUCERMEN[i]);
        }
    // the final place to put     
            RESULTMEM = malloc(sizeof(HashMap)); // the final stage
            HashMap_init(RESULTMEM);

    /// start creating threads

       
        for (int i = 0; i < num_mappers; i++) {
            map_paramer[i].pid=i;
            map_paramer[i].arg = argv[i+1];
            pthread_create(&Global_MAPthread[i], NULL, mapper_wrap,(void*)&(map_paramer[i]));
    }

    // wait for threads to join;
        
         for (int i = 0; i < num_mappers ; i++){
             pthread_join(Global_MAPthread[i],NULL);
         }


        for (int i = 0; i < ReducerNum; i++)
         {
                 reduce_param[i].pid = i;
             //    printf("pid nun should be:%d\n",i);
               
                pthread_create(&Global_Reducethread[i], NULL, reducer_wrap,(void*) &(reduce_param[i]));     
         }


        for (int i = 0; i < ReducerNum; i++)
         {
             pthread_join(Global_Reducethread[i], NULL);
         }

        for (int i = 0; i < num_mappers; i++)
        {
           // pthread_mutex_destroy(&);
            free (MAPMEM[i]); // free the place to save the received data
            free(REDUCERMEN[i]);
        }

         free(RESULTMEM);

  
}