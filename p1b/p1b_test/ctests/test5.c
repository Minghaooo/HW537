//Tests opening closing files

#include "types.h"
#include "user.h"
#include "param.h"

#define assert(x) if (x) { /* pass */ } else { \
   printf(1, "assert failed %s %s %d\n", #x , __FILE__, __LINE__); \
   exit(); \
   }


void openfile(char *file, int howmany, int* fd) {
  int i;
  
  for (i = 0; i < howmany; i++)
  {
    // assumes file opens successfully...
      fd[i] = open(file, 0);
  }
}

void closefile(int howmany, int* fd) {
  int i;
  
  for (i = 0; i < howmany; i++)
  {
      close(fd[i]);
  }
}

int
main(int argc, char *argv[])
{
  int num_file = 10;
  int fd[num_file];
  int num_close = 4;
  

  openfile("README", num_file, fd);
  int rc1 = getfilenum(getpid());
  closefile(num_close,fd);
  int rc2 = getfilenum(getpid());
  //printf(1,"rc1: %d, rc2: %d", rc1, rc2);
  assert((rc1 - rc2) == num_close);
  printf(1, "TEST PASSED\n");
  closefile(num_file,fd);
  exit();
}
