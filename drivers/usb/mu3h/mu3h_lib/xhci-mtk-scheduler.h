#ifndef _XHCI_MTK_SCHEDULER_H
#define _XHCI_MTK_SCHEDULER_H

#include "../mu3h_drv/xhci.h"

#define MTK_SCH_NEW	1

#define SCH_SUCCESS 1
#define SCH_FAIL	0

#define MAX_PORT_NUM        7
#define MAX_EP_NUM			64
#define SS_BW_BOUND			51000
//#define HS_BW_BOUND			6144
#define HS_BW_BOUND			6145	// for HS HB transfer test. (Test Plan SOP 3.4.1 and 3.4.2)	

#define USB_EP_CONTROL	0
#define USB_EP_ISOC		1
#define USB_EP_BULK		2
#define USB_EP_INT		3

#define USB_SPEED_LOW	1
#define USB_SPEED_FULL	2
#define USB_SPEED_HIGH	3
#define USB_SPEED_SUPER	5

/* mtk scheduler bitmasks */
#define BPKTS(p)	((p) & 0x3f)
#define BCSCOUNT(p)	(((p) & 0x7) << 8)
#define BBM(p)		((p) << 11)
#define BOFFSET(p)	((p) & 0x3fff)
#define BREPEAT(p)	(((p) & 0x7fff) << 16)


#if 1
typedef unsigned int mtk_u32;
typedef unsigned long long mtk_u64;
#endif

#define NULL ((void *)0)

struct mtk_xhci_ep_ctx {
	mtk_u32	ep_info;
	mtk_u32	ep_info2;
	mtk_u64	deq;
	mtk_u32	tx_info;
	/* offset 0x14 - 0x1f reserved for HC internal use */
	mtk_u32	reserved[3];
};


struct sch_ep
{
	//root hub port number
	int rh_port_num;
	//device info
	int dev_speed;
	int isTT;
	//ep info
	int is_in;
	int ep_type;
	int maxp;
	int interval;
	int burst;
	int mult;
	//scheduling info
	int offset;
	int repeat;
	int pkts;
	int cs_count;
	int burst_mode;
	//other
	int bw_cost;	//bandwidth cost in each repeat; including overhead
	mtk_u32 *ep;		//address of usb_endpoint pointer
	mtk_u32 *ep_ctx;
};

struct sch_port {
	struct sch_ep *ss_out_eps[MAX_EP_NUM];
	struct sch_ep *ss_in_eps[MAX_EP_NUM];
	struct sch_ep *hs_eps[MAX_EP_NUM];	//including tt isoc
	struct sch_ep *tt_intr_eps[MAX_EP_NUM];
};

int mtk_xhci_scheduler_init(struct device *dev);
int mtk_xhci_scheduler_add_ep(struct usb_hcd *hcd, struct usb_device *udev, struct usb_host_endpoint *ep);
int mtk_xhci_scheduler_remove_ep(struct usb_hcd *hcd, struct usb_device *udev, struct usb_host_endpoint *ep);

#endif
