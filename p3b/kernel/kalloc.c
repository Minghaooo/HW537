// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "spinlock.h"
#include "rand.h"

static unsigned int HLB[512];
//static int Current_num;
//static int length;
unsigned int n=0;
//allocator's data structure 
struct run {
  struct run *next;
 // unsigned int Cnum;
};
struct {
  // the lock protect the freelist
  struct spinlock lock;
  struct run *freelist; 
} kmem;
extern char end[]; // first address after kernel loaded from ELF file

// Initialize free list of physical pages.
void
kinit(void)
{
  char *p;

  initlock(&kmem.lock, "kmem");
  // a PTE can only refer to a physical address that aligned 
  p = (char*)PGROUNDUP((uint)end);

  n = (char*)PHYSTOP - p;
  n = n/PGSIZE-1;
  //cprintf("----------------------the num of n is %d---\n", n);
  for(; p +  PGSIZE <= (char*)PHYSTOP; p += PGSIZE) // maybe here change
  {
    //cprintf("the address is %p\n", p);
        kfree(p);
  }

}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;
  if((uint)v % PGSIZE || v < end || (uint)v >= PHYSTOP) 
    panic("kfree");
  //cprintf("the address is %p \n",(uint) v);

  // Fill with junk to catch dangling refs.
  // set every bite in the memory to 1
  //prevent the later code to read the old valid. 
  memset(v, 1, PGSIZE);

  acquire(&kmem.lock);
  // cast v, tp a pointer to struct run
  r = (struct run*)v;
  //record the old start of freelist
  r->next = kmem.freelist;
  //r->Cnum = (unsigned int)v;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
// rewrite it ! tmh

char *
kalloc(void)
{
  struct run *r;
  struct run *current;
 // cprintf("----the num of n is %d---\n", n);
  unsigned int randnum = (xv6_rand());
 // cprintf("randnum 0 =%d  \n", randnum);
  randnum = randnum %n;
//  cprintf("randnum 1 =%d  \n", randnum);
  acquire(&kmem.lock);

//  cprintf(" the head is %p\n", kmem.freelist);

 
 // cprintf(" the current is %p\n", current);
  // when we take the first;
if (randnum == 0){
  r = kmem.freelist;
 // current = kmem.freelist;
  kmem.freelist = r->next;
// when allocate the next
}
else if (randnum == n){
  current = kmem.freelist;
  for (int i=0; i<n-randnum; i++){
    current = current->next;
  }
  r =current->next;
  current->next = NULL;
}
else
{
  current = kmem.freelist;
  for (int i = 0; i < randnum-1 ; i++)
  {
    current = current->next;
  }
  r = current->next;
  current-> next = r->next;
}
  release(&kmem.lock);
//  cprintf("the address allocated is %p \n", r);

  // add the num to the last
  //for (int j = 0; j < 512; j++)
  //{

    //if (HLB[j] == 0)
    //{
      HLB[3820-n] =(uint) r;
//      cprintf("the i is %d, HLB is %d\n" ,3820-n,HLB[3820-n]);
      //  cprintf("randnum = %d,cnum =%d, i= %d\n",randnum, HLB[i], i);
    //  break;
  //}
n--;
 // cprintf("n:%d, cnum:%d\n", n, r->Cnum);
  return (char *)r;
}




//static int kframes[512];

int dump_allocated(int * frames,int numframe )
{

  if (numframe > 3820 - n)
  return -1;



    int j;

    //cprintf("the number kalloc %d\n",numframe );

     for ( j = 0; HLB[j+1]!= 0; j++);

    // } // find the number of j,

    //
    //printf("j = %d\n", j);

    //if (j<numframe)
    //  return -1;

    // if (numframe>j){
  
  
    for (int ii =0; ii < numframe; ii++)
    {
       frames[ii] = HLB[j-ii];
     //  cprintf("HLB:%x, numfram %d\n", HLB[numframe-ii], j-ii);
    }
  
  return 0;
}
