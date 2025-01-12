# Custom EXT2 File System - README

## Project Overview
This project involves custom modifications to the EXT2 file system to enhance its functionality. It consists of two main components:

1. **ext2-modified**: A customized version of the EXT2 file system with added features.
2. **quota_module**: A kernel module for managing user quotas on the EXT2 file system.

## Features

### Ext2-modified
- **Create Time Attribute**: Adds a `create_time` field to the inode structure using extended attributes (xattr) to automatically track the creation time of files.
- **File/Folder Operation Logging**: Logs operations such as file/folder creation, deletion, and movement to aid in auditing and monitoring.

### Quota_module
- **Quota Management**: Implements a quota system for the EXT2 file system using kprobe to monitor and enforce user limits on file system usage.

---

## Setup Instructions

### Prerequisites
- Linux kernel source code (ensure it matches your running kernel version).
- GCC and make tools installed.
- Basic understanding of Linux kernel compilation.

### Enable EXT2 Filesystem Support

#### 1. Download and Prepare the Kernel Source
- If you don’t have the kernel source code, download it from the official repository or your distribution's package manager.
- Extract the source code and navigate to its directory:
  ```bash
  tar -xvf linux-x.x.x.tar.xz
  cd linux-x.x.x
  ```

#### 2. Enable EXT2 Filesystem Support
- Launch the kernel configuration menu:
  ```bash
  make menuconfig
  ```
- In the configuration menu:
  - Navigate to **File Systems** > **<*> Second extended fs support (EXT2)**.
  - Select `<*>` to build EXT2 support directly into the kernel or `<M>` to build it as a loadable module.

#### 3. Build the Kernel
- Start compiling the kernel using the following commands:
  ```bash
  make -j$(nproc)
  make modules_install
  make install
  ```
  - `-j$(nproc)` allows the build process to use all available CPU cores for faster compilation.

#### 4. Update GRUB Bootloader
- Update the GRUB bootloader to recognize the newly built kernel:
  ```bash
  sudo update-grub
  ```

#### 5. Reboot and Verify
- Reboot your system to load the new kernel:
  ```bash
  sudo reboot
  ```
- After rebooting, verify that your system is running the updated kernel:
  ```bash
  uname -r
  ```

--- 

#### Additional Notes
- Ensure that the EXT2 filesystem support is selected correctly in the `menuconfig` step to avoid any runtime issues.
- Always back up your existing kernel and GRUB configuration files before making changes.
- For debugging or testing purposes, you may want to keep your previous kernel available in the bootloader.

### Compilation and Installation

#### 1. Build ext2-modified
1. Navigate to the directory containing the custom EXT2 source code.
2. Run the following commands to compile and install the EXT2 file system:
   ```bash
   cd ext2_modified
   sudo make
   ```

3. Load the custom EXT2 file system and verify the module is loaded:

   ```bash
   sudo insmod ext2.ko
   sudo lsmod | grep ext2
   ```

#### 2. Build and Load quota_module
1. Navigate to the `quota_module` directory.
2. Run the following commands to build and load the module:

   ```bash
   cd quota_module
   sudo make
   sudo insmod ext2_quota.ko
   ```

3. Verify the module is loaded:

   ```bash
   lsmod | grep ext2_quota
   ```

---

## Usage

### ext2-modified
1. Mount a partition with the custom EXT2 file system, create a log file :

   ```bash
   sudo mount -t ext2 /dev/sdX /mnt
   sudo touch /var/log/ext2_log
   ```

2. Create files/folders as usual. The `create_time` attribute will be added automatically. You can check it with command 
   ```bash
   sudo getfattr -n user.creation_time file/folder_name
   ```
3. Logs for operations will be available in the log file in `/var/log/ext2_log`:

   ```bash
   cat /var/log/ext2_log
   ```

### quota_module
1. Set quotas for a user (example):

   ```bash
   sudo mkdir -p /etc/ext2_quota
   echo "<user_id> <quota_limit_in_bytes> <usage_default=0>" > /etc/ext2_quota/quotas
   ```

2. Monitor usage and enforce limits dynamically.
   
   ```bash
   sudo dmesg | tail -n 30
   ```
---
