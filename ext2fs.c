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

#define min(a,b) ((a) < (b) ? (a) : (b))

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
  bp = bread(dev, 1);
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
  bwrite(bp);
  brelse(bp);
}

// check if a block is free and return it's bit number

static uint
ext2fs_free_block(char *bitmap)
{
  int i, mask;
  for(i = 0; i < ext2_sb.s_blocks_per_group * 8; i++)
  {
    mask = 1 << (i % 8);
    if ((bitmap[i / 8] & mask) == 0)
    {
      bitmap[i / 8] = bitmap[i / 8] | mask;
      return i;
    }
  }
  return -1;
}

// Allocate a zeroed disk block.
static uint
ext2fs_balloc(uint dev, uint inum)
{
  int gno, fbit, zbno;
  struct ext2_group_desc bgdesc;
  struct buf *bp1, *bp2;

  gno = GET_GROUP_NO(inum, ext2_sb);
  bp1 = bread(dev, 2);
  memmove(&bgdesc, bp1->data + gno * sizeof(bgdesc), sizeof(bgdesc));
  brelse(bp1);
  bp2 = bread(dev, bgdesc.bg_block_bitmap);

  fbit = ext2fs_free_block((char *)bp2->data);
  if (fbit > -1)
  {
    zbno = bgdesc.bg_block_bitmap + fbit;
    bwrite(bp2);
    ext2fs_bzero(dev, zbno);
    brelse(bp2);
    return zbno;
  }
  brelse(bp2);
  panic("ext2_balloc: out of blocks\n");
}

// Free a disk block.
static void
ext2fs_bfree(int dev, uint b)
{
  int gno, iindex, mask;
  struct ext2_group_desc bgdesc;
  struct buf *bp1, *bp2;

  gno = GET_GROUP_NO(b, ext2_sb);
  iindex = GET_INODE_INDEX(b, ext2_sb);
  bp1 = bread(dev, 2);
  memmove(&bgdesc, bp1->data + gno * sizeof(bgdesc), sizeof(bgdesc));
  bp2 = bread(dev, bgdesc.bg_block_bitmap);
  iindex -= bgdesc.bg_block_bitmap;
  mask = 1 << (iindex % 8);

  if ((bp2->data[iindex / 8] & mask) == 0)
    panic("ext2fs_bfree: block already free\n");
  bp2->data[iindex / 8] = bp2->data[iindex / 8] & ~mask;
  bwrite(bp2);
  brelse(bp2);
  brelse(bp1);
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
  int i, fbit, bno, iindex, bgcount, inum;
  struct buf *bp1, *bp2, *bp3;
  struct ext2_inode *din;
  struct ext2_group_desc bgdesc;

  bgcount = ext2_sb.s_blocks_count / ext2_sb.s_blocks_per_group;
  for (i = 0; i <= bgcount; i++){
    bp1 = bread(dev, 2);
    memmove(&bgdesc, bp1->data + i * sizeof(bgdesc), sizeof(bgdesc));
    brelse(bp1);

    bp2 = bread(dev, bgdesc.bg_inode_bitmap);
    fbit = ext2fs_free_block((char *)bp2->data);
    if (fbit == -1){
      brelse(bp2);
      continue;
    }

    bno = bgdesc.bg_inode_table + fbit / (EXT2_BSIZE / sizeof(struct ext2_inode));    iindex = fbit % (EXT2_BSIZE / sizeof(struct ext2_inode));
    bp3 = bread(dev, bno);
    din = (struct ext2_inode *)bp3->data + iindex;
    memset(din, 0, sizeof(*din));
    if (type == T_DIR)
      din->i_mode = S_IFDIR;
    else if (type == T_FILE)
      din->i_mode = S_IFREG;
    bwrite(bp3);
    bwrite(bp2);
    brelse(bp3);
    brelse(bp2);

    inum = i * ext2_sb.s_inodes_per_group + fbit + 1;
    return iget(dev, inum);
  }
  panic("ext2_ialloc: no inodes");
}

void
ext2fs_iupdate(struct inode *ip)
{
  struct buf *bp, *bp1;
  struct ext2_group_desc bgdesc;
  struct ext2_inode din;
  struct ext2fs_addrs *ad;
  int gno, ioff, bno, iindex;

  gno = GET_GROUP_NO(ip->inum, ext2_sb);
  ioff = GET_INODE_INDEX(ip->inum, ext2_sb);
  bp = bread(ip->dev, 2);
  memmove(&bgdesc, bp->data + gno * sizeof(bgdesc), sizeof(bgdesc));
  brelse(bp);
  bno = bgdesc.bg_inode_table + ioff / (EXT2_BSIZE / ext2_sb.s_inode_size);
  iindex = ioff % (EXT2_BSIZE / ext2_sb.s_inode_size);
  bp1 = bread(ip->dev, bno);
  memmove(&din, bp1->data + iindex * ext2_sb.s_inode_size, sizeof(din));

  if (ip->type == T_DIR)
    din.i_mode = S_IFDIR;
  if (ip->type == T_FILE)
    din.i_mode = S_IFREG;
  din.i_links_count = ip->nlink;
  din.i_size = ip->size;
  din.i_dtime = 0;
  din.i_faddr = 0;
  din.i_file_acl = 0;
  din.i_flags = 0;
  din.i_generation = 0;
  din.i_gid = 0;
  din.i_mtime = 0;
  din.i_uid = 0;
  din.i_atime = 0;

  ad = (struct ext2fs_addrs *)ip->addrs;
  memmove(din.i_block, ad->addrs, sizeof(ad->addrs));
  memmove(bp1->data + (iindex * ext2_sb.s_inode_size), &din, sizeof(din));
  bwrite(bp1);
  brelse(bp1);
}

void
ext2fs_ilock(struct inode *ip)
{
  struct buf *bp, *bp1;
  struct ext2_group_desc bgdesc;
  struct ext2_inode din;
  struct ext2fs_addrs *ad;
  int gno, ioff, bno, iindex;
  if (ip == 0 || ip->ref < 1)
    panic("ext2fs_ilock");

  acquiresleep(&ip->lock);
  ad = (struct ext2fs_addrs *)ip->addrs;

  if (ip->valid == 0){
    gno = GET_GROUP_NO(ip->inum, ext2_sb);
    ioff = GET_INODE_INDEX(ip->inum, ext2_sb);
    bp = bread(ip->dev, 2);
    memmove(&bgdesc, bp->data + gno * sizeof(bgdesc), sizeof(bgdesc));
    brelse(bp);
    bno = bgdesc.bg_inode_table + ioff / (EXT2_BSIZE / ext2_sb.s_inode_size);
    iindex = ioff % (EXT2_BSIZE / ext2_sb.s_inode_size);
    bp1 = bread(ip->dev, bno);
    memmove(&din, bp1->data + iindex * ext2_sb.s_inode_size, sizeof(din));
    brelse(bp1);

    if (S_ISDIR(din.i_mode) || din.i_mode == T_DIR)
      ip->type = T_DIR;
    else
      ip->type = T_FILE;
    ip->major = 0;
    ip->minor = 0;
    ip->nlink = din.i_links_count;
    ip->size = din.i_size;
    ip->iops = &ext2fs_inode_ops;
    memmove(ad->addrs, din.i_block, sizeof(ad->addrs));

    ip->valid = 1;
    if (ip->type == 0)
      panic("ext2fs_ilock: no type");
  }
  return;
}

void
ext2fs_iunlock(struct inode *ip)
{
  if (ip == 0 || !holdingsleep(&ip->lock) || ip->ref < 1)
    panic("ext2fs_iunlock");

  releasesleep(&ip->lock);
}

// Free a inode
static void
ext2fs_ifree(struct inode *ip)
{
  int gno, index, mask;
  struct ext2_group_desc bgdesc;
  struct buf *bp1, *bp2;

  gno = GET_GROUP_NO(ip->inum, ext2_sb);
  bp1 = bread(ip->dev, 2);
  memmove(&bgdesc, bp1->data + gno * sizeof(bgdesc), sizeof(bgdesc));
  brelse(bp1);
  bp2 = bread(ip->dev, bgdesc.bg_inode_bitmap);
  index = (ip->inum - 1) % ext2_sb.s_inodes_per_group;
  mask = 1 << (index % 8);

  if ((bp2->data[index / 8] & mask) == 0)
    panic("ext2fs_ifree: inode already free\n");
  bp2->data[index / 8] = bp2->data[index / 8] & ~mask;
  bwrite(bp2);
  brelse(bp2);
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
      ext2fs_ifree(ip);
      ext2fs_itrunc(ip);
      ip->type = 0;
      ip->iops->iupdate(ip);
      ip->valid = 0;
      ip->iops = 0;
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
  /*
   * EXT2BSIZE -> 1024
   * If < EXT2_NDIR_BLOCKS then it is directly mapped, allocate and return
   * If < 128 (Indirect blocks) then need to allocate using indirect block
   * If < 128*128 (Double indirect) ...
   * If < 128*128*128 (Triple indirect) ...
   * Else panic()
  */
static uint
ext2fs_bmap(struct inode *ip, uint bn)
{
  ext2fs_balloc(ip->dev, ip->inum);
  uint addr, *a, *b, *c;
  struct buf *bp, *bp1, *bp2;
  struct ext2fs_addrs *ad;
  ad = (struct ext2fs_addrs *)ip->addrs;

  if (bn < EXT2_NDIR_BLOCKS){
    if ((addr = ad->addrs[bn]) == 0)
      ad->addrs[bn] = addr = ext2fs_balloc(ip->dev, ip->inum);
    return addr;
  }
  bn -= EXT2_NDIR_BLOCKS;
  if (bn < EXT2_INDIRECT){
    if ((addr = ad->addrs[EXT2_IND_BLOCK]) == 0)
      ad->addrs[EXT2_IND_BLOCK] = addr = ext2fs_balloc(ip->dev, ip->inum);
    bp = bread(ip->dev, addr);
    a = (uint *)bp->data;
    if ((addr = a[bn]) == 0)
      a[bn] = addr = ext2fs_balloc(ip->dev, ip->inum);
    brelse(bp);
    return addr;
  }
  bn -= EXT2_INDIRECT;

  if (bn < EXT2_DINDIRECT){
    if ((addr = ad->addrs[EXT2_DIND_BLOCK]) == 0)
      ad->addrs[EXT2_DIND_BLOCK] = addr = ext2fs_balloc(ip->dev, ip->inum);
    bp = bread(ip->dev, addr);
    a = (uint *)bp->data;
    if ((addr = a[bn / EXT2_INDIRECT]) == 0)
      a[bn / EXT2_INDIRECT] = addr = ext2fs_balloc(ip->dev, ip->inum);
    bp1 = bread(ip->dev, addr);
    b = (uint *)bp1->data;
    if ((addr = b[bn / EXT2_INDIRECT]) == 0)
      b[bn / EXT2_INDIRECT] = addr = ext2fs_balloc(ip->dev, ip->inum);
    brelse(bp);
    brelse(bp1);
    return addr;
  }
  bn -= EXT2_DINDIRECT;

  if (bn < EXT2_TINDIRECT){
    if ((addr = ad->addrs[EXT2_TIND_BLOCK]) == 0)
      ad->addrs[EXT2_TIND_BLOCK] = addr = ext2fs_balloc(ip->dev, ip->inum);
    bp = bread(ip->dev, addr);
    a = (uint *)bp->data;
    if ((addr = a[bn / EXT2_INDIRECT]) == 0)
      a[bn / EXT2_INDIRECT] = addr = ext2fs_balloc(ip->dev, ip->inum);
    bp1 = bread(ip->dev, addr);
    b = (uint *)bp1->data;
    if ((addr = b[bn / EXT2_INDIRECT]) == 0)
      b[bn / EXT2_INDIRECT] = addr = ext2fs_balloc(ip->dev, ip->inum);
    bp2 = bread(ip->dev, addr);
    c = (uint *)bp2->data;
    if ((addr = c[bn / EXT2_INDIRECT]) == 0)
      c[bn / EXT2_INDIRECT] = addr = ext2fs_balloc(ip->dev, ip->inum);
    brelse(bp);
    brelse(bp1);
    brelse(bp2);
    return addr;
  }
  panic("ext2_bmap: block number out of range\n");
}

// Truncate inode (discard contents).
// Only called when the inode has no links
// to it (no directory entries referring to it)
// and has no in-memory reference to it (is
// not an open file or current directory).
static void
ext2fs_itrunc(struct inode *ip)
{
  int i, j, k;
  struct buf *bp1, *bp2, *bp3;
  uint *a, *b, *c;
  struct ext2fs_addrs *ad;
  ad = (struct ext2fs_addrs *)ip->addrs;

  // for direct blocks
  for (i = 0; i < EXT2_NDIR_BLOCKS; i++){
    if (ad->addrs[i]){
      ext2fs_bfree(ip->dev, ad->addrs[i]);
      ad->addrs[i] = 0;
    }
  }
  // EXT2_INDIRECT -> (EXT2_BSIZE / sizeof(uint))
  // for indirect blocks
  if (ad->addrs[EXT2_IND_BLOCK]){
    bp1 = bread(ip->dev, ad->addrs[EXT2_IND_BLOCK]);
    a = (uint *)bp1->data;
    for (i = 0; i < EXT2_INDIRECT; i++){
      if(a[i]){
        ext2fs_bfree(ip->dev, a[i]);
        a[i] = 0;
      }
    }
    brelse(bp1);
    ext2fs_bfree(ip->dev, ad->addrs[EXT2_IND_BLOCK]);
    ad->addrs[EXT2_IND_BLOCK] = 0;
  }

  // for double indirect blocks
  if (ad->addrs[EXT2_DIND_BLOCK]){
    bp1 = bread(ip->dev, ad->addrs[EXT2_DIND_BLOCK]);
    a = (uint *)bp1->data;
    for (i = 0; i < EXT2_INDIRECT; i++){
      if(a[i]){
        bp2 = bread(ip->dev, a[i]);
	b = (uint *)bp2->data;
	for (j = 0; j < EXT2_INDIRECT; j++){
          if(b[j]){
	    ext2fs_bfree(ip->dev, b[j]);
            b[j] = 0;
	  }
	}
	brelse(bp2);
	ext2fs_bfree(ip->dev, a[i]);
	a[i] = 0;
      }
    }
    brelse(bp1);
    ext2fs_bfree(ip->dev, ad->addrs[EXT2_DIND_BLOCK]);
    ad->addrs[EXT2_DIND_BLOCK] = 0;
  }

  // for triple indirect blocks
  if (ad->addrs[EXT2_TIND_BLOCK]){
    bp1 = bread(ip->dev, ad->addrs[EXT2_TIND_BLOCK]);
    a = (uint *)bp1->data;
    for (i = 0; i < EXT2_INDIRECT; i++){
      if(a[i]){
        bp2 = bread(ip->dev, a[i]);
	b = (uint *)bp2->data;
	for (j = 0; j < EXT2_INDIRECT; j++){
	  if (b[j]){
	    bp3 = bread(ip->dev, b[j]);
	    c = (uint *)bp3->data;
	    for (k = 0; k < EXT2_INDIRECT; k++){
	      if (c[k]){
	        ext2fs_bfree(ip->dev, c[k]);
		c[k] = 0;
	      }
	    }
	    brelse(bp3);
	    ext2fs_bfree(ip->dev, b[j]);
	    b[j] = 0;
	  }
	}
	brelse(bp2);
	ext2fs_bfree(ip->dev, a[i]);
	a[i] = 0;
      }
    }
    brelse(bp1);
    ext2fs_bfree(ip->dev, ad->addrs[EXT2_TIND_BLOCK]);
    ad->addrs[EXT2_TIND_BLOCK] = 0;
  }

  ip->size = 0;
  ip->iops->iupdate(ip);
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

  for(tot = 0; tot < n; tot += m, off += m, dst += m){
    bp = bread(ip->dev, ext2fs_bmap(ip, off / EXT2_BSIZE));
    m = min(n - tot, EXT2_BSIZE - off % EXT2_BSIZE);
    memmove(dst, bp->data + off % EXT2_BSIZE, m);
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
  if(off + n > EXT2_MAXFILE*EXT2_BSIZE)
    return -1;

  for(tot=0; tot<n; tot+=m, off+=m, src+=m){
    bp = bread(ip->dev, ext2fs_bmap(ip, off / EXT2_BSIZE));
    m = min(n - tot, EXT2_BSIZE - off%EXT2_BSIZE);
    memmove(bp->data + off%EXT2_BSIZE, src, m);
    bwrite(bp);
    brelse(bp);
  }

  if(n > 0 && off > ip->size){
    ip->size = off;
    ip->iops->iupdate(ip);
  }
  return n;
}

int
ext2fs_namecmp(const char *s, const char *t)
{
  return strncmp(s, t, EXT2_NAME_LEN);
}

struct inode*
ext2fs_dirlookup(struct inode *dp, char *name, uint *poff)
{
  uint off;
  struct ext2_dir_entry_2 de;
  char file_name[EXT2_NAME_LEN + 1];
  for (off = 0; off < dp->size; off += de.rec_len){
    if (dp->iops->readi(dp, (char *)&de, off,sizeof(de)) != sizeof(de))
      panic("ext2fs_dirlookup: read error");
    if (de.inode == 0)
      continue;
    strncpy(file_name, de.name, de.name_len);
    file_name[de.name_len] = '\0';
    if (ext2fs_namecmp(name, file_name) == 0){
      if (poff)
        *poff = off;
      return iget(dp->dev, de.inode);
    }
  }
  return 0;
}

int
ext2fs_dirlink(struct inode *dp, char *name, uint inum)
{
  int off;
  struct ext2_dir_entry_2 de;
  struct inode *ip;

  if((ip = dp->iops->dirlookup(dp, name, 0)) != 0){
    ip->iops->iput(ip);
    return -1;
  }

  for (off = 0; off < dp->size; off += de.rec_len){
    if (dp->iops->readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      panic("ext2_dirlink read 1");

    if (de.rec_len > sizeof(de) && de.rec_len == EXT2_BSIZE - (off % EXT2_BSIZE)){
      de.rec_len = sizeof(de) - EXT2_NAME_LEN + de.name_len;
      if (dp->iops->writei(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
        panic("ext2_dirlink write");

      off += de.rec_len;
      if(dp->iops->readi(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
        panic("ext2_dirlink read 2");
      break;
    }
  }

  if (off == dp->size){
    strncpy(de.name, name, EXT2_NAME_LEN);
    de.name_len = strlen(de.name);
    de.inode = inum;
    de.rec_len = EXT2_BSIZE;
    dp->size = off + de.rec_len;
    dp->iops->iupdate(dp);
    if (dp->iops->writei(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
      panic("ext2_dirlink write");
    return 0;
  }

  strncpy(de.name, name, EXT2_NAME_LEN);
  de.inode = inum;
  de.name_len = strlen(de.name);
  de.rec_len = EXT2_BSIZE - off % EXT2_BSIZE;
  dp->size = off + de.rec_len;
  dp->iops->iupdate(dp);
  if (dp->iops->writei(dp, (char *)&de, off, de.rec_len) != de.rec_len)
    panic("dirlink");
  return 0;
}

