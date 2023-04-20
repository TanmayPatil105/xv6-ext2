#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

char buf[2048];

void
readtest(void)
{
  int fd, n;
  fd = open("/mnt/file1", O_RDWR);
  if (fd < 0){
    printf(1, "cannot open /mnt/file1\n");
    exit();
  }
  while(( n = read(fd, buf, sizeof(buf))) > 0){
    write(1, buf, n);
  }
  printf(1, "read test1 passed\n");
  fd = open("/mnt/file2", O_RDWR);
  if (fd < 0){
    printf(1, "cannot open /mnt/file2\n");
    exit();
  }
  while(( n = read(fd, buf, sizeof(buf))) > 0){
    write(1, buf, n);
  }
  printf(1, "read test2 passed\n");
  printf(1, "All read test passed succesfully\n");
}

void
writetest(void)
{
  int fd, i;

  printf(1,"Ext2 writetest\n");

  fd = open("/mnt/file1", O_CREATE|O_RDWR);
  if (fd < 0){
    printf(1,"Open in ext2 failed!\n");
    exit();
  }
  for (i = 0; i < 100; i++){
    write(fd, "aaaaaaaaaa", 10);
  }
  printf(1,"Write test passed\n");
}

void
balloctest(void)
{
  int fd, i;

  printf(1,"balloc test\n");

  fd = open("/mnt/file3", O_CREATE|O_RDWR);
  if (fd < 0){
    printf(1, "open in ext2 failed\n");
    exit();
  }
  for (i=0; i < 1000; i++){
    write(fd, "aaaaaaaaaa", 10);
  }
  printf(1, "Balloc test passed\n");
}

int
main(void)
{
  readtest();
  writetest();
  //balloctest();
  exit();
}
