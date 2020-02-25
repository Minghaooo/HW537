//CS537 Project 1a
//Mignhao Tang mtang64@wisc.edu
//
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

struct stat info;

int main(int argc, char** argv){

    int argc_count = 2;
if (argc < 2){
    printf("wis-tar: tar-file file [...]\n");
    exit(1);
}
if (argc <3){
    printf("wis-tar: tar-file file [...]\n");
    exit(1);
}
    
    FILE *fpw = NULL;

    fpw = fopen(argv[1],"w+");

        while (argc_count < argc){

        FILE *fp = fopen(argv[argc_count],"r");

            int err = stat(argv[argc_count], &info);  //read the file size
            if (err){
                printf("wis-tar: cannot open file\n");
                    exit(1);
            }

            char *line = NULL;
            size_t linecap = 0;
            ssize_t linelen;
            
                char filename[200];
                
            strncpy(filename ,argv[argc_count],100);
            
            fprintf(fpw,"%s", filename);

           for (int i = 0;i<100-strlen(filename);i++){   //title 100 byte
               fprintf(fpw,"%c",00);
           }

            long filesize = info.st_size;
             fwrite(&filesize,sizeof(long),1,fpw);

            while ((linelen = getline(&line, &linecap, fp)) > 0){    //print the content 
                fprintf(fpw,"%s", line);
            }

            fclose(fp);
            argc_count ++;
    }

    fclose(fpw);

}