struct file {
  enum { FD_NONE, FD_PIPE, FD_INODE } type;
  int ref; // reference count
  char readable;
  char writable;
  struct pipe *pipe;
  struct inode *ip;
  uint off;
};

struct inode_operations {
	int             (*dirlink)(struct inode*, char*, uint);
	struct inode*   (*dirlookup)(struct inode*, char*, uint*);
	struct inode*   (*ialloc)(uint, short);
	//struct inode*   (*idup)(struct inode*);
	void            (*iinit)(int dev);
	void            (*ilock)(struct inode*);
	void            (*iput)(struct inode*);
	void            (*iunlock)(struct inode*);
	void            (*iunlockput)(struct inode*);
	void            (*iupdate)(struct inode*);
	//int             (*namecmp)(const char*, const char*);
	//struct inode*   (*namei)(char*);
	//struct inode*   (*nameiparent)(char*, char*);
	int             (*readi)(struct inode*, char*, uint, uint);
	void            (*stati)(struct inode*, struct stat*);
	int             (*writei)(struct inode*, char*, uint, uint);
};

// in-memory copy of an inode
struct inode {
  uint dev;           // Device number
  uint inum;          // Inode number
  int ref;            // Reference count
  struct sleeplock lock; // protects everything below here
  int valid;          // inode has been read from disk?
  struct inode_operations *iops; // pointer to inode_operations

  short type;         // copy of disk inode
  short major;
  short minor;
  short nlink;
  uint size;
  void *addrs;
};

// table mapping major device number to
// device functions
struct devsw {
  int (*read)(struct inode*, char*, int);
  int (*write)(struct inode*, char*, int);
};

extern struct devsw devsw[];

#define CONSOLE 1
