#include <linux/module.h>
#include <linux/kernel.h>
#include <net/sock.h> 
#include <linux/netlink.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/skbuff.h> 
#include <linux/pid.h>
#include <linux/mm.h>

#define NETLINK_USER 31

struct sock *nl_sk = NULL;
int pid;

typedef struct KllNode {
	int pid; 
	struct list_head points;
}KllNode; 

KllNode sub = {
	0, 
	LIST_HEAD_INIT(sub.points)
}; 

KllNode pub = {
	0,
	LIST_HEAD_INIT(pub.points)
}; 


static void hello_nl_recv_msg(struct sk_buff *skb)
{

    struct nlmsghdr *nlh;
	struct sk_buff *skb_out;
	int msg_size;
	char *msg ;
	int res;

	printk(KERN_INFO "Entering: %s\n", __FUNCTION__);


    	nlh = (struct nlmsghdr *)skb->data;
    	printk(KERN_INFO "Netlink received msg payload:%s\n", (char *)nlmsg_data(nlh));
		msg =nlmsg_data(nlh);
    	msg_size = strlen(msg);

	if (msg[0]=='P'){
		struct KllNode* newnode= vmalloc(sizeof(struct KllNode)); 
		newnode -> pid = nlh -> nlmsg_pid; 
		INIT_LIST_HEAD(& newnode -> points); 
		list_add_tail(& newnode -> points, &pub.points); 

	}

	else if (msg[0] == 'S'){

		struct KllNode* newnode= vmalloc(sizeof(struct KllNode)); 
		newnode -> pid = nlh -> nlmsg_pid;
		INIT_LIST_HEAD(& newnode -> points); 
		list_add_tail(& newnode -> points, &sub.points);
	}

	struct list_head *it, *temp;
	list_for_each_safe(it, temp, &pub.points) {
		struct KllNode* pubnode= list_entry(it, struct KllNode, points); 
		if (!pid_task(find_get_pid(pubnode->pid), PIDTYPE_PID)){
				printk(KERN_ERR "PID not active"); 
				list_del(it); 
				vfree(pubnode);
		}
		else if(pubnode-> pid == nlh-> nlmsg_pid){
			printk("Here 1 kpt\n"); 
			struct list_head *it2; 
			list_for_each_safe(it2, temp, &sub.points) {
				printk("Here 2 \n"); 
				struct KllNode* subnode = list_entry(it2, struct KllNode, points); 
				if(subnode->pid != pubnode ->pid) {
					printk(KERN_INFO"Message: %s has been sent from ID:%d to ID: %d", msg, pubnode -> pid, subnode -> pid); 
					//struct sk_buff *skb_out;
					skb_out = nlmsg_new(msg_size, 0);
					if (!skb_out) {
						printk(KERN_ERR "Failed to allocate new skb\n");
						return;
					}
					nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
					NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
					strncpy(nlmsg_data(nlh), msg, msg_size);
					if (nlmsg_unicast(nl_sk, skb_out, subnode->pid) < 0){
						printk(KERN_ERR "Failed to Broadcast message to %d pid.\n", subnode ->pid); 
						list_del(it); 
						vfree(subnode); 
					}
				} 
			}
			return;
		}
	}
}




static int __init hello_init(void)
{

    printk("Entering: %s\n", __FUNCTION__);
    struct netlink_kernel_cfg cfg = {
        .input = hello_nl_recv_msg,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    return 0;
}

static void __exit hello_exit(void)
{

    printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(nl_sk);
}

module_init(hello_init); module_exit(hello_exit);

MODULE_LICENSE("GPL");
