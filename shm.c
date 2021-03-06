#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct
{
  struct spinlock lock;
  struct shm_page
  {
    uint id;
    char *frame;
    int refcnt;
  }
  shm_pages[64];
}
shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer)
{
  //pte_t* pte;
  int pos=0;
  void* v;
  int i=0;
  acquire(&(shm_table.lock));
  for (; i< 64; i++)
  {
    //CASE1 : IT ALREADY EXIST
    if(shm_table.shm_pages[i].id==id)//S1: if it finds id in table
    {
      v = ((char*)PGROUNDUP(myproc()->sz));
      mappages(myproc()->pgdir, v, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
      shm_table.shm_pages[i].refcnt++;
      *pointer=v;
      release(&(shm_table.lock));
      return 0;
    }
  }
//CASE 2:case it doesnt find the id in shm_table
  //S1:find an empty entry in shm_table

   for(pos=0;pos<64;pos++)
   {
    if(shm_table.shm_pages[pos].id==0)
    {

      shm_table.shm_pages[pos].id=id; //initialize its id to the id passed in
     //kmalloc a page and store its address in frame
      shm_table.shm_pages[pos].frame = kalloc();
      //lin: dont forget to call memset after kalloc
      memset(shm_table.shm_pages[pos].frame , 0, PGSIZE);
      //map pages,assuming here kalloc doesnt handle mapping need to Check
      v=(char *) PGROUNDUP(myproc()->sz);
      mappages(myproc()->pgdir, v, PGSIZE, V2P(shm_table.shm_pages[pos].frame), PTE_W|PTE_U);
      *pointer=v;
      myproc()->sz+= PGSIZE;
      shm_table.shm_pages[pos].refcnt=1;
      release(&(shm_table.lock));
      return 0;
    }

   }
  release(&(shm_table.lock));
  return 0;
}
int shm_close(int id)
{
   int i;
  //you write this too!
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++)
  {
    //CASE ID exist
    if(shm_table.shm_pages[i].id==id)//if it finds id in table
    {
       shm_table.shm_pages[i].refcnt-=1;
       if(shm_table.shm_pages[i].refcnt==0)//if no one else is using it
        shm_table.shm_pages[i].id=0;
       release(&(shm_table.lock));
      return 0;
    }
  }
  release(&(shm_table.lock));
 // printf("it doesnt exist %s\n",id);
 return 0; //added to remove compiler warning -- you should decide what to return
}
