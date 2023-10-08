#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/smp_lock.h>



asmlinkage long sys_cs1550_down(struct cs1550_sem *sem)
{
  mutex_lock(&sem_lock);
  sem->num -=1;
  if(sem->num <0)
  {
    struct cs1550_node* first = (struct cs1550_node*)kmalloc(sizeof(struct cs1550_node),GFP_ATOMIC);
    first->task = NULL;
    first->next =NULL;

    if(sem->head ==NULL)
    {
      seme->head =first;
    }
    else
    {
      sem->tail->next =first;
    }

    
    sem->tail = first;
    set_current_state(TASK_INTERRUPTIBLE);
    mutex_unlock(&sem_lock);
    schedule();
  }
  else
  {
    mutex_unlock(&sem_lock);
  }
  return NULL;
}


asmlinkage long sys_cs1550_up(struct cs1550_sem *sem)
{
  mutex_lock(&sem_lock);
  sem->num+=1;
  if(sem->num <=0)
  {
    struct task_struct* task;
    struct cs1550_node* first =sem->head;


    if(first!=NULL)
    {
    task =first->task;
    if(first ==sem->tail)
    {
      sem->head =NULL;
      sem->tail =NULL;
    }
    else
    {
      sem->head =first->next;
    }
    wake_up_process(task);
    }
    kfree(first);
  }
  mutex_unlock(&sem_lock);
	return 0;
}