#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/skbuff.h>

int (*ra_sw_nat_hook_rx) (struct sk_buff * skb) = NULL;
int (*ra_sw_nat_hook_tx) (struct sk_buff * skb, int gmac_no) = NULL;
#if defined (CONFIG_RA_HW_NAT_WIFI_NEW_ARCH)
void (*ppe_dev_register_hook) (struct net_device *dev);
void (*ppe_dev_unregister_hook) (struct net_device *dev);
EXPORT_SYMBOL(ppe_dev_register_hook);
EXPORT_SYMBOL(ppe_dev_unregister_hook);
#endif
EXPORT_SYMBOL(ra_sw_nat_hook_rx);
EXPORT_SYMBOL(ra_sw_nat_hook_tx);

