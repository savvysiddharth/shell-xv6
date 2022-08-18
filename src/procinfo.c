#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(int argc, char *argv[])
{
    if(argc == 2) {
      int pid = atoi(argv[1]);
      printf(1,"pid= %d\n", pid);
      printf(1, "Number of files opened: %d\n", numOpenFiles(pid));
      printf(1,"Memory allocated: %d bytes\n", memAlloc(pid));
      getprocesstimedetails(pid);
      exit(0);
    } else {
      printf(1, "This program must contain exactly 1 argument.\n");
      exit(-1);
    }
}
