#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

char buf[2048];

void
writetest(void)
{
  int fd;

  printf(1,"Ext2 writetest\n");

  fd = open("/mnt/new", O_CREATE|O_RDWR);
  if (fd < 0){
    printf(1,"Open in ext2 failed!\n");
    exit();
  }
  write(fd,buf, 2048);
  printf(1,"write test passed\n");
}

int
main(void){
  writetest();
  exit();
}
