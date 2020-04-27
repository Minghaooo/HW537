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

    void *img_ptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    struct superblock *sb = (struct superblock *) (img_ptr + BSIZE);

    int nblocks = sb->nblocks;
    int ninodes = sb->ninodes;
    int size = sb->size;

    uint bitblocks = size / (BSIZE * 8) + 1;
    uint usedblocks = ninodes / IPB + 1 + bitblocks;

    int start_bit_blocks = BBLOCK(0, ninodes);

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

    int ck9_inodes_used[ninodes];               // check 9 10 11 12
    for (int i = 0; i < ninodes; ++ i) ck9_inodes_used[i] = 0;

    int parent[ninodes];
    for (int i = 0; i < ninodes; ++ i) parent[i] = 0;

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
		        block_used[dip[i].addrs[NDIRECT]] ++;
                uint* cat_indirect = (uint*) (img_ptr + BSIZE * dip[i].addrs[NDIRECT]);
                for (int j = 0; j < BSIZE/sizeof(uint); ++j) {
                    if ((cat_indirect[j] < data_block_addr || cat_indirect[j] >= size) && cat_indirect[j] != 0){
                        fprintf(stderr, "ERROR: bad indirect address in inode.\n");
                        exit(1);
                    }
                    if (cat_indirect[j] != 0){
                        block_used[cat_indirect[j]] ++;
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

    //check 5 6
    for (int i = data_block_addr; i < size; ++ i){
        int byte_num = i / 8;
        int bit_offset = i % 8;
        char* the_byte = (char*)(img_ptr + start_bit_blocks * BSIZE);
        int love = *(the_byte + byte_num) & (1 << bit_offset);
        if (love > 0 && block_used[i] == 0){                // bitmap in use but actually not
            fprintf(stderr, "ERROR: bitmap marks block in use but it is not in use.\n");
            exit(1);
        }
        if(love == 0 && block_used[i] > 0){                 // bitmap not in use but actually is in use
            fprintf(stderr, "ERROR: address used by inode but marked free in bitmap.\n");
            exit(1);
        }
    }

    // check 8
    for (int i = 0; i < ninodes; ++ i){
        if (dip[i].type == 2){
            int ck8_used_blocks = 0;
            for (int j = 0; j < NDIRECT; ++ j){
                if (dip[i].addrs[j] != 0) ck8_used_blocks ++;
            }
            if (dip[i].addrs[NDIRECT] != 0){
                uint* cat_indirect = (uint*) (img_ptr + BSIZE * dip[i].addrs[NDIRECT]);
                for (int k = 0; k < BSIZE / sizeof(uint); ++ k){
                    if (cat_indirect[k] != 0) ck8_used_blocks ++;
                }
            }
            if (! ((int)dip[i].size > (ck8_used_blocks - 1) * BSIZE && (int)dip[i].size <= ck8_used_blocks * BSIZE)){
                fprintf(stderr, "ERROR: incorrect file size in inode.\n");
                exit(1);
            }
        }
    }

    // check 9 10 11 12
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
    ck9_inodes_used[1] ++;


    for (int i = 0; i < ninodes; ++ i){
        if (dip[i].type > 0 && ck9_inodes_used[i] == 0){        // check 9
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



    //parent
    for (int i = 0; i < ninodes; ++ i){
        if(dip[i].type == 1){
            for (int k = 0; k < NDIRECT; ++ k){
                if (dip[i].addrs[k] != 0){
                    uint temp_addr = dip[i].addrs[k];
                    struct dirent* entry = (struct dirent *)(img_ptr + temp_addr * BSIZE);
                    for (int j = 0; j < BSIZE / sizeof(struct dirent); ++ j){
                        if (entry[j].inum != 0 && strcmp(entry[j].name, s1) != 0 && strcmp(entry[j].name, s2) != 0)
                            parent[entry[j].inum] = i;
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
                                parent[entry[j].inum] = i;
                        }

                    }
                }
            }


        }
    }

    parent[1] = 1;

    //ex 1
    for (int i = 0; i < ninodes; ++ i){
        if (dip[i].type == 1){
            uint temp_addr = dip[i].addrs[0];
            struct dirent* entry = (struct dirent *)(img_ptr + temp_addr * BSIZE);
            if (entry[1].inum != parent[i]){
                fprintf(stderr, "ERROR: parent directory mismatch.\n");
                exit(1);
            }
        }
    }

    //ex 2
    int trace_path[ninodes];
    for (int i = 0; i < ninodes; ++ i){
        if (dip[i].type == 1){
            for (int j = 0; j < ninodes; ++ j) trace_path[j] = 0;
            int p = i;
            while(p != 1){
                if( ++ trace_path[p] > 1) {         // a loop
                    fprintf(stderr, "ERROR: inaccessible directory exists.\n");
                    exit(1);
                }
                p = parent[p];
            }
        }
    }



    return 0;
}
