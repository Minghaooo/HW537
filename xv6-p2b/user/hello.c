#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"



int main(int argc, char *argv[])
{
  //struct pstat ppp;
  struct pstat *pst=malloc(sizeof(struct pstat));
  //pst = &ppp;

   //boostproc();
   getprocinfo(pst);

   for (int i = 0; i < 5; i++)
   {
     printf(1, "-------------%d----------------\n", i);
     printf(1, "inuse: %d \n", pst->inuse[i]);
     printf(1, "pid: %d\n", pst->pid[i] );
     printf(1, "prioeity: %d\n", pst->priority[0]);
     printf(1, "procstate: %d\n", pst->state[i]);
     printf(1, "tickes0: %d\n", pst->ticks[i][0]);
     printf(1, "tickes1: %d\n", pst->ticks[i][1]);
     printf(1, "tickes2: %d\n", pst->ticks[i][2]);
     printf(1, "tickes3: %d\n", pst->ticks[i][3]);
     printf(1, "wait0:%d \n", pst->wait_ticks[i][0]);
     printf(1, "wait1:%d \n", pst->wait_ticks[i][1]);
     printf(1, "wait2:%d \n", pst->wait_ticks[i][2]);
     printf(1, "wait3:%d \n", pst->wait_ticks[i][3]);
   }

   // printf(1, "this is hello\n");
    return 0;
}