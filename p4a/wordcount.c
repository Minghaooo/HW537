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
    printf("this is the beginning of map\n");
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

    printf("now in the combiner: \n" );
    int count = 0;
    char *value;

    while ((value = get_next(key)) != NULL) // return the value
    {
        count++; // Emmited Map values are "1"s
        printf("count %d\n", count);
    }
    // Convert integer (count) to string (value)
    printf("count finish\n ");

    value = (char *)malloc(10 * sizeof(char));
    sprintf(value, "%d", count);
    printf("hello001\n");
    MR_EmitToReducer(key, value);

    free(value);

    printf("mapper finish\n");
}

// invoked once per intermediate key

void Reduce(char *key, ReduceStateGetter get_state,
            ReduceGetter get_next, int partition_number)
{
    // `get_state` is only being used for "eager mode" (explained later)
    assert(get_state == NULL);

    int count = 0;

    char *value;
    while ((value = get_next(key, partition_number)) != NULL)
    {
        count += atoi(value);
    }

    // Convert integer (count) to string (value)
    value = (char *)malloc(10 * sizeof(char));
    sprintf(value, "%d", count);

    printf("%s %s\n", key, value);
    free(value);
}

int main(int argc, char *argv[])
{
    MR_Run(argc, argv, Map, 1, Reduce, 10, Combine, MR_DefaultHashPartition);

}