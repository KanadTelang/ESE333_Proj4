#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

const int max = 10000000; //number of message to be sent
pthread_mutex_t lock;
int length = 0; //number of entries in the linked list
pthread_t tid[2];

struct node* head = NULL;
struct node* tail = NULL;

struct node
{
	struct node* next;
	int data;
};

// (head, 0) -> (1) -> (2) -> (3, tail)
//count(producer) = 3, count(consumer) = 0
// if consumer goes next: (head, 1) -> (2) -> (3, tail)
// if producer goes next: (head, 0) -> (1) -> (2) -> (3) -> (4, tail)
void *consumer(void *vargp)
{
	int count = 0;
	while(count < max)
	{
	// 3) grab the mutex (to avoid critical region stuff), decrement the length, remove the lowest value from the linked list (head), increment count
	//consume messages (data from 0 to max-1, throw error if data out of order), invoke free on the head
		pthread_mutex_lock(&lock);
		struct node* temp; 
		if(head != NULL){
			if(head->data != count){
				printf("ERROR! data %d should be %d!\n", head->data, count);
			}
			else{
				temp = head; 
				if (head == tail){
					head = NULL; 
					tail = NULL; 
				}
				else{
					head = head -> next;
				}
				free(temp);
				length --; 
				printf("%d count consumed by the consumer.\n", count++);
			}
		}
		pthread_mutex_unlock(&lock); 
	}
	//return NULL;
}

void *producer(void *vargp)
{
	int count = 0;
	while(count < max)
	{
		pthread_mutex_lock(&lock);
		struct node* newnode= malloc (sizeof (struct node)); 
		newnode -> data = count; 
		newnode -> next = NULL; 
		if (head == NULL){
			head = newnode;  
		}
		else {
			tail -> next = newnode; 
		}
		tail = newnode; 
		length ++; 
		printf("%d count added by the producer.\n", count++); 
		pthread_mutex_unlock(&lock);
		
		

	// 2) grab the mutex (to avoid critical region stuff), increment the length, put the value of count on the linked list, increment count
	//produce messages (data from 0 to max-1), malloc new tails
	}
	//return NULL;
}
void makeList(){
	for (int i= 0; i < max; i++){
		struct node* newnode= malloc (sizeof (struct node)); 
		newnode -> data = i; 
		newnode -> next = NULL; 
		if (head == NULL){
			head = newnode;  
		}
		else {
			tail -> next = newnode; 
		}
		tail = newnode; 
	}
}

int main()
{
	// 1) create the list
	//makeList(); 
	head = NULL; 
	tail = NULL;

	pthread_mutex_init(&lock, NULL);
	pthread_create(&tid[0], NULL, &producer, NULL);
	pthread_create(&tid[1], NULL, &consumer, NULL);
	pthread_join(tid[1], NULL);
	pthread_join(tid[0], NULL);
	if(head != NULL) {printf("ERROR! List not empty\n");}
	exit(0);
}



/*
Useful commands:
pthread_mutex_init(&lock, NULL)
pthread_create(&tid[0], NULL, &producer, NULL);
pthread_join(&tid[1], NULL);
pthread_mutex_lock(&lock);
pthread_mutex_unlock(&lock);
*/
