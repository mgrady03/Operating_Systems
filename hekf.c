#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/stddef.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/cs1550.h>
//#define GFP_ATOMIC (__GFP_HIGH)---> GFP_KERNEL as well
//GFP_ATOMIC prevents sleeping by telling the memory allocation code 
//that it's not allowed to sleep to satisfy the allocation
// https://stackoverflow.com/questions/13936795/how-does-gfp-atomic-prevent-sleep
//^this is where i did a little research on GFP!! those notes r for me :)

static LIST_HEAD(sem_list); //readme 
static DEFINE_RWLOCK(sem_rwlock); //readme 
int semaphore_id_value=0; //readme 


/*
Each semaphore has a unique unsigned long integer identifier. 
You are free to generate the identifier at random, or use a global integer value that gets 
incremented with the creation of each semaphore. Each element in the FIFO queue stores a 
pointer to the task control block of a process waiting on the semaphore. 
The value field represents the value of the semaphore.
*/



/**
 * Creates a new semaphore. The long integer value is used to
 * initialize the semaphore's value.
 *
 * The initial `value` must be greater than or equal to zero.
 *
 * On success, returns the identifier of the created
 * semaphore, which can be used with up() and down().
 *
 * On failure, returns -EINVAL or -ENOMEM, depending on the
 * failure condition.
 */
SYSCALL_DEFINE1(cs1550_create, long, value) //create 
{
	struct cs1550_sem *sem= kmalloc(sizeof(struct cs1550_sem), GFP_KERNEL);// some memory allocation and initialization routine
	//GFP_ATOMIC prevents sleeping by telling the memory allocation code 
	//that it's not allowed to sleep to satisfy the allocation
					
	if(sem==NULL) //cant be null or 0
		return -ENOMEM;
	else if(value<0)
		return -EINVAL;
	
	
	INIT_LIST_HEAD(&sem->list);						
	INIT_LIST_HEAD(&sem->waiting_tasks);// initialize head and tail	
					
	//further initliaize other variables from sem glblb sem list...
	sem->sem_id=semaphore_id_value;
	spin_lock_init(&sem->lock);
	sem->value=value; //basically this to this 
					
					
	write_lock(&sem_rwlock); //lock
	list_add(&sem->list, &sem_list); //add to list of sem
	semaphore_id_value+=1; //new sem
	write_unlock(&sem_rwlock);
		
	
	
	return sem->sem_id; //sem created, sem id new to search in list 
}

/**
 * Performs the down() operation on an existing semaphore
 * using the semaphore identifier obtained from a previous call
 * to cs1550_create().
 *
 * This decrements the value of the semaphore, and *may cause* the
 * calling process to sleep (if the semaphore's value goes below 0)
 * until up() is called on the semaphore by another process.
 *
 * Returns 0 when successful, or -EINVAL or -ENOMEM if an error
 * occurred.
 */
SYSCALL_DEFINE1(cs1550_down, long, sem_id)//down 
{
	struct cs1550_sem *sem=NULL;// some memory allocation and initialization routine
	
	read_lock(&sem_rwlock); //readme...protection!
	list_for_each_entry(sem, &sem_list, list) //read me..this loops through glbl semaphore list until the semaphore i d matches 
	{ //^there is no semicolon after this it took me forever to find that tiny buggy and i was stupid 
		if(sem->sem_id==sem_id) //we gots a match 
		{
			spin_lock(&sem->lock); //lock
			
			
			
			sem->value-=1;//sub 1 
			if(sem->value<0) //time 2 sleep
			{
				struct cs1550_task *task_node= kmalloc(sizeof(struct cs1550_task), GFP_KERNEL);// some memory allocation and initialization routine
				//^alloc 
				if(task_node==NULL) //no mem
				{
					
					spin_unlock(&sem->lock); //lock it up
					read_unlock(&sem_rwlock);
					return -ENOMEM; // error
				
				}
				
				INIT_LIST_HEAD(&task_node->list);//new task == alloc ok 
				list_add_tail(&task_node->list, &sem->waiting_tasks);//add to tail
				task_node->task= current; //curr aka current task---from task file and readme 
				//^based on task files in run struct on task...
				
				spin_unlock(&sem->lock);//unlock
				
				
				
				//As part of the down() operation, the current process may need to sleep. 
				//In Linux, you can do that as part of a two-step process.
				//for below 
				set_current_state(TASK_INTERRUPTIBLE); //readme
				schedule(); //readme
				
				
				read_unlock(&sem_rwlock);//unlock  
				return 0;//return 0 bc ok
			}
				spin_unlock(&sem->lock);
				read_unlock(&sem_rwlock);
				return 0;// we return 0 bc its ok
		
		}
		
	
	} //this is end of ID "loop"
	read_unlock(&sem_rwlock); //no sem matches smepahore id 
	return -EINVAL; //return errorororor -einval 
}

/**
 * Performs the up() operation on an existing semaphore
 * using the semaphore identifier obtained from a previous call
 * to cs1550_create().
 *
 * This increments the value of the semaphore, and *may cause* the
 * calling process to wake up a process waiting on the semaphore,
 * if such a process exists in the queue.
 *
 * Returns 0 when successful, or -EINVAL if the semaphore ID is
 * invalid.
 */
SYSCALL_DEFINE1(cs1550_up, long, sem_id)
{
	struct cs1550_sem *sem=NULL;// some memory allocation and initialization routine
	read_lock(&sem_rwlock); //readme...protection!
	
	list_for_each_entry(sem, &sem_list, list) //read me..this loops through glbl semaphore list until the semaphore i d matches 
	{
		if(sem->sem_id==sem_id) //we gots a match
		{
			spin_lock(&sem->lock); //lock
			sem->value+=1; //add 1 bc up adn task
			if(sem->value<=0) //check its not below or equal to 0 
			{
				//if it is then get the first task and delete and wake up and free 
				struct cs1550_task *task_node= list_first_entry(&sem->waiting_tasks, struct cs1550_task, list); //readme hlp
				//^task node temp. first item, sem, struct task , list of task
				list_del(&task_node->list); // readme
				wake_up_process(task_node->task); //readme
				kfree(task_node);//free the current node aka task 
			}

			spin_unlock(&sem->lock);
			read_unlock(&sem_rwlock);
			return 0;
		}
	}
	
	read_unlock(&sem_rwlock); //no sem matches smepahore id 
	return -EINVAL; //return errorororor -einval 
}

/**
 * Removes an already-created semaphore from the system-wide
 * semaphore list using the identifier obtained from a previous
 * call to cs1550_create().
 *
 * Returns 0 when successful or -EINVAL if the semaphore ID is
 * invalid or the semaphore's process queue is not empty.
 */
SYSCALL_DEFINE1(cs1550_close, long, sem_id)
{
	struct cs1550_sem *sem=NULL;// some memory allocation and initialization routine
	
	write_lock(&sem_rwlock); //gblb lock
	
	//cycle through below 
	
	list_for_each_entry(sem, &sem_list, list) //read me..this loops through glbl semaphore list until the semaphore i d matches 
	{
		if(sem->sem_id==sem_id) //we gots a match
		{
			spin_lock(&sem->lock);//protection!
			
			if(!list_empty(&sem->waiting_tasks))//readme...waiting tasks check
			{
				spin_unlock(&sem->lock);
				write_unlock(&sem_rwlock);
				return -EINVAL;
				//^we cant close sem cuz there are waiting tasks 
			}
			
			list_del(&sem->list);//readme...remove sem from gblb list 
			spin_unlock(&sem->lock);
			kfree(sem);//free alloc 
			write_unlock(&sem_rwlock);
			return 0;
				
		}
	}
	
	write_unlock(&sem_rwlock);
	return -EINVAL; //sem not found or empty 
		
}
