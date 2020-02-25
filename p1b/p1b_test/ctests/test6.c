//Tests child process open close file

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

  int pid, fc1;
  int num_file = 13;
  int close_file = 3;
  pid = fork();
  if (pid > 0) {
    sleep(10);
    fc1 = getfilenum(pid);
    wait();
    assert(fc1 ==num_file - close_file + 3);
    printf(1, "TEST PASSED\n");
    //printf(1,"parent terminated\n");
  } else if(pid == 0){
    int fd[num_file];
    openfile("README", num_file, fd);
    closefile(close_file,fd);
    sleep(20);
    //printf(1,"child terminated\n");
  }

  exit();  
}
