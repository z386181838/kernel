#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include "partition_define.h"

#define EMMC_PARTITION_INFO		"emmc_partition"
#define EMMC_DBG(x, ...) do { if (emmc_debug_level) printk("\n%s %d: " x, __FILE__, __LINE__, ## __VA_ARGS__); } while(0)
#define EMMC_ERROR(x, ...) do { printk("%s %d: ERROR! " x, __FILE__, __LINE__, ## __VA_ARGS__); } while(0)

static struct proc_dir_entry *emmc_partition_info_proc;
extern const struct excel_info PartInfo_Private[PART_NUM];
int emmc_debug_level = 0;

static int partition_index(char *name)
{
	int idx;

	for (idx=0; idx<PART_NUM; idx++)
		if (!strcasecmp(name, PartInfo_Private[idx].name))
			break;

	return idx;
}

int ra_mtd_write_nm(char *name, loff_t to, size_t len, const u_char *buf)
{
	struct file *fd;
	int retLen = -1, idx;
	mm_segment_t old_fs = get_fs();
	loff_t pos;

	set_fs(KERNEL_DS);
	EMMC_DBG("%s(%d)- name:%s val:%s \n",__func__,__LINE__,name,buf);
	fd = filp_open("/dev/mmcblk0", O_WRONLY, 0);
	if(IS_ERR(fd)) {
		EMMC_ERROR("failed to open!!\n");
		return -1;
	}
	idx = partition_index(name);
	EMMC_DBG("%s(%d)- partition idx: %d\n",__func__,__LINE__,idx);
	if (idx == PART_NUM) {
		EMMC_ERROR("not found %s partition!\n", name);
		return -1;
	}
	pos = PartInfo_Private[idx].start_address;
	pos += to;
	EMMC_DBG("%s(%d): pos=%llx\n",__func__,__LINE__, pos);
	if ((fd->f_op == NULL) || (fd->f_op->write == NULL)) {
		EMMC_ERROR("file can not be write!!\n");
		return -1;
	}
	if (fd->f_pos != pos) {
		if (fd->f_op->llseek) {
			if(fd->f_op->llseek(fd, pos, 0) != pos) {
				EMMC_ERROR("failed to seek!!\n");
				return -1;
			}
		} else {
			fd->f_pos = pos;
		}
	}
	retLen = fd->f_op->write(fd, buf, len, &fd->f_pos);
	EMMC_DBG("%s(%d): return=%d\n",__func__,__LINE__, retLen);
	set_fs(old_fs);
	filp_close(fd, NULL);
	set_fs(old_fs);

	return retLen;
}
EXPORT_SYMBOL(ra_mtd_write_nm);

int ra_mtd_read_nm(char *name, loff_t from, size_t len, u_char *buf)
{
	struct file *fd;
	int retLen = -1, idx;
	mm_segment_t old_fs = get_fs();
	loff_t pos;

	set_fs(KERNEL_DS);

	EMMC_DBG("%s(%d)- name:%s \n",__func__,__LINE__,name);
	fd = filp_open("/dev/mmcblk0", O_WRONLY, 0);
	if(IS_ERR(fd)) {
		EMMC_ERROR("failed to open!!\n");
		return -1;
	}
	idx = partition_index(name);
	EMMC_DBG("%s(%d)- partition idx: %d\n",__func__,__LINE__,idx);
	if (idx == PART_NUM) {
		EMMC_ERROR("not found %s partition!\n", name);
		return -1;
	}
	pos = PartInfo_Private[idx].start_address;
	pos += from;
	EMMC_DBG("%s(%d): pos=%llx\n",__func__,__LINE__, pos);
	if ((fd->f_op == NULL) || (fd->f_op->read == NULL))
	{
		EMMC_ERROR("file can not be read!!\n");
		return -1;
	}
	if (fd->f_pos != pos) {
		if (fd->f_op->llseek) {
			if(fd->f_op->llseek(fd, pos, 0) != pos) {
				EMMC_ERROR("failed to seek!!\n");
				return -1;
			}
		} else {
			fd->f_pos = pos;
		}
	}
	retLen = fd->f_op->read(fd, buf, len, &fd->f_pos);
	EMMC_DBG("%s(%d): return=%d\n",__func__,__LINE__, retLen);
	filp_close(fd, NULL);
	set_fs(old_fs);

	return retLen;
}
EXPORT_SYMBOL(ra_mtd_read_nm);

static int dump_partition(struct seq_file *seq, void *v)
{
	int i;
	seq_puts(seq, "Part_Name\tSize\t\t\tStartAddr\t\tType\tMapTo\n");

	for (i = 1; i < PART_NUM; i++) {
		seq_printf(seq, "%-10s", PartInfo_Private[i].name);
		seq_printf(seq, "\t");
		seq_printf(seq, "0x%016llx", PartInfo_Private[i].size);
		seq_printf(seq, "\t");
		seq_printf(seq, "0x%016llx", PartInfo_Private[i].start_address);
		seq_printf(seq, "\t");
		seq_printf(seq, "%x", (int)PartInfo_Private[i].type);
		seq_printf(seq, "\t");
		if (PartInfo_Private[i].partition_idx == 0)
			seq_printf(seq, "/dev/mmcblk%d", PartInfo_Private[i].partition_idx);
		else
			seq_printf(seq, "/dev/mmcblk0p%d", PartInfo_Private[i].partition_idx);
		seq_printf(seq, "\n");
	}
	return 0;
}

static int partition_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, dump_partition, NULL);
}

static struct file_operations partition_info_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= partition_info_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.release 	= single_release
};

int mtk_emmc_init(void)
{
	emmc_partition_info_proc = 
		proc_create(EMMC_PARTITION_INFO, S_IFREG | S_IRUGO, NULL, 
				&partition_info_fops);
	if (!emmc_partition_info_proc)
		EMMC_ERROR("!! FAIL to create %s PROC !!\n", EMMC_PARTITION_INFO);

	return 0;
}

void mtk_emmc_exit(void)
{
	if (emmc_partition_info_proc)
		remove_proc_entry(EMMC_PARTITION_INFO, NULL);
}

module_init(mtk_emmc_init);
module_exit(mtk_emmc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Red Hung");
MODULE_DESCRIPTION("MTK eMMC Access Module");
