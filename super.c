#include "ksfs.h"
#include "sfs.h"
//#include "inode.c"
int sfs_fill_super(struct super_block *sb, void *data, int silent) {
    struct inode *root_inode;
    struct sfs_inode *root_sfs_inode;
    struct buffer_head *bh;
    struct sfs_superblock *sfs_sb;
    int ret = 0;

    bh = sb_bread(sb, SFS_SUPERBLOCK_BLOCK_NO);   
    BUG_ON(!bh);   
    sfs_sb = (struct sfs_superblock *)bh->b_data;

    //check filesystem validity based on MAGIC_NUM
    if (unlikely(sfs_sb->magic != SFS_MAGIC)) {
        printk(KERN_ERR
               "The filesystem being mounted is not of type sfs. "
               "Magic number mismatch: %llu != %llu\n",
               sfs_sb->magic, (uint64_t)SFS_MAGIC);
        goto release;
    }

    //check block size of sfs_sb
    if (unlikely(sb->s_blocksize != sfs_sb->blocksize)) {
        printk(KERN_ERR
               "sfs seem to be formatted with mismatching blocksize: %lu\n",
               sb->s_blocksize);
        goto release;
    }

    sb->s_magic = sfs_sb->magic;
    sb->s_fs_info = sfs_sb;
    sb->s_maxbytes = sfs_sb->blocksize;
    //sb->s_op = &sfs_sb_ops;

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

