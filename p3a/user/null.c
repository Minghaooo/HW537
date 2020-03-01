#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int main(int argc, char *argv[]){


   // uchar* p =0;

   
/*
    
    for (int i = 0; i<4; ++i){
        printf(1, "Old %p\n",(uchar)*p);
      *p = (uchar)0xFC;
    ++p;
    }
    //mprotect(p, 8);
    printf(1, "the pointer is %p", p);

    uchar *p1 =0;
    for (int i = 0; i<4; ++i){
        printf(1, "New new %p\n", (unsigned char)*p1);
        ++p1;
    }
*/

char * nullptr = (char*)0;
nullptr[0] = 33;

mprotect(nullptr, 2);

printf(1, "the null 0 is %d, %p\n", nullptr[0], &nullptr[0]);
//nullptr[1] = 44;
printf(1, "the null 1 is %d, %p\n", nullptr[1], &nullptr[1]);

//nullptr[2] = 44;
printf(1, "the null 2 is %d, %p\n", nullptr[2], &nullptr[2]);
nullptr[3] = 44;
printf(1, "the null 3 is %d\n", nullptr[3]);
//nullptr[4] = 44;
//int num[512];
//int numframes = 1;

//dump_allocated(num, numframes);
//printf(1, "the num[1]: %d, the numframes: %d", num[1],  numframes );
exit();

}
