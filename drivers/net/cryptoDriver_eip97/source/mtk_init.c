
#include "basic_defs.h"
#include "device_rw.h"
#include "mtk_AdapterInternal.h"
//#include "mtk_hwDmaAccess.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/rt2880/surfboardint.h>
#include "api_pec.h"
#include <net/mtk_esp.h>
#include <linux/proc_fs.h>
	
static struct proc_dir_entry *mcrypto_entry;

extern void mtk_ipsec_init(void);
extern void mtk_ipsec_release(void);
extern unsigned int g_global_bus_burst_size;
void VDriver_Exit(void);


/*
 * proc write procedure
 */
static ssize_t mcrypto_proc_write(struct file *file, const char __user *buffer, 
			    size_t count, loff_t *data)
{
	char buf[256];
	char cmd;
	char* pch[5];
	char* ptr;
	int i, cpu, pt;
	memset(buf, 0, 256);
	memset(pch, 0, 5*4);
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	ptr = buf;
	pch[i] = strsep(&ptr," ");
	cmd = (char)*(pch[0]+0);
	while (pch[i] != NULL)
	{
		//printk ("[%d]%s\n",i,pch[i]);
		i++;
		pch[i] = strsep (&ptr, " ");
		if (i >= 5)
		  break;
	}

	switch (cmd)
	{
		case 'i' :
			strict_strtol(pch[1], 10, &pt);
			strict_strtol(pch[2], 10, &cpu);
			mcrypto_proc.ipicpu[pt] = cpu;
			if (cpu >=0)
				printk("change IPI point%d to CPU%d\n", pt,cpu);
			else
				printk("turn off IPI point%d\n",pt);	
			break;
		default :
			break;	
	}	
	return count;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,14)
static int mcrypto_proc_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len, i;
    if (off > 0)
    {
        return 0;
    }
    for (i = 0; i < 10; i++)
		len = sprintf(buf, "ipicpu[%d] : %d\n", i,mcrypto_proc.ipicpu[i]);
    len = sprintf(buf, "expand : %d\n", mcrypto_proc.copy_expand_count);
    len += sprintf(buf + len, "nolinear packet : %d\n", mcrypto_proc.nolinear_count);
    len += sprintf(buf + len, "oom putpacket : %d\n", mcrypto_proc.oom_in_put);
    for (i = 0; i < 16; i++)
    	len += sprintf(buf + len, "skbq[%d] : %d\n", i, mcrypto_proc.qlen[i]);
    for (i = 0; i < 16; i++)
    	len += sprintf(buf + len, "dbgpt[%d] : %d\n", i, mcrypto_proc.dbg_pt[i]);	
    return len;
}
#else
#include <linux/seq_file.h>
static int mcrypto_proc_read(struct seq_file *seq, void *v)
{
    int len, i;
	ipsecEip93Adapter_t *currAdapterPtr;
		
	mcrypto_proc.chstatus = 0;
		
	for (i = 0; i < IPESC_EIP93_ADAPTERS; i++)
	{
		mcrypto_proc.qlen[i] = 0;
		mcrypto_proc.qlen[i+IPESC_EIP93_ADAPTERS] = 0;
		currAdapterPtr = ipsecEip93AdapterListOut[i];
		if (currAdapterPtr)
		{	
			if (currAdapterPtr->status==TBL_DEL)
				mcrypto_proc.chstatus|=(1<<i);
			mcrypto_proc.qlen[i] = 0x0000FFFF &(currAdapterPtr->packet_count);
			mcrypto_proc.qlen[i+IPESC_EIP93_ADAPTERS] = 0x0000FFFF & (currAdapterPtr->skbQueue.qlen);
		}
	}
	for (i = 0; i < IPESC_EIP93_ADAPTERS; i++)
	{
		currAdapterPtr = ipsecEip93AdapterListIn[i];
		if (currAdapterPtr)
		{
			if (currAdapterPtr->status==TBL_DEL)
				mcrypto_proc.chstatus|=(1<<(i+IPESC_EIP93_ADAPTERS));	
			mcrypto_proc.qlen[i] |= currAdapterPtr->packet_count<<16;
			mcrypto_proc.qlen[i+IPESC_EIP93_ADAPTERS] |= currAdapterPtr->skbQueue.qlen<<16;
		}
	}
	for (i = 0; i < 10; i++)
		seq_printf(seq, "ipicpu[%d] : %d\n", i,mcrypto_proc.ipicpu[i]);
    seq_printf(seq, "expand : %d\n", mcrypto_proc.copy_expand_count);
    seq_printf(seq, "nolinear packet : %d\n", mcrypto_proc.nolinear_count);
    seq_printf(seq, "oom putpacket : %d\n", mcrypto_proc.oom_in_put);
	seq_printf(seq, "ch status : %08X\n", mcrypto_proc.chstatus);

	for (i = 0; i < 16; i++)
		seq_printf(seq, "ch[%d] Out %d : In %d\n", i, (short)(mcrypto_proc.qlen[i]&0x0000FFFF),\
			   	(short)(mcrypto_proc.qlen[i]>>16));
	for (i = 16; i < 32; i++)
        seq_printf(seq, "ch[%d] qOut %d : qIn %d\n", i-16, mcrypto_proc.qlen[i]&0x0000FFFF, mcrypto_proc.qlen[i]>>16);
                	 
    for (i = 0; i < 16; i++)
    	seq_printf(seq, "dbgpt[%d] : %08X\n", i, mcrypto_proc.dbg_pt[i]);	
    return 0;
}

static int mcrypto_open(struct inode *inode, struct file *file)
{
    return single_open(file, mcrypto_proc_read, NULL);
}

static const struct file_operations mcrypto_fops = {
    .owner      = THIS_MODULE,
    .open       = mcrypto_open,
    .read       = seq_read,
    .write			= mcrypto_proc_write,
    .llseek     = seq_lseek,
    .release    = single_release
};
#endif

int
VDriver_Init(
	void
)
{
	int i;
	
	g_global_bus_burst_size = 5;
	
	PEC_InitBlock_t InitBlock = {0, 0};

	if (Driver97_Init())
	{
		printk("\n !Driver97_Init failed! \n");
		return -1;
	}
	
	for (i = 0; i < DDK_PEC_IF_ID; i++)
	if (PEC_Init(i, &InitBlock) != PEC_STATUS_OK)
	{
		Driver97_Exit();
		printk("\n !PEC is initialized failed! \n");
		return -1;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,14)
	mcrypto_entry = create_proc_entry(PROCNAME, 0666, NULL);
	if (mcrypto_entry == NULL)
	{
		printk("HW Crypto : unable to create /proc entry\n");
		VDriver_Exit();
		return -1;
	}
	mcrypto_entry->read_proc = mcrypto_proc_read;
	mcrypto_entry->write_proc = mcrypto_proc_write;
#else
	if (!(mcrypto_entry = proc_create(PROCNAME, 0, NULL, &mcrypto_fops)))
	{
		printk("HW Crypto : unable to create /proc entry\n");
		VDriver_Exit();
		return -1;
	}
#endif	
	memset(&mcrypto_proc, 0, sizeof(mcrypto_proc_type));
	for (i = 0 ; i < 10 ; i++)
  		mcrypto_proc.ipicpu[i] = -1;

	mtk_ipsec_init();
	
    return 0;   // success
}



void
VDriver_Exit(
	void
)
{
	int i;
	for (i = 0; i < DDK_PEC_IF_ID; i++)
		PEC_UnInit(i);
    Driver97_Exit();
	mtk_ipsec_release();
	remove_proc_entry(PROCNAME, NULL);	
}

MODULE_LICENSE("GPL");

module_init(VDriver_Init);
module_exit(VDriver_Exit);
