extern struct inode_operations ext2fs_inode_ops;


#define EXT2_NAME_LEN 			255

/*
 * Constants relative to the data blocks
 */
#define EXT2_NDIR_BLOCKS                12
#define EXT2_IND_BLOCK                  EXT2_NDIR_BLOCKS
#define EXT2_DIND_BLOCK                 (EXT2_IND_BLOCK + 1)
#define EXT2_TIND_BLOCK                 (EXT2_DIND_BLOCK + 1)
#define EXT2_N_BLOCKS                   (EXT2_TIND_BLOCK + 1)

#define EXT2_LABEL_LEN			16

struct ext2_super_block {
/*000*/ uint   s_inodes_count;         /* Inodes count */
        uint   s_blocks_count;         /* Blocks count */
        uint   s_r_blocks_count;       /* Reserved blocks count */
        uint   s_free_blocks_count;    /* Free blocks count */
/*010*/ uint   s_free_inodes_count;    /* Free inodes count */
        uint   s_first_data_block;     /* First Data Block */
        uint   s_log_block_size;       /* Block size */
        uint   s_log_cluster_size;     /* Allocation cluster size */
/*020*/ uint   s_blocks_per_group;     /* # Blocks per group */
        uint   s_clusters_per_group;   /* # Fragments per group */
        uint   s_inodes_per_group;     /* # Inodes per group */
        uint   s_mtime;                /* Mount time */
/*030*/ uint   s_wtime;                /* Write time */
        uint   s_mnt_count;            /* Mount count */
        uint   s_max_mnt_count;        /* Maximal mount count */
        uint   s_magic;                /* Magic signature */
        uint   s_state;                /* File system state */
        uint   s_errors;               /* Behaviour when detecting errors */
        uint   s_minor_rev_level;      /* minor revision level */
/*040*/ uint   s_lastcheck;            /* time of last check */
        uint   s_checkinterval;        /* max. time between checks */
        uint   s_creator_os;           /* OS */
        uint   s_rev_level;            /* Revision level */
/*050*/ uint   s_def_resuid;           /* Default uid for reserved blocks */
        uint   s_def_resgid;           /* Default gid for reserved blocks */
        uint   s_first_ino;            /* First non-reserved inode */
        uint   s_inode_size;           /* size of inode structure */
        uint   s_block_group_nr;       /* block group # of this superblock */
        uint   s_feature_compat;       /* compatible feature set */
/*060*/ uint   s_feature_incompat;     /* incompatible feature set */
        uint   s_feature_ro_compat;    /* readonly-compatible feature set */
/*0c8*/ uint   s_algorithm_usage_bitmap; /* For compression */
        /*
         * Performance hints.  Directory preallocation should only
         * happen if the EXT2_FEATURE_COMPAT_DIR_PREALLOC flag is on.
         */
        uint   s_prealloc_blocks;      /* Nr of blocks to try to preallocate*/
        uint   s_prealloc_dir_blocks;  /* Nr to preallocate for dirs */
        uint   s_reserved_gdt_blocks;  /* Per group table for online growth */
        /*
         * Journaling support valid if EXT2_FEATURE_COMPAT_HAS_JOURNAL set.
         */
/*0e0*/ uint   s_journal_inum;         /* inode number of journal file */
        uint   s_journal_dev;          /* device number of journal file */
        uint   s_last_orphan;          /* start of list of inodes to delete */
/*0ec*/ uint   s_hash_seed[4];         /* HTREE hash seed */
/*0fc*/ uint   s_def_hash_version;     /* Default hash version to use */
        uint   s_jnl_backup_type;      /* Default type of journal backup */
        uint   s_desc_size;            /* Group desc. size: INCOMPAT_64BIT */
/*100*/ uint   s_default_mount_opts;   /* default EXT2_MOUNT_* flags used */
        uint   s_first_meta_bg;        /* First metablock group */
        uint   s_mkfs_time;            /* When the filesystem was created */
/*10c*/ uint   s_jnl_blocks[17];       /* Backup of the journal inode */
/*150*/ uint   s_blocks_count_hi;      /* Blocks count high 32bits */
        uint   s_r_blocks_count_hi;    /* Reserved blocks count high 32 bits*/
        uint   s_free_blocks_hi;       /* Free blocks count */
        uint   s_min_extra_isize;      /* All inodes have at least # bytes */
        uint   s_want_extra_isize;     /* New inodes should reserve # bytes */
/*160*/ uint   s_flags;                /* Miscellaneous flags */
        uint   s_raid_stride;          /* RAID stride in blocks */
        uint   s_mmp_update_interval;  /* # seconds to wait in MMP checking */
        uint   s_mmp_block;            /* Block for multi-mount protection */
/*170*/ uint   s_raid_stripe_width;    /* blocks on all data disks (N*stride)*/
        uint   s_log_groups_per_flex;  /* FLEX_BG group size */
        uint   s_checksum_type;        /* metadata checksum algorithm */
        uint   s_encryption_level;     /* versioning level for encryption */
        uint   s_reserved_pad;         /* Padding to next 32bits */
        uint   s_kbytes_written;       /* nr of lifetime kilobytes written */
/*180*/ uint   s_snapshot_inum;        /* Inode number of active snapshot */
        uint   s_snapshot_id;          /* sequential ID of active snapshot */
        uint   s_snapshot_r_blocks_count; /* active snapshot reserved blocks */
/*190*/ uint   s_snapshot_list;        /* inode number of disk snapshot list */
#define EXT4_S_ERR_START ext4_offsetof(struct ext2_super_block, s_error_count)
        uint   s_error_count;          /* number of fs errors */
        uint   s_first_error_time;     /* first time an error happened */
        uint   s_first_error_ino;      /* inode involved in first error */
/*1a0*/ uint   s_first_error_block;    /* block involved in first error */
/*1c8*/ uint   s_first_error_line;     /* line number where error happened */
        uint   s_last_error_time;      /* most recent time of an error */
/*1d0*/ uint   s_last_error_ino;       /* inode involved in last error */
        uint   s_last_error_line;      /* line number where error happened */
        uint   s_last_error_block;     /* block involved of last error */
#define EXT4_S_ERR_END ext4_offsetof(struct ext2_super_block, s_mount_opts)
/*240*/ uint   s_usr_quota_inum;       /* inode number of user quota file */
        uint   s_grp_quota_inum;       /* inode number of group quota file */
        uint   s_overhead_clusters;    /* overhead blocks/clusters in fs */
/*24c*/ uint   s_backup_bgs[2];        /* If sparse_super2 enabled */
/*254*/ uint   s_encrypt_algos[4];     /* Encryption algorithms in use  */
/*258*/ uint   s_encrypt_pw_salt[16];  /* Salt used for string2key algorithm */
/*268*/ uint   s_lpf_ino;              /* Location of the lost+found inode */
        uint   s_prj_quota_inum;       /* inode for tracking project quota */
/*270*/ uint   s_checksum_seed;        /* crc32c(orig_uuid) if csum_seed set */
/*274*/ uint   s_wtime_hi;
        uint   s_mtime_hi;
        uint   s_mkfs_time_hi;
        uint   s_lastcheck_hi;
        uint   s_first_error_time_hi;
        uint   s_last_error_time_hi;
        uint   s_first_error_errcode;
        uint   s_last_error_errcode;
/*27c*/ uint   s_encoding;             /* Filename charset encoding */
        uint   s_encoding_flags;       /* Filename charset encoding flags */
        uint   s_reserved[95];         /* Padding to the end of the block */
/*3fc*/ uint   s_checksum;             /* crc32c(superblock) */
};

struct ext2_inode {
/*00*/  uint   i_mode;         /* File mode */
        uint   i_uid;          /* Low 16 bits of Owner Uid */
        uint   i_size;         /* Size in bytes */
        uint   i_atime;        /* Access time */
        uint   i_ctime;        /* Inode change time */
/*10*/  uint   i_mtime;        /* Modification time */
        uint   i_dtime;        /* Deletion Time */
        uint   i_gid;          /* Low 16 bits of Group Id */
        uint   i_links_count;  /* Links count */
        uint   i_blocks;       /* Blocks count */
/*20*/  uint   i_flags;        /* File flags */
        union {
                struct {
                        uint  l_i_version; /* was l_i_reserved1 */
                } linux1;
                struct {
                        uint  h_i_translator;
                } hurd1;
        } osd1;                         /* OS dependent 1 */
/*28*/  uint   i_block[EXT2_N_BLOCKS];/* Pointers to blocks */
/*64*/  uint   i_generation;   /* File version (for NFS) */
        uint   i_file_acl;     /* File ACL */
        uint   i_size_high;
/*70*/  uint   i_faddr;        /* Fragment address */
        union {
                struct {
                        uint   l_i_blocks_hi;
                        uint   l_i_file_acl_high;
                        uint   l_i_uid_high;   /* these 2 fields    */
                        uint   l_i_gid_high;   /* were reserved2[0] */
                        uint   l_i_checksum_lo; /* crc32c(uuid+inum+inode) */
                        uint   l_i_reserved;
                } linux2;
                struct {
                        uint   h_i_frag;       /* Fragment number */
                        uint   h_i_fsize;      /* Fragment size */
                        uint   h_i_mode_high;
                        uint   h_i_uid_high;
                        uint   h_i_gid_high;
                        uint   h_i_author;
                } hurd2;
        } osd2;                         /* OS dependent 2 */
};

struct ext2_group_desc
{
        uint   bg_block_bitmap;        /* Blocks bitmap block */
        uint   bg_inode_bitmap;        /* Inodes bitmap block */
        uint   bg_inode_table;         /* Inodes table block */
        uint   bg_free_blocks_count;   /* Free blocks count */
        uint   bg_free_inodes_count;   /* Free inodes count */
        uint   bg_used_dirs_count;     /* Directories count */
        uint   bg_flags;
        uint   bg_exclude_bitmap_lo;   /* Exclude bitmap for snapshots */
        uint   bg_block_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bitmap) LSB */
        uint   bg_inode_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bitmap) LSB */
        uint   bg_itable_unused;       /* Unused inodes count */
        uint   bg_checksum;            /* crc16(s_uuid+group_num+group_desc)*/
};

struct ext2_dir_entry_2 {
        uint   inode;                  /* Inode number */
        uint   rec_len;                /* Directory entry length */
        uint   name_len;               /* Name length */
        uint   file_type;
        char   name[EXT2_NAME_LEN];    /* File name */
};



