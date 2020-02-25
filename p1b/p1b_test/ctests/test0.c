//Tests opening small number of files

#include "types.h"
#include "user.h"
#include "param.h"


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
  int num_file = 5;
  int fd[num_file];
  int rc1 = getfilenum(getpid());

  openfile("README", num_file, fd);
  int rc2 = getfilenum(getpid());
  if((rc2 - rc1) == num_file)
    printf(1, "TEST PASSED\n");
  else
    printf(1, "TEST FAILED: rc2 - rc1 = %d, expected %d, %s: %d\n", rc2 - rc1 ,num_file, __FILE__, __LINE__);
  closefile(num_file,fd);
  exit();
}
