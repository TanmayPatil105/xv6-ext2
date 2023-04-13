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
  /*
   * Calculate the group number of the inode
   * Read the block bitmap
   * Find the block number of the free block
   * Write to the block bitmap that it is no longer free
   * Write zero to that block
   */
  int bno;
  struct ext2_group_desc bgdesc;
  struct buf *bp1;
  bno = GET_GROUP_NO(inum, ext2_sb);
  // iindex = GET_INODE_INDEX(inum, ext2_sb);
  bp1 = bread(dev, 2);
  memmove(&bgdesc, bp1->data + bno * sizeof(bgdesc), sizeof(bgdesc));
  brelse(bp1);
  // bp2 = bread(dev, bgdesc.bg_block_bitmap);
  // iterate through bp->data to get the first free block
  // then bzero the free block
  ext2fs_bzero(dev, inum);
  /*
   * Release the buffer
   * Return the block number of the allocated block
   */
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
  /*
   * EXT2BSIZE -> 1024
   * If < EXT2_NDIR_BLOCKS then it is directly mapped, allocate and return
   * If < 128 (Indirect blocks) then need to allocate using indirect block
   * If < 128*128 (Double indirect) ...
   * If < 128*128*128 (Triple indirect) ...
   * Else panic()
  */
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
  return 0;
}

int
ext2fs_writei(struct inode *ip, char *src, uint off, uint n)
{
  ext2fs_bmap(ip, 0);
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

