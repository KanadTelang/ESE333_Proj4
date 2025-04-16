#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/netlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define NETLINK_USER 31

#define MAX_PAYLOAD 1024 /* maximum payload size*/
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;
char temp[1024], input[1024];
char char_inp; 

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
	// sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0)
        return -1;

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */

    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    //strcpy(NLMSG_DATA(nlh), "TEST DATA 1");

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    strcpy(NLMSG_DATA(nlh), "S");

   // printf("Sending message to kernel\n");
    sendmsg(sock_fd, &msg, 0);
   // printf("Waiting for message from kernel\n");

    /* Read message from kernel */
    while(1){
    recvmsg(sock_fd, &msg, 0);
    printf("Received message payload: %s\n", NLMSG_DATA(nlh));
    }
    //close(sock_fd);
}

void *producer(void *vargp)
{
	
    if (sock_fd < 0)
        return -1;

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */

    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;
 
    strcpy(NLMSG_DATA(nlh), "P"); 

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    // printf("Sending message to kernel\n");
    sendmsg(sock_fd, &msg, 0);

    while(1){
   printf("Enter the message : \t"); 
    fgets (temp, MAX_PAYLOAD, stdin); 
    // strcpy(input, "P"); 
    strcpy(input,temp); 
    strcpy(NLMSG_DATA(nlh), input); 
    printf("Sending message to kernel\n");
    sendmsg(sock_fd, &msg, 0);
    }
    
    printf("Waiting for message from kernel\n");

    /* Read message from kernel */
    recvmsg(sock_fd, &msg, 0);
    printf("Received message payload: %s\n", NLMSG_DATA(nlh));
    

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


    //pthread_mutex_init(&lock, NULL);
     sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
	pthread_create(&tid[0], NULL, &producer, NULL);
	pthread_create(&tid[1], NULL, &consumer, NULL);
	pthread_join(tid[1], NULL);
	pthread_join(tid[0], NULL);
    close(sock_fd);
   
}



/*
Useful commands:
pthread_mutex_init(&lock, NULL)
pthread_create(&tid[0], NULL, &producer, NULL);
pthread_join(&tid[1], NULL);
pthread_mutex_lock(&lock);
pthread_mutex_unlock(&lock);
*/
