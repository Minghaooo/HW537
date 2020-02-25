#include "types.h"
#include "user.h"
#include "param.h"


#define assert(x) if (x) { /* pass */ } else { \
   printf(1, "assert failed %s %s %d\n", #x , __FILE__, __LINE__); \
   exit(); \
   }


// Test4: 2 sleeping, 1 running, and (roughly) 61 zombies
int
main(int argc, char *argv[])
{
  int i, pid;
  // give birth to children
  for(i = 3; i < NPROC; i++) { // 0: init, 1: sh, 2: procs4
    pid = fork();
    if (pid == 0) {
      exit();
    } else {
      kill(pid);
    }
  }

  sleep(5); // wait for children to turn into zombies \(:scream:)/

  // printf(1, "%d processes are currently running.\n", getprocs());
  assert(getprocs() == NPROC);

  printf(1, "TEST PASSED\n");

  for (i = 3; i < NPROC; i++) { // 0: init, 1: sh, 2: procs4
    wait();
  }

  exit();
}
