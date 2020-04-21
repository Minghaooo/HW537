gcc -iquote ../include -Wall -Werror -ggdb -o xv6_fsimg_mmap xfsck.c
./xv6_fsimg_mmap /u/m/i/minghao/cs537/HW537/p5/FS_checker/p5/images/Goodlarge

#Addronce  Badfmt     Badindir2  Badrefcnt   Badroot   Badsize  Good       Goodlink    Goodrm   Imrkfree  Indirfree  Mrkused  Superblock
#Badaddr   Badindir1  Badinode   Badrefcnt2  Badroot2  Dironce  Goodlarge  Goodrefcnt  Goodrm2  Imrkused  Mrkfree    Repair