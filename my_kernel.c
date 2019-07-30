#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <net/sock.h>
#include <net/netlink.h>

#define NETLINK_TEST 21

struct sock *nl_sk = NULL;
EXPORT_SYMBOL_GPL(nl_sk);
u32 pid;

void send_to_user(char *str, int str_len)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	int rc;
  	int len = NLMSG_SPACE(1200);
    	skb = alloc_skb(len, GFP_ATOMIC);
    	if (!skb) {
      		printk(KERN_ERR "net_link: allocate failed.\n");
      		return;
    	}
    	nlh = nlmsg_put(skb,0,0,0,1200,0);
    	NETLINK_CB(skb).portid = 0; 

    	memcpy(NLMSG_DATA(nlh), str, str_len);
    	printk("net_link: going to send.\n");
    	rc = netlink_unicast(nl_sk, skb, pid, MSG_DONTWAIT);
    	if (rc < 0) {
      		printk(KERN_ERR "net_link: can not unicast skb (%d)\n", rc);
    	}
}
void nl_data_ready (struct sk_buff *__skb)
{
  	struct sk_buff *skb;
  	struct nlmsghdr *nlh;
  	char str[100];

  	printk("net_link: data is ready to read.\n");
  	skb = skb_get(__skb);

  	if (skb->len >= NLMSG_SPACE(0)) {
    		nlh = nlmsg_hdr(skb);
    		printk("net_link: recv %s.\n", (char *)NLMSG_DATA(nlh));
    		memcpy(str,NLMSG_DATA(nlh), sizeof(str)); 
    		pid = nlh->nlmsg_pid; 
    		printk("net_link: pid is %d\n", pid);
    		kfree_skb(skb);
		send_to_user("shenmeazhshiu", 15);
    		printk("net_link: send is ok.\n");
  	}
  	return;
}

static int test_netlink(void) {

	struct netlink_kernel_cfg cfg = {
    		.input = nl_data_ready,
  	};
  	nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);

  	if (!nl_sk) {
    		printk(KERN_ERR "net_link: Cannot create netlink socket.\n");
    		return -EIO;
  	}
  	printk("net_link: create socket ok.\n");
	return 0;
}

static int __init init_kernel(void)
{
  	test_netlink();
  	return 0;
}
static void __exit cleanup_kernel(void)
{
  	if (nl_sk != NULL) {
    		sock_release(nl_sk->sk_socket);
  	}
  	printk("net_link: remove ok.\n");
}
MODULE_LICENSE("GPL");
module_init(init_kernel);
module_exit(cleanup_kernel);
