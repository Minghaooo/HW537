//
// Created by yinl on 4/20/20.
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>
#include <string.h>

#define stat xv6_stat  // avoid clash with host struct stat
//#define dirent xv6_dirent  // avoid clash with host struct stat
#include "types.h"
#include "fs.h"
#include "stat.h"
#undef stat
//#undef dirent

const char* s1 = ".";
const char* s2 = "..";

int main(int argc, char *argv[]) {
    int fd;
    // Usage is something like <my prog> <fs.img>
    if (argc == 2) {
        fd = open(argv[1], O_RDONLY);
    } else {
        fprintf(stderr, "Usage: xfsck <file_system_image>\n");
        exit(1);
    }

    if (fd < 0) {
        fprintf(stderr, "image not found.\n");
        exit(1);
    }

    struct stat sbuf;
    fstat(fd, &sbuf);
//    printf("Image that i read is %ld in size\n", sbuf.st_size);

    void *img_ptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    struct superblock *sb = (struct superblock *) (img_ptr + BSIZE);
//    printf("size %d nblocks %d ninodes %d\n", sb->size, sb->nblocks, sb->ninodes);

    int nblocks = sb->nblocks;
    int ninodes = sb->ninodes;
    int size = sb->size;


    uint bitblocks = size / (BSIZE * 8) + 1;
    uint usedblocks = ninodes / IPB + 1 + bitblocks;
    if (size <= usedblocks + nblocks){
        fprintf(stderr, "ERROR: superblock is corrupted.\n");
        exit(1);
    }
    struct dinode *dip = (struct dinode *) (img_ptr + 2 * BSIZE);
    uint data_block_addr = dip[1].addrs[0];


    int block_used[size];                   // check7, how many times each block has been in inodes
    for (int i = 0; i < size; ++ i){
        block_used[i] = 0;
    }

    int ck9_inodes_used[ninodes];
    for (int i = 0; i < ninodes; ++ i) ck9_inodes_used[i] = 0;

//    int ck12_inodes_used[ninodes];
//    for (int i = 0; i < ninodes; ++ i) ck12_inodes_used[i] = 0;


    for (int i = 0; i < ninodes; ++ i){
        if (dip[i].type > 3 || dip[i].type < 0){
            fprintf(stderr, "ERROR: bad inode.\n");
            exit(1);
        }
        if (dip[i].type > 0){           // in-use check3

            for(int j = 0; j < NDIRECT; ++j){
                if ((dip[i].addrs[j] < data_block_addr || dip[i].addrs[j] >= size) && dip[i].addrs[j] != 0){
                    fprintf(stderr, "ERROR: bad direct address in inode.\n");
                    exit(1);
                }
                if (dip[i].addrs[j] != 0){      // check 7
                    if (++ block_used[dip[i].addrs[j]] > 1){
                        fprintf(stderr, "ERROR: direct address used more than once.\n");
                        exit(1);
                    }
                }
            }
            if (dip[i].addrs[NDIRECT] > 0){
                if ((dip[i].addrs[NDIRECT] < data_block_addr || dip[i].addrs[NDIRECT] >= size) && dip[i].addrs[NDIRECT] != 0){
                    fprintf(stderr, "ERROR: bad indirect address in inode.\n");
                    exit(1);
                }
                uint* cat_indirect = (uint*) (img_ptr + BSIZE * dip[i].addrs[NDIRECT]);
                for (int j = 0; j < BSIZE/sizeof(uint); ++j) {
                    if ((cat_indirect[j] < data_block_addr || cat_indirect[j] >= size) && cat_indirect[j] != 0){
                        fprintf(stderr, "ERROR: bad indirect address in inode.\n");
                        exit(1);
                    }
                }
            }
        }


        if (dip[i].type == 1){      // check 4
            uint temp_addr = dip[i].addrs[0];
            struct dirent* entry = (struct dirent *)(img_ptr + temp_addr * BSIZE);
            if (strcmp(entry[0].name, s1) != 0 || strcmp(entry[1].name, s2) != 0 || entry[0].inum != i){
                fprintf(stderr, "ERROR: directory not properly formatted.\n");
                exit(1);
            }
        }
    }

    for (int i = 0; i < ninodes; ++ i){
        if(dip[i].type == 1){
            for (int k = 0; k < NDIRECT; ++ k){
                if (dip[i].addrs[k] != 0){
                    uint temp_addr = dip[i].addrs[k];
                    struct dirent* entry = (struct dirent *)(img_ptr + temp_addr * BSIZE);
                    for (int j = 0; j < BSIZE / sizeof(struct dirent); ++ j){
                        if (entry[j].inum != 0 && strcmp(entry[j].name, s1) != 0 && strcmp(entry[j].name, s2) != 0)
                            ck9_inodes_used[entry[j].inum] ++;
                    }
                }
            }
            if (dip[i].addrs[NDIRECT] != 0){
                uint* cat_indirect = (uint*) (img_ptr + BSIZE * dip[i].addrs[NDIRECT]);
                for(int l = 0; l < BSIZE / sizeof(uint); ++ l) {
                    if (cat_indirect[l] != 0) {
                        struct dirent *entry = (struct dirent *) (img_ptr + cat_indirect[l] * BSIZE);
                        for (int j = 0; j < BSIZE / sizeof(struct dirent); ++j) {
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

//    for (int i = 0; i < ninodes; ++ i){
//        printf("inum : %d,  type:  %d, ck9:  %d\n", i, dip[i].type, ck9_inodes_used[i]);
//    }
//    printf("size of dirent: %ld, dirent per block: %ld\n", sizeof(struct dirent), BSIZE / sizeof(struct dirent));

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

//    for(int i = 0; i < ninodes; ++ i){
//        if (dip[i].type == 2){       //check 8
//            int flag = 0;
//            int block_c = 0;
//            for(int j = 0; j < NDIRECT; ++j) {
//                if (dip[i].addrs[j] != 0) block_c++;
//                else{
//                    flag = 1;
//                    break;
//                }
//            }
//            if (flag == 1 || dip[i].addrs[NDIRECT] == 0){
//                if (dip[i].size <= (block_c - 1) * BSIZE || dip[i].size > block_c * BSIZE){
//                    fprintf(stderr, "ERROR: incorrect file size in inode.\n");
//                    exit(1);
//                }
//            } else{
//                uint* cat_indirect = (uint*) (img_ptr + BSIZE * dip[i].addrs[NDIRECT]);
//                for (int j = 0; j < BSIZE/sizeof(uint); ++j) {
//                    if (cat_indirect[j] != 0) block_c ++;
//                    else break;
//                }
//                if (dip[i].size <= (block_c - 1) * BSIZE || dip[i].size > block_c * BSIZE){
//                    fprintf(stderr, "ERROR: incorrect file size in inode.\n");
//                    exit(1);
//                }
//            }
//        }
//    }






    return 0;
}