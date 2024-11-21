#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sfs.h"


int main(int argc, char *argv[]) {
    int fd;
    ssize_t ret;

    // Mở thiết bị
    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("Error while opening device");
        return -1;
    }

    struct sfs_superblock sfs_sb = {
        .version = 1,
        .magic = SFS_MAGIC,
        .blocksize = SFS_DEFAULT_BLOCKSIZE,
        .inode_table_size = SFS_DEFAULT_INODE_TABLE_SIZE,
        .inode_count = 1, // Chỉ có root inode
        .data_block_table_size = SFS_DEFAULT_DATA_BLOCK_TABLE_SIZE,
        .data_block_count = 1, // 1 data block cho root
    };

    // Tạo root inode
    struct sfs_inode root_sfs_inode = {
        .mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, // Thư mục với quyền đầy đủ
        .inode_no = SFS_ROOTDIR_INODE_NO, // Số inode gốc
        .data_block_no = SFS_INODE_TABLE_START_BLOCK_NO + 1, // Khối dữ liệu đầu tiên
        .dir_children_count = 0, // Chưa có tệp con
    };

    // Ghi superblock
    ret = write(fd, &sfs_sb, sizeof(sfs_sb));
    if (ret != sizeof(sfs_sb)) {
        perror("Failed to write superblock");
        close(fd);
        return -2;
    }

    // Ghi root inode
    if (lseek(fd, SFS_INODE_TABLE_START_BLOCK_NO * sfs_sb.blocksize, SEEK_SET) == (off_t)-1) {
        perror("Failed to seek to inode table");
        close(fd);
        return -3;
    }

    ret = write(fd, &root_sfs_inode, sizeof(root_sfs_inode));
    if (ret != sizeof(root_sfs_inode)) {
        perror("Failed to write root inode");
        close(fd);
        return -4;
    }

    close(fd);
    return 0; 
}
