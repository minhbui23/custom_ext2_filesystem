#include "sfs.h"

int sfs_fill_super(struct super_block *sb, void *data, int silent);

// Superblock operations
const struct super_operations sfs_super_ops = {
    .put_super = kill_block_super,  
};


// Mount func: Init superblock and assign to filesystem 
static struct dentry *sfs_mount(struct file_system_type *fs_type, int flags,
                                    const char *dev_name, void *data)
{
    struct dentry *ret;

    ret = mount_bdev(fs_type, flags, dev_name, data, sfs_fill_super);
    if (IS_ERR(ret)) {
        printk(KERN_ERR "sfs: Error mounting filesystem\n");
    } else {
        printk(KERN_INFO "sfs: Successfully mounted\n");
    }
    return ret;
}

// Init superblock
int sfs_fill_super(struct super_block *sb, void *data, int silent)
{
    struct inode *root_inode;

    // Init superblock
    sb->s_magic = sfs_MAGIC;
    sb->s_op = &sfs_super_ops;

    // Create root inode
    root_inode = new_inode(sb);
    if (!root_inode) {
        printk(KERN_ERR "sfs: Failed to allocate root inode\n");
        return -ENOMEM;
    }

    // Config root inode
    root_inode->i_ino = 1; // Inode 1 is root
    root_inode->i_sb = sb;
    root_inode->i_op = &simple_dir_inode_operations; // Use default inode func
    root_inode->i_fop = &simple_dir_operations;      // Use default dir func
    root_inode->i_mode = S_IFDIR | 0755;

    // assign root dentry for superblock
    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root) {
        printk(KERN_ERR "sfs: Failed to create root dentry\n");
        iput(root_inode);
        return -ENOMEM;
    }

    printk(KERN_INFO "sfs: Superblock successfully created\n");
    return 0;
}


// Define filesystem type for sfs
struct file_system_type sfs_fs_type = {
    .owner = THIS_MODULE,
    .name = "sfs",
    .mount = sfs_mount,
    .kill_sb = kill_block_super, 
    .fs_flags = FS_REQUIRES_DEV,
};


static int __init sfs_init(void)
{
    int ret;

    ret = register_filesystem(&sfs_fs_type);
    if (ret == 0) {
        printk(KERN_INFO "sfs: Filesystem registered successfully\n");
    } else {
        printk(KERN_ERR "sfs: Failed to register filesystem. Error: %d\n", ret);
    }

    return ret;
}


static void __exit sfs_exit(void)
{
    int ret;

    ret = unregister_filesystem(&sfs_fs_type);
    if (ret == 0) {
        printk(KERN_INFO "sfs: Filesystem unregistered successfully\n");
    } else {
        printk(KERN_ERR "sfs: Failed to unregister filesystem. Error: %d\n", ret);
    }
}

module_init(sfs_init);
module_exit(sfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("minhbui23");
MODULE_DESCRIPTION("Minimal sfs filesystem module");
