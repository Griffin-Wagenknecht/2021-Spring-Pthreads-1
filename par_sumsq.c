#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

//git add -A
//git commit -m "message"
//git push

// aggregate variables
long sum = 0;
long odd = 0;
long min = INT_MAX;
long max = INT_MIN;
bool done = false;
char volatile action;
long volatile num;
pthread_mutex_t mutex;
# define SIZE 10000
char holder;
volatile bool busy[SIZE];


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
}




//Linked List
struct  node 
{
   int volatile data;
   int volatile key;
   volatile struct node *next;
};

volatile struct  node *head = NULL;
volatile struct node *current = NULL;

int flag=0;
//insert link at the first location
void insertFirst(int key, int data) {
   //create a link
   volatile struct node *link = (struct node*) malloc(sizeof(struct node));
	
   link->key = key;
   link->data = data;
	
   //point it to old first node
   link->next = head;
	
   //point first to new first node
   head = link;
}

   volatile struct node  *tempLink;
//delete first item
struct node* deleteFirst() {

   //save reference to first link
   tempLink=head;
	
   //mark next to first link as first 
   head = head->next;
	
   //return the deleted link
   return tempLink;
}

int reverseCheck = 0;
void reverse(struct node** head_ref) {
   struct node* prev   = NULL;
   struct node* current = *head_ref;
   struct node* next;
	
   while (current != NULL) {
      next  = current->next;
      current->next = prev;   
      prev = current;
      current = next;
   }
	
   *head_ref = prev;
}



//is list empty
bool isEmpty() {
   return head == NULL;
}

void* routine()
{

    while(1)
    {
    	if(isEmpty())
    	{
    		break;
    	}
    	pthread_mutex_lock(&mutex);  		
    	volatile struct node *x = head;
    	if(reverseCheck == 0)
    	{
    		reverse(&x);
    		reverseCheck == 1;
    	}
    	//busy[pthread_self()]=true;
    	if(x->key == 'p')
    	{
    		pthread_mutex_unlock(&mutex);
    		calculate_square(x->data);
    		pthread_mutex_lock(&mutex);
    		deleteFirst();  
    		//busy[pthread_self()]=false;
    		pthread_mutex_unlock(&mutex);		
    	}
    	if(x->key == 'w')
    	{
    		pthread_mutex_unlock(&mutex);
    		sleep(x->data);
    		pthread_mutex_lock(&mutex);
    		deleteFirst();  
    		//busy[pthread_self()]=false;
    		pthread_mutex_unlock(&mutex);		
    	}	
    	
    }
    return 0;
}

// get a linked list volatile keyword for variables thata re being accessed
//convert routine to be an infite loop
// bool volatile busy
// set it to busy after the lock
//if list empty then break in while
// do calculations in parallel
//only have lock during read/write
int main(int argc, char* argv[])
{
    printf("Start \n");
    char *fn = argv[1];
    char *v = argv[2];
    int n = atoi(v);	
    pthread_mutex_init(&mutex, NULL);
    
    
  // check and parse command line options
  if (argc != 3) {
    printf("Usage: sumsq <infile>\n");
    exit(EXIT_FAILURE);
  }

  
  // load numbers and add them to the queue
  FILE* fin = fopen(fn, "r");
  pthread_t th[n];
  
  
 

  
   while (fscanf(fin, "%c %ld\n", &action, &num) == 2) 
   {
      if (action == 'p')          // process, do some work
      {
      	insertFirst(action,num);
      	//calculate_square(num);
      }
     else if (action == 'w')     // wait, nothing new happening
     {
     insertFirst(action,num);
     //sleep(num);
     }
     else 
     {
      printf("ERROR: Unrecognized action: '%c'\n", action);
      exit(EXIT_FAILURE);
     }
  }
	
	printf("File has been scanned \n");
     for (int i = 0; i < n; i++)
        {
            if (pthread_create(th + i, NULL, &routine, NULL) != 0) 
                {
                    perror("Failed to create thread");
                    return 1;
                }
        }    


  
  fclose(fin);
  
      for (int i = 0; i < n; i++) 
    {
        if (pthread_join(th[i], NULL) != 0) 
         {
             return 2;
         }
    }
    printf("Threads have been joined \n");
  
  // print results
  printf("%ld %ld %ld %ld\n", sum, odd, min, max);
  // clean up and return
  pthread_mutex_destroy(&mutex);
  printf("End\n");

  return (EXIT_SUCCESS);
}




