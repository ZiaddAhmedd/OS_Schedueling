#include "headers.h"
#include <time.h>

int remainingTime;

int main(int agrc, char *argv[])
{
    // initClk();
    remainingTime = atoi(argv[1]);
    printf("The running time is %d\n", remainingTime);
    while ((clock() + 100000) < remainingTime * CLOCKS_PER_SEC);
    return 0;
}
