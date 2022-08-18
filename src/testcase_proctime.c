#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "date.h"

int main(void)
{
    
    // int cpid = fork();
    // if(cpid == 0) {
    //     getprocesstimedetails(0);
    //     sleep(100);
    //     exit(0);
    // }
    // wait(0);
    getprocesstimedetails(2);
    exit(0);
    return 0;
}
