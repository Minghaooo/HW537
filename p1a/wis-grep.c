//CS537 Project 1a
//Mignhao Tang mtang64@wisc.edu
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>




int  main(int argc, char * argv[] ){

   int argc_count = argc-1;

    if (argc_count  <=0){    // if no argument is specified
        printf("wis-grep: searchterm [file ...]\n");
        exit(1);
    }

    if (argc_count == 1){    // if just a search term then read from input
    
        char *line = NULL;
        size_t linecap = 0;
        ssize_t linelen;

            while ((linelen = getline(&line, &linecap, stdin)) > 0){
                if (strstr(line, argv[1]))
                    printf("%s", line);
            }

    }

    while (argc_count>=2 ){         // if we have several files to search from 
        FILE *fp = fopen(argv[argc_count],"r");
        if(fp == NULL ) {
            printf("wis-grep: cannot open file\n");   // if cannot open the file
            exit(1);
        } 

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;


    while ((linelen = getline(&line, &linecap, fp)) > 0){
        if (strstr(line, argv[1]))              //find if the search term is in the line
            printf("%s", line);
    }
    

    fclose(fp); 
    argc_count--;
    }

return 0;
}