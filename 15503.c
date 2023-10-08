#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


struct cs1550_sem{
  int num; 
  struct cs1550_node* head; //head 
  struct cs1550_node* tail; //tail
};

void down(struct cs1550_sem *sem) {
       syscall(441, sem); //down syscall----copied from project handout 
}       //syscall number 441 

void up(struct cs1550_sem *sem) {
       syscall(442, sem); //up syscall----copied from project handout 
       //syscall number 442 
}

int main()
{
  //random nums
  srand(time(NULL));

//how many cars 
  int buffer_size = 12; //12 cars max 
  
 //direction the flagperson is looking
  //if a car has honk yet
  //start time
  //the # of cars across both directions
  void *direction_mem = mmap(NULL,sizeof(char),PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
  void *honk = mmap(NULL,sizeof(char),PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
  void *start_memory = mmap(NULL,sizeof(time_t),PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
  void *long_car_memory = mmap(NULL,sizeof(long),PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);



  //point duh the start pointer of each block
  time_t* stime = (time_t*) start_mem;
  long* total_cars = (long*) long_car_mem; //12 is max, incremment by 1 each time and check if >=12


  char* direction = (char*) dir_mem; //direction... north or south. N or S 
  char* honk = (char*) honk; //honk yes or no 


//sem 
  void *sem_memory = mmap(NULL, sizeof(struct cs1550_sem)*6, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
  void *north_memory = mmap(NULL, sizeof(int)*(buffer_size+2), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
  void *south_memory = mmap(NULL, sizeof(int)*(buffer_size+2), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);

 //full n empty spots n and s 
  struct cs1550_sem* mutex = (struct cs1550_sem*)sem_memory;
  struct cs1550_sem* car_sem = (struct cs1550_sem*)sem_memory;


  struct cs1550_sem* north_full = (struct cs1550_sem*)sem_memory; //full aka 12 or more 
  struct cs1550_sem* north_empty = (struct cs1550_sem*)sem_memory; //empty 

  struct cs1550_sem* south_full = (struct cs1550_sem*)sem_memory;
  struct cs1550_sem* south_empty = (struct cs1550_sem*)sem_memory;



  //lines 33-63 kinda j copied from the slides and project handout tbh 






  //sem nums aka initialize 
  mutex->num = 1;//0?
  car_sem->num = 0;

  north_empty->num = buffer_size; //12 empty spots buffer is 12 
  south_empty->num = buffer_size;

  north_full->num = 0; //no cars 
  south_full->num = 0;


 

  *total_cars = 0; //no cars 
  *direction = ' '; //neither north or south for now 
  *honk = 'N'; // honk starts off as no 


  printf("The flagperson is asleep."); //start with flag person asleep. no cars. 

   //producing n consuming 
  *north_producer_ptr = 0; //consumer n producer for both directions 
  *north_consumer_ptr = 0;

  *south_producer_ptr = 0;
  *south_consumer_ptr = 0;

  //south
  int* south_producer_ptr = (int*)south_memory;
  int* south_consumer_ptr = (int*)south_memory;
  int* south_buffer_ptr = (int*)south_memory;


  //north
  int* north_producer_ptr = (int*)north_memory; //ptr for producer north
  int* north_consumer_ptr = (int*)north_memory; // consumer north
  int* north_buffer_ptr= (int*)north_memory; //buffer 

 



  int i;
  for(i = 0; i<buffer_size;i++)
  {
    north_buffer_ptr[i] = 0; //initialize buffers for north and south
    south_buffer_ptr[i] = 0; 
  }
 
  *stime = time(NULL); //start time 



// ok now its go time 


  //forking
  if(fork()==0)
  {
     //printf("The flagperson is now awake.");
    //north
    if(fork()==0)
    {
      while(1)
      {
        down(north_empty);
        down(mutex);
        //increment the total number of cars that have passed through the intersection
        *total_cars = *total_cars + 1;
        //car #
        north_buffer_ptr[*north_producer_ptr%buffer_size] = *total_cars;
        //increment which spot in the buffer
        *north_producer_ptr = *north_producer_ptr + 1;
        //check to see if the car needs to honk
        if(car_sem->num <= 0 && *honk == 'N')
        {
          //HONK!! beep beep
          *honk = 'Y';
          *direction = 'N';
          printf("The flagperson is now awake.");
          printf("Car %d coming from the N direction, blew their horn at time %d.",*total_cars,time(NULL)- *stime);
        }
        //buffer
        printf("Car %d coming from the N direction arrived in the queue at time %d.",*total_cars,time(NULL) - *stime);
        
        up(mutex);
        up(north_full);
        up(car_sem);
        //we sleep now beach
        if(rand()%10 > 7) //70%
          sleep(10);
          //exit(1);
      }
    }
    //south 
    else
    {
      while(1)//infinite
      {
        down(south_empty);
        down(mutex);
        //increment 

        *total_cars = *total_cars + 1; //add one to total 
        //car #
        south_buffer_ptr[*south_producer_ptr%buffer_size] = *total_cars;
        //increment
        *south_producer_ptr = *south_producer_ptr + 1;
        //check honk?



        if(car_sem->num <= 0 && *honk == 'N')
        {
          *honk = 'Y'; //HONK! beep beep
          *direction = 'S';
          printf("The flagperson is now awake.");

          printf("Car %d coming from the S direction, blew their horn at time %d.",*total_cars,time(NULL)-*stime);
        }
        //buffer
        printf("Car %d coming from the S direction arrived in the queue at time %d.",*total_cars,time(NULL)-*stime);
        
        up(mutex);
        up(south_full);
        up(car_sem);
        //we sleep bleh
        if(rand()%10 > 7) //70%
          sleep(10); //sleep for 10
      }
    }
  }
  
  
  
  
  //consumer flaggy person 

  if(fork()==0)
  {
    while(1)
    {
      //car number and direction
      int car_number;
      char car_direction;


      down(mutex);
     //down(car_sem);

      if(car_sem->num ==0)
      {
        printf("The flagperson is asleep.");
        
        //exit(1);
      }
      up(mutex);
      //sleep
      //down(mutex);
      down(car_sem);
      //sleep(1);

      //never be in the consumer without a car in a queue
      if(car_sem->num <=0)
      {
        exit(1);
      }
      //switch directions
      if(*direction == 'N' && south_full->num >= 12) //total 12
        *direction = 'S';
      else if(*direction == 'S' && north_full->num >= 12)
        *direction = 'N';

        //up(mutex);
      //sleep(1);

      //we need to "consume" cars from the N or the S
      if(*direction == 'N') //north check consume 
      {
        down(north_full);
        car_number = north_buffer_ptr[(*north_consumer_ptr)%buffer_size]; //keep track # of cars
        *north_consumer_ptr = *north_consumer_ptr + 1; //consumed 1 
        car_direction = 'N';
        
        //switch directions
        if(north_full->num == 0) *direction = 'S';
       
      }
      else if(*direction == 'S') //south check consume 
      {
        // *south_consumer_ptr = *south_consumer_ptr + 1; 
        down(south_full);
        car_number = south_buffer_ptr[(*south_consumer_ptr)%buffer_size];
        *south_consumer_ptr = *south_consumer_ptr + 1; //consumed 1 
        car_direction = 'S';
       
        //switch directions
        if(south_full->num == 0)
          *direction = 'N';
          //*honk = 'Y';
      }
      up(mutex);
      sleep(1);
      printf("Car %d coming from the %c direction left the construction zone at time %d.",car_number,car_direction,time(NULL)-*stime); 
      
      
      if(car_direction == 'N') //north
      up(north_empty);


      if(car_direction == 'S') //south
      up(south_empty);
    }
  }



  int waits; //wait! waiittttt
  wait(&waits);
  return 0;
}