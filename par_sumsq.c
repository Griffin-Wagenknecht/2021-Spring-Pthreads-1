#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// aggregate variables
long sum = 0;
long odd = 0;
long min = INT_MAX;
long max = INT_MIN;
char volatile action;
long volatile num;
volatile bool done = false;
pthread_mutex_t mutex;
pthread_mutex_t mutexList;
pthread_cond_t waitCond = PTHREAD_COND_INITIALIZER;



// function prototypes
void calculate_square(long number);

/*
 * update global aggregate variables given a number
 */
void calculate_square(long number)
{

  // calculate the square
  long the_square = number * number;

  // ok that was not so hard, but let's pretend it was
  // simulate how hard it is to square this number!
  sleep(number);

  // let's add this to our (global) sum
  pthread_mutex_lock(&mutex);
  sum += the_square;

  // now we also tabulate some (meaningless) statistics
  if (number % 2 == 1) {
    // how many of our numbers were odd?
    odd++;
  }

  // what was the smallest one we had to deal with?
  if (number < min) {
    min = number;
  }

  // and what was the biggest one?
  if (number > max) {
    max = number;
  }
  pthread_mutex_unlock(&mutex);
}




typedef struct Task
{
    long t;                 //time 
    struct Task* next;      //pointer to next task 
} Task;

// Queue that holds a task
typedef struct TaskQueue
{
    struct Task* head;
} TaskQueue;

//ads a new task
bool addTask(TaskQueue* queue, long t)
{
    Task* temp = (Task*)malloc(sizeof(struct Task));
    temp->t = t;
    temp->next = NULL;

    if(queue->head == NULL)
    {
         queue->head = temp;
    }
    else
    {
        Task* curr = queue->head;
        while(curr->next != NULL)
        {
            curr = curr->next;
        }
        curr->next = temp;
    }
    return 1;
}

//pops the top of the queue
long popTask(TaskQueue* queue)
{
    Task* curr = queue->head;
    if(curr != NULL)
    {
        queue->head = curr->next;
    }
    else
    {
        return -1;
    }

    long headVal = curr->t;
    free(curr);
    
    return headVal;
}
//deletes queue
void delQueue(TaskQueue* queue)
{
    if(queue == NULL) return;
    while(queue->head != NULL) popTask(queue);
    free(queue);
}

//new queue
TaskQueue* newQueue()
{
    TaskQueue *temp = (TaskQueue*)malloc(sizeof(struct TaskQueue));
    temp->head = NULL;

    return temp;
}

void* routine(void* arg)
{
	TaskQueue* task = (TaskQueue*) arg;
	while(!done)
	{
    		pthread_mutex_lock(&mutexList);  		
    		while(task->head == NULL && !done)
    		{
    			pthread_cond_wait(&waitCond, &mutexList);	
    		}
    		long newTask = popTask(task);
    		pthread_mutex_unlock(&mutexList);
    		if(newTask != -1)
    		{
    			calculate_square(newTask);	
    		}	
    	
    	}

	pthread_mutex_unlock(&mutexList);
	pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    char *fn = argv[1];
    char *v = argv[2];
    int n = atoi(v);	
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexList,NULL);
    volatile TaskQueue* task = newQueue();
    
    
  // check and parse command line options
  if (argc != 3) {
    printf("Usage: sumsq <infile>\n");
    exit(EXIT_FAILURE);
  }

  
  // load numbers and add them to the queue
  FILE* fin = fopen(fn, "r");
  pthread_t th[n];
  
  
  for (int i = 0; i < n; i++)
        {
            if (pthread_create(&th[i], NULL, &routine, (void*)task) != 0) 
                {
                    perror("Failed to create thread");
                    return 1;
                }
        }   
  


  
   while (fscanf(fin, "%c %ld\n", &action, &num) == 2) 
   {
      if (action == 'p')          // process, do some work
      {
      	pthread_mutex_lock(&mutexList);
      	addTask(task,num);
      	pthread_cond_signal(&waitCond);
      	pthread_mutex_unlock(&mutexList);
      	//calculate_square(num);
      }
     else if (action == 'w')     // wait, nothing new happening
     {
     sleep(num);
     }
     else 
     {
      printf("ERROR: Unrecognized action: '%c'\n", action);
      exit(EXIT_FAILURE);
     }
  }
     
  fclose(fin);
  while(task->head != NULL) {}
  done = true;
  pthread_mutex_lock(&mutexList);
  pthread_cond_broadcast(&waitCond);
  pthread_mutex_unlock(&mutexList);
  
      for (int i = 0; i < n; i++) 
    {
        if (pthread_join(th[i], NULL) != 0) 
         {
             return 2;
         }
    }
  
  // print results
  printf("%ld %ld %ld %ld\n", sum, odd, min, max);
  // clean up and return
  pthread_mutex_destroy(&mutex);
  pthread_mutex_destroy(&mutexList);
  return (EXIT_SUCCESS);
}



