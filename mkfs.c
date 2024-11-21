#include "sfs.h"

int mkfs_sfs(const char *device_path) {
    int fd;
    ssize_t ret;
    uint64_t welcome_inode_no;
    uint64_t welcome_data_block_no_offset;

    // Mở thiết bị
    fd = open(device_path, O_RDWR);
    if (fd == -1) {
        perror("Error while open device");
        return -1;
    }

    // create sfs_superblock
    struct sfs_superblock sfs_sb = {
        .version = 1,
        .magic = SFS_MAGIC,
        .blocksize = SFS_DEFAULT_BLOCKSIZE,
        .inode_table_size = SFS_DEFAULT_INODE_TABLE_SIZE,
        .inode_count = 1, //  inode: root 
        .data_block_table_size = SFS_DEFAULT_DATA_BLOCK_TABLE_SIZE,
        .data_block_count = 1, // 1 datablock for root 
    };

    // create bitmap inode
    char inode_bitmap[sfs_sb.blocksize];
    memset(inode_bitmap, 0, sizeof(inode_bitmap));
    inode_bitmap[0] = 1; // mark inode root inuse

    // create data block bitmap
    char data_block_bitmap[sfs_sb.blocksize];
    memset(data_block_bitmap, 0, sizeof(data_block_bitmap));
    data_block_bitmap[0] = 1; // mark the first datablock inuse

    // Xây dựng inode gốc (root inode)
    struct sfs_inode root_sfs_inode = {
        .mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, // Thư mục, quyền truy cập đầy đủ cho user, group và others
        .inode_no = SFS_ROOTDIR_INODE_NO, // Số hiệu inode gốc
        .data_block_no = SFS_DATA_BLOCK_TABLE_START_BLOCK_NO(&sfs_sb)
                + SFS_ROOTDIR_DATA_BLOCK_NO_OFFSET, // Khối dữ liệu cho inode gốc
        .dir_children_count = 0, 
    };

    
    ret = 0;
    do {
        // Ghi siêu khối
        if (sizeof(sfs_sb)
                != write(fd, &sfs_sb, sizeof(sfs_sb))) {
            ret = -1;
            break;
        }
        if ((off_t)-1
                == lseek(fd, sfs_sb.blocksize, SEEK_SET)) {
            ret = -2;
            break;
        }

        // Ghi bitmap inode
        if (sizeof(inode_bitmap)
                != write(fd, inode_bitmap, sizeof(inode_bitmap))) {
            ret = -3;
            break;
        }

        // Ghi bitmap khối dữ liệu
        if (sizeof(data_block_bitmap)
                != write(fd, data_block_bitmap, sizeof(data_block_bitmap))) {
            ret = -4;
            break;
        }

        // Ghi inode gốc
        if (sizeof(root_sfs_inode)
                != write(fd, &root_sfs_inode, sizeof(root_sfs_inode))) {
            ret = -5;
            break;
        }

        // Ghi inode cho tệp welcome
        if (sizeof(welcome_sfs_inode)
                != write(fd, &welcome_sfs_inode, sizeof(welcome_sfs_inode))) {
            ret = -6;
            break;
        }

        // Ghi khối dữ liệu cho inode gốc
        if ((off_t)-1
                == lseek(fd, SFS_DATA_BLOCK_TABLE_START_BLOCK_NO_HSB(&sfs_sb)
                        * sfs_sb.blocksize, SEEK_SET)) {
            ret = -7;
            break;
        }
    } while (0);

    close(fd);
    return ret;
}
