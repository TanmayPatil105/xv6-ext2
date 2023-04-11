#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"
#include "file.h"
#include "ext2fs.h"

struct inode_operations ext2fs_inode_ops = {
        ext2fs_dirlink,
        ext2fs_dirlookup,
        ext2fs_ialloc,
        ext2fs_iinit,
        ext2fs_ilock,
        ext2fs_iput,
        ext2fs_iunlock,
        ext2fs_iunlockput,
        ext2fs_iupdate,
        ext2fs_readi,
        ext2fs_stati,
        ext2fs_writei,
};

struct ext2fs_addrs ext2fs_addrs[NINODE];
struct ext2_super_block ext2_sb;

void
ext2fs_readsb(int dev, struct ext2_super_block *ext2_sb)
{
  struct buf *bp;
  bp = bread(dev, 2);
  memmove(ext2_sb, bp->data, sizeof(*ext2_sb));
  brelse(bp);
}

void
ext2fs_iinit(int dev)
{
  ext2fs_readsb(dev, &ext2_sb);
  cprintf("ext2_sb: magic_number %x size %d nblocks %d ninodes %d \
inodes_per_group %d inode_size %d\n", ext2_sb.s_magic, 1024<<ext2_sb.s_log_block_size,
  ext2_sb.s_blocks_count, ext2_sb.s_inodes_count, ext2_sb.s_inodes_per_group,
  ext2_sb.s_inode_size);
}

struct inode*
ext2fs_ialloc(uint dev, short type)
{
  return 0;
}

void
ext2fs_iupdate(struct inode *ip)
{
  return;
}

void
ext2fs_ilock(struct inode *ip)
{
  return;
}

void
ext2fs_iunlock(struct inode *ip)
{
  if (ip == 0 || !holdingsleep(&ip->lock) || ip->ref < 1)
    panic("iunlock");

  releasesleep(&ip->lock);
}

void
ext2fs_iput(struct inode *ip)
{
  return;
}

void
ext2fs_iunlockput(struct inode *ip)
{
  ip->iops->iunlock(ip);
  ip->iops->iput(ip);
}

void
ext2fs_stati(struct inode *ip, struct stat *st)
{
  st->dev = ip->dev;
  st->ino = ip->inum;
  st->type = ip->type;
  st->nlink = ip->nlink;
  st->size = ip->size;
}

int
ext2fs_readi(struct inode *ip, char *dst, uint off, uint n)
{
  return 0;
}

int
ext2fs_writei(struct inode *ip, char *src, uint off, uint n)
{
  return 0;
}

struct inode*
ext2fs_dirlookup(struct inode *dp, char *name, uint *poff)
{
  return 0;
}

int
ext2fs_dirlink(struct inode *dp, char *name, uint num)
{
  return 0;
}

