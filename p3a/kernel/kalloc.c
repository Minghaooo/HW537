// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "spinlock.h"

static int HLB[512];
int n=0;
//allocator's data structure 
struct run {
  struct run *next;
  int Cnum;
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
  //char *p2;
  //p2 = p + PGSIZE;
  // it assumes has 224M, (PHYSTOP), use all the memory between to as the initial pool as free memory. 
  // 1 3  5 7 9 11 13 15 
  for(; p +  PGSIZE <= (char*)PHYSTOP; p += PGSIZE) // maybe here change
  {
    kfree(p);
  }
  //add memory to the free list
  //for(; p2 + 2* PGSIZE <= (char*)PHYSTOP; p2 += 2*PGSIZE) // maybe here change 
  //add memory to the free list
    //  kfree(p2)
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

  // Fill with junk to catch dangling refs.
  // set every bite in the memory to 1
  //prevent the later code to read the old valid. 
  memset(v, 1, PGSIZE);

  acquire(&kmem.lock);
  // cast v, tp a pointer to struct run
  r = (struct run*)v;
  //record the old start of freelist
  r->next = kmem.freelist;
  r->Cnum = n;
  kmem.freelist = r;
  release(&kmem.lock);
  n++;
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
// rewrite it ! tmh

// 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
// 1   2             
char*
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);

  
  r = kmem.freelist;// head of link list 
  // point the freelist to the second entry

  // just skip one and get xxx the next
  if(r) 
    kmem.freelist = r->next->next;

  release(&kmem.lock);
  
  for (int i =0; i<512; i++){
    if (HLB[i]== 0){
      HLB[i] = r->Cnum;
     // cprintf("cnum =%d, i= %d\n", HLB[i], i);
      break;
    }
  }
  return (char*)r;
}

static int kframes[512];

int * dump_allocated_helper(int numframe){
  int j;

  for ( j = 0; HLB[j+1]!= 0; j++); // find the number of j,
  if (j<numframe){
    kframes[0] = -1;
  }
  else{

    for (int i = 0; i <numframe; i++)
    {
      kframes[i] = HLB[i];
    }
  }

return kframes;
}
