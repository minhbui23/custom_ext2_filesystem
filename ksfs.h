#ifndef _SFS_H_
#define _SFS_H_

#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/mount.h>
#include <linux/errno.h>
#include <linux/kernel.h>

#define SFS_MAGIC 0x20231120

// Superblock operations
extern const struct super_operations sfs_super_ops;

// File system type structure
extern struct file_system_type sfs_fs_type;

// Mount function
struct dentry *sfs_mount(struct file_system_type *fs_type, int flags,
                             const char *dev_name, void *data);

// Fill superblock structure
int sfs_fill_super(struct super_block *sb, void *data, int silent);

#endif
