#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "pstat.h" //tmh

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
  struct pstat pstt;
} ptable;

static struct pstat *pst =&(ptable.pstt);
static struct proc *initproc;

int nextpid = 1;
int current_num =0;
int tick_temp = 0;
int tick_wait=0;
extern void forkret(void);
extern void trapret(void);
//extern int getprocinfo(struct pstat * pssss);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)    // this is where to start tmh
{
  struct proc *p;
  char *sp;
  int i =0;
  acquire(&ptable.lock);
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    i = p - ptable.proc;
     if (p->state == UNUSED) goto found;
  }
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  

  pst->inuse[i] = 1;
  pst->pid[i] = p->pid;
  release(&ptable.lock);

  pst->state[i] = EMBRYO; //tmh initilization 
  pst->priority[i]=3; //at the begining it is in 3;
  pst->ticks[i][0]=0;
  pst->ticks[i][1]=0;
  pst->ticks[i][2]=0;
  pst->ticks[i][3]=0;
  //pst->ticks[i][4]=0;
  pst->wait_ticks[i][0]=0;
  pst->wait_ticks[i][1]=0;
  pst->wait_ticks[i][2]=0;
  pst->wait_ticks[i][3]=0;
  //pst->wait_ticks[i][4]=0;
  

  // Allocate kernel stack if possible.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;


  return p;
}

// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  p = allocproc();
  

  acquire(&ptable.lock);
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;
  release(&ptable.lock);

  for (int i=0; i<NPROC;i++)
    pst->state[i] = UNUSED;

}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  
  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  proc->sz = sz;
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);
 
  pid = np->pid;
  np->state = RUNNABLE;
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");
  


  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  iput(proc->cwd);
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }


  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  pst->state[current_num] = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.

int pstpro[]={0,0,0,0};
//where to start !!!! tttttt
void
scheduler(void)  //tmh main  a signle cycle is 10ms
{
  struct proc *p;
  int prori=0;
  
int i;
  for(;;){         //loop infinite 
    // Enable interrupts on this processor.
    sti();
    //handle priority ++
    pstpro[0] = 0;
    pstpro[1] = 0;
    pstpro[2] = 0;
    pstpro[3] = 0;
    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
//update
    //pst->ticks[current_num][pst->priority[current_num]] += tick_temp;
    //tick_temp = 0;
    //first , get the number of the process
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      i = p - ptable.proc;
      prori = pst->priority[i];
      //pst->inuse[i]=0;
  
      if(p->state == RUNNABLE){
        pstpro[prori]++;
     //   cprintf("%d\n",pstpro[prori] );
        pst->wait_ticks[i][prori] += tick_wait;
        
      }
      if(((pst->wait_ticks[i][0]>640 ) && (pst->priority[i] == 0))||
        ((pst->wait_ticks[i][1]> 320 ) && (pst->priority[i] == 1))||
        ((pst->wait_ticks[i][2]> 160) && (pst->priority[i] == 2))
        )
     {//handle pri++
        
        pst->wait_ticks[i][0] = 0;
        pst->wait_ticks[i][1] = 0;
        pst->wait_ticks[i][2] = 0;
        pst->wait_ticks[i][3] = 0;
        pst->priority[i]++;
      }
    }
    //cprintf("p3: %d ", pstpro[3]);
    //cprintf("p2: %d ", pstpro[2]);
    //cprintf("p1: %d ", pstpro[1]);
    //cprintf("p0: %d ", pstpro[0]);

    //select which to run--------------------------------------

    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    {

      // choosing //
      if ((p->state == RUNNABLE))
      {

        i = p - ptable.proc;
        if ((pstpro[3] > 0) && (pst->priority[i] != 3))
          continue;
        else if ((pstpro[2] > 0) && (pst->priority[i] != 2))
          continue;
        else if ((pstpro[1] > 0) && (pst->priority[i] != 1))
          continue;

        //jump
        // Switch to chosen process.  It is the process's job
        // to release ptable.lock and then reacquire it
        // before jumping back to us.
        current_num = p - ptable.proc;
        proc = p;

        switchuvm(p);
        p->state = RUNNING;
        pst->state[current_num] = RUNNING;

        pst->inuse[current_num] = 1;
        pst->pid[current_num] = p->pid;

        //current_num =i;
    
        // special function, jump or switch from here to whatever is in the context
        swtch(&cpu->scheduler, proc->context);
        switchkvm();
        // Process is done running for now.
        // It should have changed its p->state before coming back.
        proc = 0;
        }
    }

    release(&ptable.lock);
    //pstt->pid[0] = 0;
    //pst->pid[i] = p->pid;
  }
}

// Enter scheduler.  Must hold only **ptable.lock**
// and have changed proc->state.
void
sched(void)  // tmh yield -> sched ->swtch  
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);  // save the current context to proc->context, call cpu->schedulrt
  cpu->intena = intena;

  
}

int temp =0;
// Give up the CPU for one scheduling round.//forced by trap.c
void
yield(void)  // switch the scheduler
{
  //switch back
  
  temp++;
  pst->ticks[current_num][pst->priority[current_num]]++;
  tick_temp = pst->ticks[current_num][pst->priority[current_num]];
  // cprintf("process chosen to run with PID %d and pri %d, pstpro: %d,tick: %d\n", p->pid, pst->priority[current_num], pstpro[pst->priority[current_num]], pst->ticks[current_num][pst->priority[current_num]]); // tmh print
  //cprintf("pid: %d, prior:%d, temp:%d, ticks:%d \n",proc->pid,pst->priority[current_num], tick_temp,pst->ticks[current_num][pst->priority[current_num]]);
  if (((tick_temp%8 ==0) && (pst->priority[current_num] == 3) ) ||
      ((tick_temp%16==0) && (pst->priority[current_num] == 2) ) ||
      ((tick_temp%32==0) && (pst->priority[current_num] == 1) ) ||
      ((pst->priority[current_num] == 3) && (pstpro[3] > 1) && (tick_temp > 0)) ||
      ((pst->priority[current_num] == 2) && (pstpro[2] > 1) && (tick_temp > 1)) ||
      ((pst->priority[current_num] == 1) && (pstpro[1] > 1) && (tick_temp > 3)) ||
      ((pst->priority[current_num] == 0) && (pstpro[0] > 1) && (tick_temp > 63))
      // ((pst->priority[current_num] == 0) && (tick_temp > 32) && (pstpro[1] =1))
  )
  {

    //cprintf("process chosen to run with PID %d and pri %d, pstpro: %d,tick: %d\n", proc->pid, pst->priority[current_num], pstpro[pst->priority[current_num]], pst->ticks[current_num][pst->priority[current_num]]); // tmh print
    //cprintf("pid: %d, prior:%d, temp:%d, ticks:%d, wait_ticks:%d\n", proc->pid, pst->priority[current_num], tick_temp, pst->ticks[current_num][pst->priority[current_num]], pst->wait_ticks[current_num][pst->priority[current_num]]); //  cprintf("get in \n");

    pst->inuse[current_num] = 0;
    acquire(&ptable.lock); //DOC: yieldlock
    proc->state = RUNNABLE;
    pst->state[current_num] = RUNNABLE;
    tick_wait = temp;
    temp =0;

    //demote
    if ( (
      ((tick_temp%8 ==0) && (pst->priority[current_num] == 3) ) ||
      ((tick_temp%16==0) && (pst->priority[current_num] == 2) ) ||
      ((tick_temp%32==0) && (pst->priority[current_num] == 1) )
      ))
    {
     // cprintf("demote\n");
      pst->wait_ticks[current_num][0] = 0;
      pst->wait_ticks[current_num][1] = 0;
      pst->wait_ticks[current_num][2] = 0;
      pst->wait_ticks[current_num][3] = 0;
      pst->priority[current_num]--;
      tick_temp = 0;
    }

    //cprintf("   Yielding process with PID %d and name %s\n", proc->pid, proc->name); // tmh print
    sched(); // yield -> sched -> swtch
    release(&ptable.lock);
  }
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);
  
  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

int getprocinfo(struct pstat * pstt){ //tmh getproc info
  //pstt =&pst;
  //pstt->priority[0] = 1;
  for(int i=0;i<64;i++){
    pstt->inuse[i]=pst->inuse[i];
    pstt->pid[i] = pst->pid[i];
    pstt->priority[i] = pst->priority[i];
    pstt->state[i] = pst->state[i];
    pstt->ticks[i][0] = pst->ticks[i][0];
    pstt->ticks[i][1] = pst->ticks[i][1];
    pstt->ticks[i][2] = pst->ticks[i][2];
    pstt->ticks[i][3] = pst->ticks[i][3];
   // pstt->ticks[i][4] = pst->ticks[i][4];
    pstt->wait_ticks[i][0] = pst->wait_ticks[i][0];
    pstt->wait_ticks[i][1] = pst->wait_ticks[i][1];
    pstt->wait_ticks[i][2] = pst->wait_ticks[i][2];
    pstt->wait_ticks[i][3] = pst->wait_ticks[i][3];
   // pstt->wait_ticks[i][4] = pst->wait_ticks[i][4];
    //wait_ticks[i][4]
    }

   return 0 ;
}

int boostproc(void) //tmh boostproc info
{
  for(int i =0; i<NPROC; i++){
    if (proc->pid == pst->pid[i]){
      if (pst->priority[i] <3){
       // pst->wait_ticks[i][pst->priority[i]]=0;
        pst->priority[i]++;
        pst->wait_ticks[i][0] = 0;
        pst->wait_ticks[i][1] = 0;
        pst->wait_ticks[i][2] = 0;
        pst->wait_ticks[i][3] = 0;
      }
    }

}
 // cprintf("this is boost proc \n");
  return 0;
}