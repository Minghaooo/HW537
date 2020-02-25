//CS537 Project 1a
//Mignhao Tang mtang64@wisc.edu
//
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

struct stat info;

int main(int argc, char** argv){

if (argc < 2){
    printf("wis-untar: tar-file\n");
    exit(1);
}
        char title[99];  // the length of title <= 100
        FILE *fp = fopen(argv[1],"r");
            if(fp == NULL ) {             // if the fp does not exist.
                printf("wis-untar: cannot open file\n"); 
                exit(1);      
            }  
      
       while(1){

            if (!fread(title, 100,1,fp))            //if the file fp is empty or get to the end
                break;                              //then break

            long file_size;                         //read file size here
            fread(&file_size ,sizeof(long),1,fp);

            FILE *fpw = NULL;                       // use the file title to create a file
            fpw = fopen(title,"w+"); 
            
            char*  buffer ; 

            buffer = (char*)malloc(sizeof(char)*file_size); //allocate the size of the buffer

            fread(buffer,file_size,1,fp);           //read and output the content of the file
            fprintf(fpw,"%s", buffer);              // if the file is too large, just use fread(buffer,1,file_size,fp);

            free(buffer);                           // free the space allocated before           
            fclose(fpw);
        }  
    


             fclose(fp);
          


   

}
