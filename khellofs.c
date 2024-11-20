#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/pagemap.h>
#include <linux/mount.h>
#include <linux/string.h>
#include "hellofs.h"

int hellofs_fill_super(struct super_block *sb, void *data, int silent);

// Superblock operations
const struct super_operations hellofs_super_ops = {
    .put_super = kill_block_super,  // Giải phóng superblock
};

// Hàm mount: Tạo superblock và gắn filesystem.
static struct dentry *hellofs_mount(struct file_system_type *fs_type, int flags,
                                    const char *dev_name, void *data)
{
    struct dentry *ret;

    ret = mount_bdev(fs_type, flags, dev_name, data, hellofs_fill_super);
    if (IS_ERR(ret)) {
        printk(KERN_ERR "hellofs: Error mounting filesystem\n");
    } else {
        printk(KERN_INFO "hellofs: Successfully mounted\n");
    }
    return ret;
}

// Hàm tạo superblock.
int hellofs_fill_super(struct super_block *sb, void *data, int silent)
{
    struct inode *root_inode;

    // Gán giá trị cơ bản cho superblock.
    sb->s_magic = HELLOFS_MAGIC;
    sb->s_op = &hellofs_super_ops;

    // Tạo inode gốc (root).
    root_inode = new_inode(sb);
    if (!root_inode) {
        printk(KERN_ERR "hellofs: Failed to allocate root inode\n");
        return -ENOMEM;
    }

    // Cấu hình inode gốc.
    root_inode->i_ino = 1; // Inode số 1 là root.
    root_inode->i_sb = sb;
    root_inode->i_op = &simple_dir_inode_operations; // Sử dụng hàm mặc định.
    root_inode->i_fop = &simple_dir_operations;      // Hàm xử lý thư mục mặc định.
    root_inode->i_mode = S_IFDIR | 0755;

    // Gán dentry gốc cho superblock.
    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root) {
        printk(KERN_ERR "hellofs: Failed to create root dentry\n");
        iput(root_inode);
        return -ENOMEM;
    }

    printk(KERN_INFO "hellofs: Superblock successfully created\n");
    return 0;
}

// Định nghĩa file system type cho hellofs.
struct file_system_type hellofs_fs_type = {
    .owner = THIS_MODULE,
    .name = "hellofs",
    .mount = hellofs_mount,
    .kill_sb = kill_block_super, // Sử dụng hàm mặc định để hủy superblock.
    .fs_flags = FS_REQUIRES_DEV,
};

// Hàm khởi tạo module.
static int __init hellofs_init(void)
{
    int ret;

    ret = register_filesystem(&hellofs_fs_type);
    if (ret == 0) {
        printk(KERN_INFO "hellofs: Filesystem registered successfully\n");
    } else {
        printk(KERN_ERR "hellofs: Failed to register filesystem. Error: %d\n", ret);
    }

    return ret;
}

// Hàm hủy module.
static void __exit hellofs_exit(void)
{
    int ret;

    ret = unregister_filesystem(&hellofs_fs_type);
    if (ret == 0) {
        printk(KERN_INFO "hellofs: Filesystem unregistered successfully\n");
    } else {
        printk(KERN_ERR "hellofs: Failed to unregister filesystem. Error: %d\n", ret);
    }
}

module_init(hellofs_init);
module_exit(hellofs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("accelazh");
MODULE_DESCRIPTION("Minimal hellofs filesystem module");
