#ifndef _EXT2_QUOTA_H
#define _EXT2_QUOTA_H

#include <linux/uidgid.h>
#include <linux/hashtable.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>

// ext2_quota_info
struct ext2_quota_info {
    uid_t uid;                
    unsigned long limit;      
    unsigned long usage;      
    struct hlist_node node;   // Node for hash table
};

#define QUOTA_HASH_BITS 8
extern DEFINE_HASHTABLE(quota_hash_table, QUOTA_HASH_BITS);

// Define func prototype
void add_quota(uid_t uid, unsigned long limit, unsigned long usage);
void show_quota_table(void);
void cleanup_quota_table(void);
bool check_quota(uid_t uid, unsigned long size);
void update_quota(uid_t uid, unsigned long size);

// Define kprobe func
extern struct kprobe kp_write;
extern int write_pre_handler(struct kprobe *p, struct pt_regs *regs);
extern void write_post_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags);

extern struct kprobe kp_unlink;
extern void unlink_post_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags);

//Define file I/O func
extern void save_quota_to_file(void);
extern void load_quota_from_file(void);

#endif // _EXT2_QUOTA_H
