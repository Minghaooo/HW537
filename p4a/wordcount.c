#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mapreduce.h"

//1. map, produce a set of intermediate key/value pairs

//2. Mapreduce library groups all intermediate values with the same intermediate key 
//and passes them to Reduce()

void Map(char *file_name) // each invocation of map() is handed one filename and 
{
    FILE *fp = fopen(file_name, "r");
    assert(fp != NULL);

    char *line = NULL;
    size_t size = 0;
    // read a line  from the the file 
    while (getline(&line, &size, fp) != -1)
    {
        char *token, *dummy = line;
        while ((token = strsep(&dummy, " \t\n\r")) != NULL)
        {
           // printf(token);
           // printf("\n");
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
void Combine(char *key, CombinerGetter get_next)
{
    int count = 0;
    char *value;
    while ((value = get_next(key)) != NULL)
    {
        count++; // Emmited Map values are "1"s
    }
    char buffer[5] ;
     sprintf(buffer,"%d",count);
   // MR_EmitToReducer(key, itoa(count));// windows
    MR_EmitToReducer(key, buffer); // in linux 
}


// invoked once per intermediate key

void Reduce(char *key, ReduceGetter get_next, int partition_number)
{
    int count = 0;
    char *value;
    while ((value = get_next(key, partition_number)) != NULL)
    {
        count += atoi(value);
    }
    printf("%s %d\n", key, count);
}



int main(int argc, char *argv[])
{
    MR_Run(argc, argv, Map, 2, Reduce, 10, Combine, MR_DefaultHashPartition);

}