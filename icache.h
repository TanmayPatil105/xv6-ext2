struct icache{
  struct spinlock lock;
  struct inode inode[NINODE];
};

