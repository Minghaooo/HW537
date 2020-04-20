#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>
#include <string.h>

#include <assert.h>
#include <dirent.h>
#include <stdbool.h>

#define state xv6_state // avoid clash with host struct stat
#define dirent xv6_dirent
#include "types.h"
#include "fs.h"
#include "stat.h"
#undef stat
#undef dirent
// xv6 fs img similar to vsfs
//unused | superblock | inode table | unused | bitmap(data) | data blocks

void print_inode (struct dinode dip);

int main (int argc, char *argv[]){
    int fd;
    //usage is something like <my prog> <fs.img>
    if (argc == 2){
        fd = open(argv[1],O_RDONLY);
    }else{
        printf("Usage: program fs.img\n");
        exit(1);
    }
    if (fd<0){
        printf("Usage: %s file not found\n", argv[1]);
        exit(1);
    }

    //file opens correctly
    struct stat sbuf;

    fstat(fd,&sbuf);  //get file states
    printf("Image that I read is %ld in size\n",sbuf.size);

    //(a) use read/write.fread/fwrite to access the file's content 
    //(b) mmap map a file's contents to virtual memory //very powerful
    //
    //-----------
    //file contents
    //-----------
    //heap
    //-----------
    //stack
    //once the call finish img_ptr has the contents of fd
   void *img_ptr = mmap(NULL, sbuf.size,PROT_READ, MAP_PRIVATE,fd,0);
    
    struct dinode *dip = (struct dinode *)(img_ptr +2*BSIZE);

    print_inode(dip[0]);
    print_inode(dip[1]);
    print_inode(dip[2]);

    struct superblock *sb = (struct superblock *(img_ptr +BSIZE);
    printf("size %d, nblocks %d ninodes %d\n", )
}

void print_inode(struct dinode dip){
    printf("file type: %d", dip.type);
    printf("nlink: %d", dip.nlink);
    printf("size:%d", dip.size);
    printf("first_addr:%d\n", dip.addrs[0]);
    printf("second_addr:%d\n", dip.addrs[1]);

}