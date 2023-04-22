#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

char buf[2048];

void
opentest(void)
{
  int fd;
  fd = open("/mnt/file1", O_RDWR);
  if (fd < 0){
    printf(1, "cannot open /mnt/file1\n");
    exit();
  }
  close(fd);
  printf(1, "open test1 passed\n");

  fd = open("/mnt/file2", O_RDWR);
  if (fd < 0){
    printf(1, "cannot open /mnt/file2\n");
    exit();
  }
  close(fd);
  printf(1, "open test2 passed\n");

  fd = open("/mnt/file3", O_RDWR);
  if (fd < 0){
    printf(1, "cannot open /mnt/file3\n");
    exit();
  }
  close(fd);
  printf(1, "open test3 passed\n");

  printf(1, "All open test passed succesfully\n");
}

void
writetest(void)
{
  int fd, i;

  printf(1,"ext2 writetest\n");

  fd = open("/mnt/file1", O_CREATE|O_RDWR);
  if (fd < 0){
    printf(1,"Open in ext2 failed!\n");
    exit();
  }
  for (i = 0; i < 100; i++){
    write(fd, "aaaaaaaaaa", 10);
  }
  close(fd);
  printf(1,"Write test passed\n");
}

void
balloctest(void)
{
  int fd, i;

  printf(1,"ext2 balloc test\n");

  fd = open("/mnt/file3", O_CREATE|O_RDWR);
  if (fd < 0){
    printf(1, "open in ext2 failed\n");
    exit();
  }
  for (i=0; i < 1000; i++){
    write(fd, "aaaaaaaaaa", 10);
  }
  close(fd);
  printf(1, "balloc test passed\n");
}

void
createtest(void)
{
  int fd;

  fd = open("/mnt/new", O_CREATE);
  close(fd);
}

void
dirlookuptest(void)
{
  int fd;

  printf(1, "ext2 dirlookup test\n");
  fd = open("/mnt/dir1/dir2/dir3/file", O_RDWR);

  if (fd < 0){
    printf(1, "open in ext2 failed");
    exit();
  }
  close(fd);
  printf(1, "dirlookuptest passed\n");
}

int
main(void)
{
  //createtest();
  opentest();
  writetest();
  balloctest();
  dirlookuptest();
  exit();
}
