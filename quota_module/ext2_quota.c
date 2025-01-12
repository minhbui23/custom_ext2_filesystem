#include <linux/kernel.h>
#include <linux/hashtable.h>
#include <linux/kprobes.h>
#include <linux/fs.h>
#include <linux/uidgid.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/path.h>
#include <linux/file.h>
#include "ext2_quota.h"

#define QUOTA_FILE_PATH "/etc/ext2_quota/quotas"

void save_quota_to_file(void)
{
    struct file *file;
    struct ext2_quota_info *qinfo;
    int bkt;

    file = filp_open(QUOTA_FILE_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (IS_ERR(file)) {
        printk(KERN_ERR "Failed to open quota file for writing\n");
        return;
    }

    hash_for_each(quota_hash_table, bkt, qinfo, node) {
        char buffer[128];
        int len = snprintf(buffer, sizeof(buffer), "%u %lu %lu\n",
                           qinfo->uid, qinfo->limit, qinfo->usage);
        if (kernel_write(file, buffer, len, &file->f_pos) < 0) {
            printk(KERN_ERR "Failed to write quota to file\n");
        }
    }

    filp_close(file, NULL);
}


void load_quota_from_file(void)
{
    struct file *file;
    char buffer[128];
    loff_t pos = 0;

    file = filp_open(QUOTA_FILE_PATH, O_RDONLY, 0);
    if (IS_ERR(file)) {
        printk(KERN_WARNING "Quota file not found, starting with empty quota table\n");
        return;
    }

    while (kernel_read(file, buffer, sizeof(buffer) - 1, &pos) > 0) {
        uid_t uid;
        unsigned long limit, usage;

        buffer[sizeof(buffer) - 1] = '\0'; // Đảm bảo kết thúc chuỗi
        if (sscanf(buffer, "%u %lu %lu", &uid, &limit, &usage) == 3) {
            struct ext2_quota_info *qinfo = kmalloc(sizeof(*qinfo), GFP_KERNEL);
            if (!qinfo) {
                printk(KERN_ERR "Failed to allocate memory for quota info\n");
                continue;
            }

            qinfo->uid = uid;
            qinfo->limit = limit;
            qinfo->usage = usage;
            hash_add(quota_hash_table, &qinfo->node, uid);
        }
    }

    filp_close(file, NULL);
}

// Define kprobe
struct kprobe kp_write = {
    .symbol_name = "ext2_file_write_iter",
};

struct kprobe kp_unlink = {
    .symbol_name = "ext2_unlink",
};

void add_quota(uid_t uid, unsigned long limit, unsigned long usage)
{
    struct ext2_quota_info *qinfo;
    qinfo = kmalloc(sizeof(*qinfo), GFP_KERNEL);
    if (!qinfo)
        return;

    qinfo->uid = uid;
    qinfo->limit = limit;
    qinfo->usage = usage;
    hash_add(quota_hash_table, &qinfo->node, uid);
}


bool check_quota(uid_t uid, unsigned long size)
{
    struct ext2_quota_info *qinfo;
    hash_for_each_possible(quota_hash_table, qinfo, node, uid)
    {
        if (qinfo->uid == uid && (qinfo->usage + size > qinfo->limit))
            return false;
    }
    return true;
}

void update_quota(uid_t uid, unsigned long size)
{
    struct ext2_quota_info *qinfo;
    hash_for_each_possible(quota_hash_table, qinfo, node, uid)
    {
        if (qinfo->uid == uid)
        {
            qinfo->usage += size;
            return;
        }
    }
}

void cleanup_quota_table(void)
{
    struct ext2_quota_info *qinfo;
    struct hlist_node *tmp;
    int bkt;

    hash_for_each_safe(quota_hash_table, bkt, tmp, qinfo, node)
    {
        hash_del(&qinfo->node);
        kfree(qinfo);
    }
}

void show_quota_info(uid_t uid)
{
    struct ext2_quota_info *qinfo;
    hash_for_each_possible(quota_hash_table, qinfo, node, uid)
    {
        if (qinfo->uid == uid)
        {
            printk(KERN_INFO "Quota Info - UID: %u, Limit: %lu, Usage: %lu\n", qinfo->uid, qinfo->limit, qinfo->usage);
        }
    }
}

//Define handler 
int write_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
    struct iov_iter *from = (struct iov_iter *)regs->si; 
    uid_t uid = current_fsuid().val;                     
    size_t size = iov_iter_count(from);                  

    printk(KERN_INFO "Pre-handler: UID=%u, size=%zu\n", uid, size);


    if (!check_quota(uid, size))
    {
        printk(KERN_WARNING "Quota exceeded for UID %u. Requested size: %zu bytes\n", uid, size);

    }

    return 0; 
}


void unlink_post_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{

    struct dentry *dentry = (struct dentry *)regs->si; 
    uid_t uid = current_fsuid().val;
    struct inode *inode = dentry->d_inode;

    if (inode)
    {
        unsigned long size = inode->i_size;
        update_quota(uid, -size); 
        printk(KERN_INFO "Unlink successful: UID=%u, dentry=%s, size=%lu\n", uid, dentry->d_name.name, size);
        show_quota_info(uid);
    }
    else
    {
        printk(KERN_WARNING "Unlink failed: inode is NULL\n");
    }
}

void write_post_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{

    struct iov_iter *from = (struct iov_iter *)regs->si; 
    uid_t uid = current_fsuid().val;                     
    size_t size = iov_iter_count(from);

    printk(KERN_DEBUG "Post-handler: Write operation successfully for UID=%u, size=%zd\n", uid, size);

    if (size > 0)
    {
        update_quota(uid, (unsigned long)size);
    }
    else if (size < 0)
    {
        printk(KERN_WARNING "Post-handler: Write operation failed with size=%zd\n", size);
    }

    show_quota_info(uid);
}


static int __init ext2_quota_init(void)
{
    int ret;

    // Load quota data from file
    load_quota_from_file();

    show_quota_info(1000);

    // Add handler for write
    kp_write.pre_handler = write_pre_handler;
    kp_write.post_handler = write_post_handler;
    kp_write.symbol_name = "ext2_file_write_iter";


    ret = register_kprobe(&kp_write);
    if (ret < 0)
    {
        printk(KERN_ERR "Failed to register kprobe: %d\n", ret);
        return ret;
    }

    // Add handler for unlink
    kp_unlink.post_handler = unlink_post_handler;
    kp_unlink.symbol_name = "ext2_unlink";

    ret = register_kprobe(&kp_unlink);
    if (ret < 0)
    {
        printk(KERN_ERR "Failed to register unlink kprobe: %d\n", ret);
        unregister_kprobe(&kp_write);
        return ret;
    }


    printk(KERN_INFO "ext2_quota module loaded successfully.\n");
    return 0;
}

static void __exit ext2_quota_exit(void)
{
    save_quota_to_file();
    unregister_kprobe(&kp_write);
    unregister_kprobe(&kp_unlink);
    cleanup_quota_table();
    printk(KERN_INFO "ext2_quota module unloaded successfully.\n");
}

module_init(ext2_quota_init);
module_exit(ext2_quota_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MinhBui");
MODULE_DESCRIPTION("Kprobe-based quota management for ext2");
