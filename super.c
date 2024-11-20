#include "ksfs.h"

static int sfs_fill_super(struct super_block *sb, void *data, int silent) {
    struct inode *root_inode;
    struct sfs_inode *root_sfs_inode;
    struct buffer_head *bh;
    struct sfs_superblock *sfs_sb;
    int ret = 0;

    bh = sb_bread(sb, SFS_SUPERBLOCK_BLOCK_NO);
    BUG_ON(!bh);
    sfs_sb = (struct sfs_superblock *)bh->b_data;
    if (unlikely(sfs_sb->magic != SFS_MAGIC)) {
        printk(KERN_ERR
               "The filesystem being mounted is not of type sfs. "
               "Magic number mismatch: %llu != %llu\n",
               sfs_sb->magic, (uint64_t)SFSMAGIC);
        goto release;
    }
    if (unlikely(sb->s_blocksize != sfs_sb->blocksize)) {
        printk(KERN_ERR
               "sfs seem to be formatted with mismatching blocksize: %lu\n",
               sb->s_blocksize);
        goto release;
    }

    sb->s_magic = sfs_sb->magic;
    sb->s_fs_info = sfs_sb;
    sb->s_maxbytes = sfs_sb->blocksize;
    sb->s_op = &sfs_sb_ops;

    root_sfs_inode = sfs_get_sfs_inode(sb, SFS_ROOTDIR_INODE_NO);
    root_inode = new_inode(sb);
    if (!root_inode || !root_sfs_inode) {
        ret = -ENOMEM;
        goto release;
    }
    sfs_fill_inode(sb, root_inode, root_sfs_inode);
    inode_init_owner(root_inode, NULL, root_inode->i_mode);

    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root) {
        ret = -ENOMEM;
        goto release;
    }

release:
    brelse(bh);
    return ret;
}

struct dentry *sfs_mount(struct file_system_type *fs_type,
                             int flags, const char *dev_name,
                             void *data) {
    struct dentry *ret;
    ret = mount_bdev(fs_type, flags, dev_name, data, sfs_fill_super);

    if (unlikely(IS_ERR(ret))) {
        printk(KERN_ERR "Error mounting sfs.\n");
    } else {
        printk(KERN_INFO "sfs is succesfully mounted on: %s\n",
               dev_name);
    }

    return ret;
}

void sfs_kill_superblock(struct super_block *sb) {
    printk(KERN_INFO
           "sfs superblock is destroyed. Unmount succesful.\n");
    kill_block_super(sb);
}

void sfs_put_super(struct super_block *sb) {
    return;
}

void sfs_save_sb(struct super_block *sb) {
    struct buffer_head *bh;
    struct sfs_superblock *sfs_sb = sfs_SB(sb);

    bh = sb_bread(sb, SFS_SUPERBLOCK_BLOCK_NO);
    BUG_ON(!bh);

    bh->b_data = (char *)sfs_sb;
    mark_buffer_dirty(bh);
    sync_dirty_buffer(bh);
    brelse(bh);
}
