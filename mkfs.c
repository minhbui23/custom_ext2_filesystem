#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hellofs.h"

int mkfs_hellofs(const char *device_path) {
    int fd;
    ssize_t ret;
    uint64_t welcome_inode_no;
    uint64_t welcome_data_block_no_offset;

    // Mở thiết bị
    fd = open(device_path, O_RDWR);
    if (fd == -1) {
        perror("Lỗi khi mở thiết bị");
        return -1;
    }

    // Xây dựng siêu khối (superblock)
    struct hellofs_superblock hellofs_sb = {
        .version = 1,
        .magic = HELLOFS_MAGIC,
        .blocksize = HELLOFS_DEFAULT_BLOCKSIZE,
        .inode_table_size = HELLOFS_DEFAULT_INODE_TABLE_SIZE,
        .inode_count = 2, // Ban đầu có 2 inode: root và welcome file
        .data_block_table_size = HELLOFS_DEFAULT_DATA_BLOCK_TABLE_SIZE,
        .data_block_count = 2, // Ban đầu có 2 khối dữ liệu: cho root và welcome file
    };

    // Xây dựng bitmap inode
    char inode_bitmap[hellofs_sb.blocksize];
    memset(inode_bitmap, 0, sizeof(inode_bitmap));
    inode_bitmap[0] = 1; // Đánh dấu inode root đã được sử dụng

    // Xây dựng bitmap khối dữ liệu
    char data_block_bitmap[hellofs_sb.blocksize];
    memset(data_block_bitmap, 0, sizeof(data_block_bitmap));
    data_block_bitmap[0] = 1; // Đánh dấu khối dữ liệu đầu tiên đã được sử dụng

    // Xây dựng inode gốc (root inode)
    struct hellofs_inode root_hellofs_inode = {
        .mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, // Thư mục, quyền truy cập đầy đủ cho user, group và others
        .inode_no = HELLOFS_ROOTDIR_INODE_NO, // Số hiệu inode gốc
        .data_block_no = HELLOFS_DATA_BLOCK_TABLE_START_BLOCK_NO_HSB(&hellofs_sb)
                + HELLOFS_ROOTDIR_DATA_BLOCK_NO_OFFSET, // Khối dữ liệu cho inode gốc
        .dir_children_count = 1, // Ban đầu có 1 mục con: welcome file
    };

    // Xây dựng inode cho tệp welcome
    char welcome_body[] = "Welcome Hellofs!!\n";
    welcome_inode_no = HELLOFS_ROOTDIR_INODE_NO + 1; // Số hiệu inode cho welcome file
    welcome_data_block_no_offset = HELLOFS_ROOTDIR_DATA_BLOCK_NO_OFFSET + 1; // Offset khối dữ liệu cho welcome file
    struct hellofs_inode welcome_hellofs_inode = {
        .mode = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, // Tệp thông thường, quyền đọc/ghi cho user và group, quyền đọc cho others
        .inode_no = welcome_inode_no,
        .data_block_no = HELLOFS_DATA_BLOCK_TABLE_START_BLOCK_NO_HSB(&hellofs_sb)
                + welcome_data_block_no_offset, // Khối dữ liệu cho welcome file
        .file_size = sizeof(welcome_body), // Kích thước của tệp welcome
    };

    // Xây dựng khối dữ liệu cho inode gốc
    struct hellofs_dir_record root_dir_records[] = {
        {
            .filename = "wel_helo.txt", // Tên của welcome file
            .inode_no = welcome_inode_no, // Số hiệu inode của welcome file
        },
    };

    ret = 0;
    do {
        // Ghi siêu khối
        if (sizeof(hellofs_sb)
                != write(fd, &hellofs_sb, sizeof(hellofs_sb))) {
            ret = -1;
            break;
        }
        if ((off_t)-1
                == lseek(fd, hellofs_sb.blocksize, SEEK_SET)) {
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
        if (sizeof(root_hellofs_inode)
                != write(fd, &root_hellofs_inode, sizeof(root_hellofs_inode))) {
            ret = -5;
            break;
        }

        // Ghi inode cho tệp welcome
        if (sizeof(welcome_hellofs_inode)
                != write(fd, &welcome_hellofs_inode, sizeof(welcome_hellofs_inode))) {
            ret = -6;
            break;
        }

        // Ghi khối dữ liệu cho inode gốc
        if ((off_t)-1
                == lseek(fd, HELLOFS_DATA_BLOCK_TABLE_START_BLOCK_NO_HSB(&hellofs_sb)
                        * hellofs_sb.blocksize, SEEK_SET)) {
            ret = -7;
            break;
        }
        if (sizeof(root_dir_records)
                != write(fd, root_dir_records, sizeof(root_dir_records))) {
            ret = -8;
            break;
        }

        // Ghi khối dữ liệu cho tệp welcome
        if ((off_t)-1
                == lseek(fd, (HELLOFS_DATA_BLOCK_TABLE_START_BLOCK_NO_HSB(&hellofs_sb) + 1)
                        * hellofs_sb.blocksize, SEEK_SET)) {
            ret = -9;
            break;
        }
        if (sizeof(welcome_body) != write(fd, welcome_body, sizeof(welcome_body))) {
            ret = -10;
            break;
        }
    } while (0);

    close(fd);
    return ret;
}
