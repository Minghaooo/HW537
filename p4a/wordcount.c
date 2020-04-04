#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mapreduce.h"

//1. map, produce a set of intermediate key/value pairs

//2. Mapreduce library groups all intermediate values with the same intermediate key 
//and passes them to Reduce()

void Map(char *file_name)
{
   // printf("this is the beginning of map\n");
    FILE *fp = fopen(file_name, "r");
    assert(fp != NULL);

    char *line = NULL;
    size_t size = 0;
    while (getline(&line, &size, fp) != -1)
    {
        char *token, *dummy = line;
        while ((token = strsep(&dummy, " \t\n\r")) != NULL)
        {
            MR_EmitToCombiner(token, "1");
        }
    }
    free(line);
    fclose(fp);
}

// 3. reducer fuction, accepts an key k and a set of values for the key,
// merge the these values and to form a set of values,
// the intermediate values are supplied to the user's reduce function via an iterator


// combiner partial merging the data emitted by a singer mapper, before it is sent to reducer function.
// it executed as many times as the number of unique keys that map() produce

// combine, merge data from a single map(). and reducer merge from mulitple mappers.

void Combine(char *key, CombineGetter get_next)
{

  //  printf("now in the combiner: \n" );
    int count = 0;
    char *value;

    while ((value = get_next(key)) != NULL) // return the value
    {
        count++; // Emmited Map values are "1"s
    }

    value = (char *)malloc(10 * sizeof(char));
    sprintf(value, "%d", count);
//    printf("hello001\n");
    MR_EmitToReducer(key, value);
    free(value);
}

// invoked once per intermediate key



    // each thread is responsible for a set of keys that are assigned to it

void Reduce(char *key, ReduceStateGetter get_state,
            ReduceGetter get_next, int partition_number)
{
    printf("reduce get key %s\n",key );

    if(key == NULL)
    return ;
    
      char *state = get_state(key, 10);

    
        printf("getstate finish (%s, %s)\n", key, state);

    int count = (state != NULL) ? atoi(state) : 0;

     //   printf("the num is %d\n",count);

    char *value = get_next(key, partition_number);
    printf("get_nxt finish (%s, %s)\n", key, value);

    //printf("hello %s\n", value);

    if (value != NULL)
    {
        count += atoi(value);
        // Convert integer (count) to string (value)
        value = (char *)malloc(10 * sizeof(char));
        sprintf(value, "%d", count);
       // printf("hello02 %s\n", value);

        MR_EmitReducerState(key, value, partition_number);

       
        free(value);
    }
    else
    {
       
        printf("%s %d\n\n", key, count);
    }
    
}

    int main(int argc, char *argv[])
{
    MR_Run(argc, argv, Map, 1, Reduce, 1, Combine, MR_DefaultHashPartition);
}