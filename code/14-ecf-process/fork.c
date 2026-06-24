#include "csapp.h"

int main()
{
    pid_t pid;
    int x = 1;

    pid = Fork();
    if (pid == 0)
    { // child
        printf("child: x=%d\n", ++x);
        exit(0);
    }

    // parent
    printf("parent: x=%d\n", --x);
    exit(0);
}

/* fork:
- call once, return twice
- concurrent execution of parent and child
- duplicate but separate address space:
  . x has value of 1 when fork returns in parent and child.
  . subsequent changes to x are independent.
- shared open files:
  . stdout is the same in both parent and child.
*/

/* run: 
./fork.exe
parent: x=0
child: x=2
*/