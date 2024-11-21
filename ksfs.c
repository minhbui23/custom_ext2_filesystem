#include "ksfs.h"
#include "super.c"
struct file_system_type sfs_fs_type = {
    .owner = THIS_MODULE,
    .name = "sfs",
    .mount = sfs_mount,
    .kill_sb = sfs_kill_superblock,
    .fs_flags = FS_REQUIRES_DEV,
};

static int __init sfs_init(void)
{
    int ret = register_filesystem(&sfs_fs_type);
    if (ret == 0) {
        printk(KERN_INFO "sfs: Filesystem registered successfully\n");
    } else {
        printk(KERN_ERR "sfs: Failed to register filesystem. Error: %d\n", ret);
    }
    return ret;
}

static void __exit sfs_exit(void)
{
    int ret = unregister_filesystem(&sfs_fs_type);;
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
