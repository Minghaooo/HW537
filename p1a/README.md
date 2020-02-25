## CS 537 Project 1a README
* Name: Mignhao Tang 
* mtang64@wisc.edu
* all tests are passed 


1. ###  in the first wis-grep.c file:
    1. getline is used to read from the file 
    2. strstr( )is used to find if the term is in the line

2. ### in the wis-tar file:
    1. strncpy is used to limit the length of the file name
    2. use for iteration to print 00 to make the padding
    3. fwrite is uesd to print the size (8 bytes), because it is ued to write binary to a file 
    4. at last, the context is written to the file line by line.

3. ### in the wis-untar
   1. use malloc to allocate the size of a file. if the file is too large, we can divide and read it part by part
   2. the file is untared according to the form of tar
   3. while the filename is unabe to be read by fread(), 0 is returned and then the file is exited


   
