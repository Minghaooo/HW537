#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>
#include <string.h>

/*
#include <assert.h>
#include <dirent.h>
#include <stdbool.h>
*/
#define stat xv6_stat // avoid clash with host struct stat
#define dirent xv6_dirent
//#include "types.h"
#include "fs.h"
//#include "stat.h"
#undef stat
#undef dirent

typedef unsigned int uint;

// mmap | map a file's contents to virtual memory //very powerful
//-----------
//file contents
//-----------
//heap
//-----------
//stack
//once the call finish img_ptr has the contents of fd

// xv6 fs img similar to vsfs
//unused | superblock | inode table[25] | unused | bitmap(data) | data blocks[995]
//0      | 1          | 2.....26        |27,28                  |data blocks
// totoal 1024 blocks
//with 512 bits you can store 4096 bits -- so we have enough bits for our data blocks



void *img_ptr; // the first addreess of block;
struct dinode *dip;  //the address of inode
struct superblock *sb; // the addreess of superblock
int bitmap [10000];
int bitnode [2000];
int Block_ptr=0;
const char *s1 = ".";
const char *s2 = "..";

void error(char *msg)
{
    fprintf(stderr,"%s\n",msg);
    exit(1);
}

void print_inode (struct dinode *dip);
int walk_inode(struct dinode *dip);
void check_SuperBlock();
void  Dir_check(struct dinode *dips, int num);
int Check_BitMap(int addr);
void Map_bitmap();
void Size_check(int blockcount, int dipsize);
void Inode_check();
void walk_dir_inode(struct dinode *dip);


int main(int argc, char *argv[]){
    int fd;
    //usage is something like <my prog> <fs.img>
    if (argc == 2){
        fd = open(argv[1],O_RDONLY);
    }else{
        error("wrong usage");
    }
    if (fd<0){
        error("image not found.");
    }
    //file opens correctly
    struct stat sbuf;
    fstat(fd, &sbuf);  //get file states
   // printf("Image that I read is %ld in size\n",sbuf.st_size );

   img_ptr = mmap(NULL, sbuf.st_size ,PROT_READ, MAP_PRIVATE,fd,0);
   // the block 0 is boot, unused here;
   // block 1 is superblock;
   sb = (struct superblock *)(img_ptr + BSIZE);
  // printf("Superblock: size %d, nblocks %d, ninodes %d\n", sb->size, sb->nblocks, sb->ninodes);
   // block 2 is Inode Block
   dip = (struct dinode *)(img_ptr + 2 * BSIZE);  // Inode 1
   //dip +1 is root directory
  //  walk_inode(dip+2);

   // how can we access or parse this directories contents
   //struct xv6_dirent *dent = (struct xv6_dirent *)(img_ptr + dip[1].addrs[0] * BSIZE);
    check_SuperBlock();
    Map_bitmap();
    Inode_check();

    //   printf("map bit is %d\n", Check_BitMap(369));
}
// inode is unused again ?
//inode 1 belongs to the root direectory
//inode has 12 direct data blocks data blocks and 1 indirect data blocks

void print_inode(struct dinode *dip){
    printf("file type: %d, ", dip ->type);
    printf("nlink: %d, ", dip->nlink);
    printf("size: %d, ", dip->size);
    printf("addr_first: %d, ", dip->addrs[0]);
    printf("number of blocks: %d\n", dip->size / BSIZE);
    //printf("addr_last: %d\n", (dip.size / BSIZE + dip.addrs[0]));
}
/**
 * @brief calculate all the number of block an inode have 
 * 
 * @param dip 
 * @return int 
 */
int  walk_inode(struct dinode *dip){

    if(dip->type == 0){
    return 0; // this is an unused block
    }
    if((dip->type != 1)&&(dip->type != 2)&&(dip->type != 3)){
        error("ERROR: bad inode.");
    }

    int NumBlocks = 0;
    for (int i =0;i<NDIRECT;i++){
        if(dip->addrs[i] != 0){
           if((dip->addrs[i] >(sb->nblocks ))||(dip->addrs[i]<BBLOCK(0, sb->ninodes))){
               error("ERROR: bad direct address in inode.");
           }
           bitmap[dip->addrs[i]]++; //record the number
           if(bitmap[dip->addrs[i]]>1){
               error("ERROR: direct address used more than once.");
           }
           NumBlocks++;
        }
    }
    if(dip->addrs[NDIRECT] == 0) {
    Size_check(NumBlocks,dip->size);
    return NumBlocks;
    }
    bitmap[dip->addrs[NDIRECT]]++;

    uint *indirect = (uint *)(img_ptr + (dip->addrs[NDIRECT]) *BSIZE );
   // printf("indirect %d\n", dip->addrs[12]);
    for (int i =0; i<NINDIRECT;i++){

        if (*indirect != 0){
            if((*indirect > (sb->nblocks )) || (*indirect < BBLOCK(0, sb->ninodes))){
              error("ERROR: bad indirect address in inode.");
            }
            bitmap[*indirect]++; // count the reference of a block
            NumBlocks++;
        }
        indirect++;
    }
  //  printf("the superblock is %p\n", sb);
  //  printf("the data with extra block used is %d\n", NumBlocks);
    Size_check(NumBlocks, dip->size);
    return NumBlocks;
    //inode 0 is always unused
    //inode 25 in totoal, from 2-26
    //inode 1 is the root dir and has 512 bytes and data is at block 29
    //print_inode(dip[25]);
    //for (int i = 1; dip[i].addrs[0] != 0; i++)
}

void check_SuperBlock(){

    int size = sb->size;
    int ninodes = sb->ninodes;
    int nblocks = sb->nblocks;
    uint bitblocks = size / (BSIZE * 8) + 1;
    uint usedblocks = ninodes / IPB + 1 + bitblocks;
    if (sb->size <= usedblocks + nblocks)
    {
        fprintf(stderr, "ERROR: superblock is corrupted.\n");
        exit(1);
    }

    int total_block = 0;
    Dir_check(dip, 1);
    bitmap[BBLOCK(0, sb->ninodes)+1] = 1;

    for ( int i =2; i<(sb->ninodes);i++){
        total_block+=walk_inode(dip +i);
         Dir_check(dip,i);
    }

  //  printf("the totoal data block used  is %d\n",total_block);
//printf("the totoal block used  is %d\n", blocknum);
}

/**
 * @brief Inode check
 * invalid inode check (check != 1,2,3)
 * in_use node, address is valid 
 * contain . and .. , point to itself
 * bit map
 * each direct address is only used once
 * 
 */
void Dir_check(struct dinode *dips, int num){
    struct dinode *dip = dips +num;
    if(dip->type == 1){
        struct xv6_dirent *dir = (struct xv6_dirent *)(img_ptr + (dip->addrs[0])*BSIZE);
       // struct xv6_dirent *dir_nxt = (struct xv6_dirent *)(img_ptr + (dip->addrs[1])*BSIZE);
       // printf("dir ent has name %s and inum %d, the num is %d \n", dir->name, dir->inum, num);
        if ((dir->inum != num)||(strcmp(".", dir->name) != 0))
        {
            error("ERROR: directory not properly formatted.");
        }
        dir++;
       // printf("dir ent has name %s and inum %d, the num is %d \n", dir->name, dir->inum, num);
        if (strcmp("..", dir->name) != 0)
        {
            error("ERROR: directory not properly formatted.");
        }
    }
}

int Check_BitMap(int addr){

    //printf("the num of inodes is :%ld\n", BBLOCK(0, sb->ninodes));
   // char *bitmap = (char *)(img_ptr + BBLOCK(0, sb->ninodes) * BSIZE); //imgptr +28*Baize
    char *bitmap = (char *)(img_ptr + BBLOCK(0, sb->ninodes) * BSIZE); //imgptr +28*Baize
    int byte_num = (addr/8);
    int bit_offset = addr%8;
    int statue = *(bitmap + byte_num) & (1 << bit_offset);
    if(statue > 0)
        return 1;
    else
        return 0;
  //  printf("the num is %x\n", *(bitmap + byte_num));
  //  printf("byte: %d ,offset %d, %x\n", byte_num, bit_offset, *(bitmap + byte_num) & (1 << bit_offset));
}
void Map_bitmap(){
    for (int i = BBLOCK(0, sb->ninodes)+1; i < sb->nblocks; i++)
    {
            //printf("the block num is %d, stat:%d, bitmap%d\n", i, bitmap[i], Check_BitMap(i));
        if((bitmap[i]==0) && (Check_BitMap(i)==1)){
            error("ERROR: bitmap marks block in use but it is not in use.");
        }
        if ((bitmap[i] > 0) && (Check_BitMap(i) == 0)){
            error("ERROR: address used by inode but marked free in bitmap.");
        }
    }
}

void Size_check(int blockcount, int dipsize){
        if(dipsize>blockcount*BSIZE || dipsize <(blockcount-1)*BSIZE){
            error("ERROR: incorrect file size in inode.");
        }


}

//static int ck9_inodes_used[1000];
void Inode_check(){

   //s int nblocks = sb->nblocks;
    int ninodes = sb->ninodes;
    //int size = sb->size;
    int ck9_inodes_used[ninodes];


    for (int i = 0; i < ninodes; ++i)
        ck9_inodes_used[i] = 0;

    for (int i = 0; i < ninodes; ++i)
    {
        if(dip[i].type == 1){
            for (int k = 0; k < NDIRECT; ++ k){
                if (dip[i].addrs[k] != 0){
                    uint temp_addr = dip[i].addrs[k];
                    struct xv6_dirent* entry = (struct xv6_dirent *)(img_ptr + temp_addr * BSIZE);
                    for (int j = 0; j < BSIZE / sizeof(struct xv6_dirent); ++ j){
                        if (entry[j].inum != 0 && strcmp(entry[j].name, s1) != 0 && strcmp(entry[j].name, s2) != 0)
                            ck9_inodes_used[entry[j].inum] ++;
                    }
                }
            }
            if (dip[i].addrs[NDIRECT] != 0){
                uint* cat_indirect = (uint*) (img_ptr + BSIZE * dip[i].addrs[NDIRECT]);
                for(int l = 0; l < BSIZE / sizeof(uint); ++ l) {
                    if (cat_indirect[l] != 0) {
                        struct xv6_dirent *entry = (struct xv6_dirent *) (img_ptr + cat_indirect[l] * BSIZE);
                        for (int j = 0; j < BSIZE / sizeof(struct xv6_dirent); ++j) {
                            if (entry[j].inum != 0 && strcmp(entry[j].name, s1) != 0 && strcmp(entry[j].name, s2) != 0)
                                ck9_inodes_used[entry[j].inum]++;
                        }

                    }
                }
            }


        }
    }
//    printf("number of entries: %ld\n", dip[8].size / sizeof(struct dirent));

    ck9_inodes_used[1] ++;
    
   
    for (int i = 0; i < ninodes; ++ i){
        if (dip[i].type > 0 && ck9_inodes_used[i] == 0){        // check 9
//            printf("inode num: %d\n\n", i);
            fprintf(stderr, "ERROR: inode marked used but not found in a directory.\n");
            exit(1);
        }
        if (dip[i].type == 0 && ck9_inodes_used[i] > 0){        // check 10
            fprintf(stderr, "ERROR: inode referred to in directory but marked free.\n");
            exit(1);
        }
        if(dip[i].type == 2){
            if (dip[i].nlink != ck9_inodes_used[i]){            // check 11
                fprintf(stderr, "ERROR: bad reference count for file.\n");
                exit(1);
            }
        }

        if(dip[i].type == 1){
            if (ck9_inodes_used[i] > 1){                       // check 12
                fprintf(stderr, "ERROR: directory appears more than once in file system.\n");
                exit(1);
            }
        }
    }
}
/*
void walk_dir_inode(struct dinode *dip){
    if(dip->type != 1)
    return ;

    struct xv6_dirent *wdir;
    for (int i=0; i<NDIRECT;i++){
        if(dip->addrs[i]!=0){
            wdir = (struct xv6_dirent *)(img_ptr + (dip->addrs[i]) * BSIZE);
            for(int j = 0; j< 32; j++){
                if(wdir->inum != 0){
                    printf("name:%s, num %d\n", wdir->name, wdir->inum);
                    ck9_inodes_used[wdir->inum]++;
                    wdir ++;
                }
            }
        }
    }

    if(dip->addrs[NDIRECT]==0) return;
    uint *wdir_indirect = (uint *)(img_ptr + (dip->addrs[NDIRECT]) * BSIZE);
    for (int i =0; i<128;i++){
        if(*wdir_indirect != 0){
            wdir = (struct xv6_dirent *)(img_ptr + (*wdir_indirect) * BSIZE);
            if(wdir->inum != 0){
                printf("name:%s, num %d \n", wdir->name, wdir->inum);
                ck9_inodes_used[wdir->inum]++;
                wdir++;
            }
        }
        wdir_indirect++;
    }
}
*/