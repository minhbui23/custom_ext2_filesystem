// #include "ksfs.h"

// void sfs_fill_inode(struct super_block *sb, struct inode *inode,
//                         struct sfs_inode *sfs_inode) {
//     inode->i_mode = sfs_inode->mode;
//     inode->i_sb = sb;
//     inode->i_ino = sfs_inode->inode_no;
//     //inode->i_op = &sfs_inode_ops;
//     // TODO hope we can use sfs_inode to store timespec
//     // inode->i_atime = inode->i_mtime 
//     //                = inode->i_ctime
//     //                = CURRENT_TIME;
//     inode->i_private = sfs_inode;    
    
// }

// struct sfs_inode *sfs_get_sfs_inode(struct super_block *sb,
//                                                 uint64_t inode_no) {
//     struct buffer_head *bh;
//     struct sfs_inode *inode;
//     struct sfs_inode *inode_buf;

//     bh = sb_bread(sb, SFS_INODE_TABLE_START_BLOCK_NO + SFS_INODE_BLOCK_OFFSET(sb, inode_no));
//     BUG_ON(!bh);
    
//     inode = (struct sfs_inode *)(bh->b_data + SFS_INODE_BYTE_OFFSET(sb, inode_no));
//     inode_buf = kmem_cache_alloc(sfs_inode_cache, GFP_KERNEL);
//     memcpy(inode_buf, inode, sizeof(*inode_buf));

//     brelse(bh);
//     return inode_buf;
// }