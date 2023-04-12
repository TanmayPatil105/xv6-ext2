#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "ext2fs.h"
#include "buf.h"
#include "file.h"
#include "icache.h"

#define min(a,b) ((a) < (b) ? (a) : (b))

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

static void ext2fs_bzero(int dev, int bno);
static uint ext2fs_balloc(uint dev, uint inum);
static void ext2fs_bfree(int dev, uint b);
static uint ext2fs_bmap(struct inode *ip, uint bn);
static void ext2fs_itrunc(struct inode *ip);
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

// Zero a block.
static void
ext2fs_bzero(int dev, int bno)
{
  struct buf *bp;

  bp = bread(dev, bno);
  memset(bp->data, 0, BSIZE);
  log_write(bp);
  brelse(bp);
}

// Allocate a zeroed disk block.
static uint
ext2fs_balloc(uint dev, uint inum)
{
  ext2fs_bzero(dev, inum);
  return 0;
}

// Free a disk block.
static void
ext2fs_bfree(int dev, uint b)
{
  return;
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
  struct ext2fs_addrs *ad;
  acquiresleep(&ip->lock);
  ad = (struct ext2fs_addrs *)ip->addrs;
  if(ip->valid && ip->nlink == 0){
    acquire(&icache.lock);
    int r = ip->ref;
    release(&icache.lock);
    if(r == 1){
      // inode has no links and no other references: truncate and free.
      ext2fs_itrunc(ip);
      ip->type = 0;
      ip->iops->iupdate(ip);
      ip->valid = 0;
      ip->addrs = 0;
    }
  }
  releasesleep(&ip->lock);

  acquire(&icache.lock);
  ip->ref--;
  if (ip->ref == 0){
    ad->busy = 0;
    ip->addrs = 0;
  }
  release(&icache.lock);

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

// Inode content
//
// The content (data) associated with each inode is stored
// in blocks on the disk. The first NDIRECT block numbers
// are listed in ip->addrs[].  The next NINDIRECT blocks are
// listed in block ip->addrs[NDIRECT].

// Return the disk block address of the nth block in inode ip.
// If there is no such block, bmap allocates one.
static uint
ext2fs_bmap(struct inode *ip, uint bn)
{
  ext2fs_balloc(ip->dev, ip->inum);
  return 0;
}

// Truncate inode (discard contents).
// Only called when the inode has no links
// to it (no directory entries referring to it)
// and has no in-memory reference to it (is
// not an open file or current directory).
static void
ext2fs_itrunc(struct inode *ip)
{
  ext2fs_bfree(ip->dev, 0);
  return;
}

int
ext2fs_readi(struct inode *ip, char *dst, uint off, uint n)
{
  uint tot, m;
  struct buf *bp;

  if(ip->type == T_DEV){
    if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].read)
      return -1;
    return devsw[ip->major].read(ip, dst, n);
  }

  if(off > ip->size || off + n < off)
    return -1;
  if(off + n > ip->size)
    n = ip->size - off;

  for(tot=0; tot<n; tot+=m, off+=m, dst+=m){
    bp = bread(ip->dev, ext2fs_bmap(ip, off/BSIZE));
    m = min(n - tot, BSIZE - off%BSIZE);
    memmove(dst, bp->data + off%BSIZE, m);
    brelse(bp);
  }
  return n;
}

int
ext2fs_writei(struct inode *ip, char *src, uint off, uint n)
{
  uint tot, m;
  struct buf *bp;

  if(ip->type == T_DEV){
    if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].write)
      return -1;
    return devsw[ip->major].write(ip, src, n);
  }

  if(off > ip->size || off + n < off)
    return -1;
  if(off + n > MAXFILE*BSIZE)
    return -1;

  for(tot=0; tot<n; tot+=m, off+=m, src+=m){
    bp = bread(ip->dev, ext2fs_bmap(ip, off/BSIZE));
    m = min(n - tot, BSIZE - off%BSIZE);
    memmove(bp->data + off%BSIZE, src, m);
    log_write(bp);
    brelse(bp);
  }

  if(n > 0 && off > ip->size){
    ip->size = off;
    ip->iops->iupdate(ip);
  }
  return n;
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

