extern struct inode_operations ext2fs_inode_ops;
extern struct icache icache;

// Block size for ext2
#define EXT2_BSIZE 1024

#define GET_GROUP_NO(inum, ext2_sb) 	((inum - 1) / ext2_sb.s_inodes_per_group)
#define GET_INODE_INDEX(inum, ext2_sb) 	((inum - 1) % ext2_sb.s_inodes_per_group)

/*
 * Constants relative to the data blocks
 */
#define	EXT2_NDIR_BLOCKS		12
#define	EXT2_IND_BLOCK			EXT2_NDIR_BLOCKS
#define	EXT2_DIND_BLOCK			(EXT2_IND_BLOCK + 1)
#define	EXT2_TIND_BLOCK			(EXT2_DIND_BLOCK + 1)
#define	EXT2_N_BLOCKS			(EXT2_TIND_BLOCK + 1)

// for directory entry
#define EXT2_NAME_LEN 255

struct ext2fs_addrs {
  uint busy;
  uint addrs[EXT2_N_BLOCKS];
};
extern struct ext2fs_addrs ext2fs_addrs[NINODE];

struct ext2_super_block {
	uint	s_inodes_count;		/* Inodes count */
	uint	s_blocks_count;		/* Blocks count */
	uint	s_r_blocks_count;	/* Reserved blocks count */
	uint	s_free_blocks_count;	/* Free blocks count */
	uint	s_free_inodes_count;	/* Free inodes count */
	uint	s_first_data_block;	/* First Data Block */
	uint	s_log_block_size;	/* Block size */
	uint	s_log_frag_size;	/* Fragment size */
	uint	s_blocks_per_group;	/* # Blocks per group */
	uint	s_frags_per_group;	/* # Fragments per group */
	uint	s_inodes_per_group;	/* # Inodes per group */
	uint	s_mtime;		/* Mount time */
	uint	s_wtime;		/* Write time */
	ushort	s_mnt_count;		/* Mount count */
	ushort	s_max_mnt_count;	/* Maximal mount count */
	ushort	s_magic;		/* Magic signature */
	ushort	s_state;		/* File system state */
	ushort	s_errors;		/* Behaviour when detecting errors */
	ushort	s_minor_rev_level; 	/* minor revision level */
	uint	s_lastcheck;		/* time of last check */
	uint	s_checkinterval;	/* max. time between checks */
	uint	s_creator_os;		/* OS */
	uint	s_rev_level;		/* Revision level */
	ushort	s_def_resuid;		/* Default uid for reserved blocks */
	ushort	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 *
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	uint	s_first_ino; 		/* First non-reserved inode */
	ushort   s_inode_size; 		/* size of inode structure */
	ushort	s_block_group_nr; 	/* block group # of this superblock */
	uint	s_feature_compat; 	/* compatible feature set */
	uint	s_feature_incompat; 	/* incompatible feature set */
	uint	s_feature_ro_compat; 	/* readonly-compatible feature set */
	uchar	s_uuid[16];		/* 128-bit uuid for volume */
	char	s_volume_name[16]; 	/* volume name */
	char	s_last_mounted[64]; 	/* directory where last mounted */
	uint	s_algorithm_usage_bitmap; /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_COMPAT_PREALLOC flag is on.
	 */
	uchar	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	uchar	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	ushort	s_padding1;
	/*
	 * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
	uchar	s_journal_uuid[16];	/* uuid of journal superblock */
	uint	s_journal_inum;		/* inode number of journal file */
	uint	s_journal_dev;		/* device number of journal file */
	uint	s_last_orphan;		/* start of list of inodes to delete */
	uint	s_hash_seed[4];		/* HTREE hash seed */
	uchar	s_def_hash_version;	/* Default hash version to use */
	uchar	s_reserved_char_pad;
	ushort	s_reserved_word_pad;
	uint	s_default_mount_opts;
	uint	s_first_meta_bg; 	/* First metablock block group */
	uint	s_reserved[190];	/* Padding to the end of the block */
};

struct ext2_group_desc
{
	uint	bg_block_bitmap;		/* Blocks bitmap block */
	uint	bg_inode_bitmap;		/* Inodes bitmap block */
	uint	bg_inode_table;		/* Inodes table block */
	ushort	bg_free_blocks_count;	/* Free blocks count */
	ushort	bg_free_inodes_count;	/* Free inodes count */
	ushort	bg_used_dirs_count;	/* Directories count */
	ushort	bg_pad;
	uint	bg_reserved[3];
};

/*
 * Structure of an inode on the disk
 */
struct ext2_inode {
	ushort	i_mode;		/* File mode */
	ushort	i_uid;		/* Low 16 bits of Owner Uid */
	uint	i_size;		/* Size in bytes */
	uint	i_atime;	/* Access time */
	uint	i_ctime;	/* Creation time */
	uint	i_mtime;	/* Modification time */
	uint	i_dtime;	/* Deletion Time */
	ushort	i_gid;		/* Low 16 bits of Group Id */
	ushort	i_links_count;	/* Links count */
	uint	i_blocks;	/* Blocks count */
	uint	i_flags;	/* File flags */
	union {
		struct {
			uint  l_i_reserved1;
		} linux1;
		struct {
			uint  h_i_translator;
		} hurd1;
		struct {
			uint  m_i_reserved1;
		} masix1;
	} osd1;				/* OS dependent 1 */
	uint	i_block[EXT2_N_BLOCKS];/* Pointers to blocks */
	uint	i_generation;	/* File version (for NFS) */
	uint	i_file_acl;	/* File ACL */
	uint	i_dir_acl;	/* Directory ACL */
	uint	i_faddr;	/* Fragment address */
	union {
		struct {
			ushort	l_i_frag;	/* Fragment number */
			ushort	l_i_fsize;	/* Fragment size */
			ushort	i_pad1;
			ushort	l_i_uid_high;	/* these 2 fields    */
			ushort	l_i_gid_high;	/* were reserved2[0] */
			uint	l_i_reserved2;
		} linux2;
		struct {
			uchar	h_i_frag;	/* Fragment number */
			uchar	h_i_fsize;	/* Fragment size */
			ushort	h_i_mode_high;
			ushort	h_i_uid_high;
			ushort	h_i_gid_high;
			uint	h_i_author;
		} hurd2;
		struct {
			ushort	m_i_frag;	/* Fragment number */
			ushort	m_i_fsize;	/* Fragment size */
			ushort	m_pad1;
			uint	m_i_reserved2[2];
		} masix2;
	} osd2;				/* OS dependent 2 */
};

/*
 * The new version of the directory entry.  Since EXT2 structures are
 * stored in intel byte order, and the name_len field could never be
 * bigger than 255 chars, it's safe to reclaim the extra byte for the
 * file_type field.
 */
struct ext2_dir_entry_2 {
	uint	inode;			/* Inode number */
	ushort	rec_len;		/* Directory entry length */
	uchar	name_len;		/* Name length */
	uchar	file_type;
	char	name[EXT2_NAME_LEN];	/* File name */
};

