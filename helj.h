
#include <linux/list.h> //include from files

struct cs1550_sem
{ 
  /* 
  we need the sem id when we search through for up, down, etc. 
  lock and unlock sem obv 
  */
  long value; //readme 
  unsigned long sem_id;//readme 
  spinlock_t lock;//readme 
 // Add also a FIFO queue (see FIFO queues below)
   
  struct list_head waiting_tasks;// this is the list of processes waiting on this specific semaphore
  struct list_head list;// this is the element that points to the previous and next semaphores
};

struct cs1550_task
{ 
  //tasks list and pntr --->check list file for further explanation---esp list_head 
  struct list_head list;//readme 
  struct task_struct *task;//readme 
  
};

