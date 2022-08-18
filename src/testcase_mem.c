#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"


int main(int argc, char *argv[])
{
  printf(1,"Memory allocated till now: %d bytes\n", memAlloc(0));
  sleep(10);

  char* x = sbrk(1024);
  printf(1,"Memory allocated till now: %d bytes\n", memAlloc(0));
  
  //free(x);
  char* y = sbrk(4096);
  printf(1,"Memory allocated till now: %d bytes\n", memAlloc(0));


  printf(1,"Memory allocated till now: %d bytes\n", memAlloc(0));
  

  char* z = sbrk(-10);
  printf(1,"Memory allocated till now: %d bytes\n", memAlloc(0));

  char* a = sbrk(-20000);
  printf(1,"Memory allocated till now: %d bytes\n", memAlloc(0));



  free(x);
  free(y);
  free(z);  
  free(a);
  exit(0);
}