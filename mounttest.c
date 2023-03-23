#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char* argv[]){
   mount("/mnt", 2);
   exit();
}
