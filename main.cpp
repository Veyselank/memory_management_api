#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <stdlib.h> 
#include <queue> 
#include <semaphore.h>
#include <random>
using namespace std;

#define NUM_THREADS 10
#define MEMORY_SIZE 150

struct node
{
	int id;
	int size;
};

queue<node> myqueue; // shared que
pthread_mutex_t sharedLock = PTHREAD_MUTEX_INITIALIZER; // mutex
pthread_t server; // server thread handle
sem_t semlist[NUM_THREADS]; // thread semaphores

int thread_message[NUM_THREADS]; // thread memory information
char  memory[MEMORY_SIZE]; // memory size

int threadCounter = 0; // counting finished threads
int Temp = 0;
void my_malloc(int thread_id, int size)
{
	//This function will add the struct to the queue
	node * req = new node();
	req->id = thread_id;
	req->size = size;
	myqueue.push(*req);
}

void * server_function(void *)
{
	//This function should grant or decline a thread depending on memory size.
	while(true)
	{
		while(!myqueue.empty())
		{
			node* req = new node();
			req = &myqueue.front();
			myqueue.pop();
			if(MEMORY_SIZE - Temp < req->size)
				thread_message[req->id] = -1;
			else
				thread_message[req->id] = Temp;
			sem_post(&semlist[req->id]);
		}
		if(threadCounter == NUM_THREADS)
			break;
	}
	return 0;
}

void * thread_function(void * id) 
{
	int* ptr = (int*) id;
	int randSize = rand() % ( MEMORY_SIZE / 6); // creating random int
	//This function will create a random size, and call my_malloc
	my_malloc(*ptr,randSize);
	sem_wait(&semlist[*ptr]);
	//Block
	pthread_mutex_lock(&sharedLock); // block
	//Then fill the memory with id's or give an error prompt
	if(thread_message[*ptr] == -1)
		cout << "ThreadID: "<< *ptr << " Not enough memory"<<endl;
	else
	{
		for(int i = Temp; i< randSize + Temp; i++)
		{
			memory[i]= '1';
		}
		Temp += randSize;
	}
	pthread_mutex_unlock(&sharedLock); //unblock
	return 0;
}

void init()	 
{
	pthread_mutex_lock(&sharedLock);	//lock
	for(int i = 0; i < NUM_THREADS; i++) //initialize semaphores
	{sem_init(&semlist[i],0,0);}
	for (int i = 0; i < MEMORY_SIZE; i++)	//initialize memory 
  	{char zero = '0'; memory[i] = zero;}
   	pthread_create(&server,NULL,server_function,NULL); //start server 
	pthread_mutex_unlock(&sharedLock); //unlock
}



void dump_memory() 
{
 // You need to print the whole memory array here.
 for (int i = 0; i < MEMORY_SIZE; i++)
 {
	 cout << memory[i];
 }
}

int main (int argc, char *argv[])
 {
	srand(time(NULL));//random numbers each time
 	//You need to create a thread ID array here
	int threadID[NUM_THREADS];
	for(int i=0; i<NUM_THREADS;i++)
		threadID[i] = i;
	
 	init();	// call init
	pthread_t threads[NUM_THREADS];
 	//You need to create threads with using thread ID array, using pthread_create()
	 for(int i = 0; i <NUM_THREADS;i++)
	 {
		 pthread_create(&threads[i],NULL,thread_function,(void *) &threadID[i]);
	 }
	 
	
 	//You need to join the threads
	pthread_mutex_lock(&sharedLock); //Block
	threadCounter++; //increment the number of used threads
	pthread_mutex_unlock(&sharedLock); // unblock
	for(int i = 0;i < NUM_THREADS;i++)
		pthread_join(threads[i],NULL);
 	dump_memory(); // print the memory
 	
 	printf("\nMemory Indexes:\n" );
 	for (int i = 0; i < NUM_THREADS; i++)
 	{
 		printf("[%d]" ,thread_message[i]); // this will print out the memory indexes
 	}
 	printf("\nTerminating...\n");
	
 }