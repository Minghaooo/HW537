#include "types.h"
#include "user.h"


#define assert(x) if (x) { /* pass */ } else { \
   printf(1, "assert failed %s %s %d\n", #x , __FILE__, __LINE__); \
   exit(); \
   }


int
main(int argc, char *argv[])
{
  int rc1 = getfilenum(getpid());
  sleep(50);
  int rc2 = getfilenum(getpid());
  assert((rc2 - rc1) == 0);
  printf(1, "TEST PASSED\n");
  exit();
}
