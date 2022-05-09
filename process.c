#include "headers.h"

void handle_new_proc_arrival(int);

int main(int agrc, char *argv[])
{
    initClk();
    signal(SIGINT, handle_new_proc_arrival);

    // get clk, running proc from shared memory
    const int start_time = getClk();

    //TODO it needs to get the remaining time from somewhere
    int remainingtime = atoi(argv[1]);
    pid_t scheduler_pid = atoi(argv[2]);
    ALGORITHIM_TYPE sched_type = atoi(argv[3]);
    int pid = atoi(argv[4]);

    int i = 0;
    while (remainingtime > 0)
    {

        
        int current_time = getClk();

        int time_elapsed = current_time - start_time;

        if (time_elapsed == remainingtime)
        {
            
            break;
        }
        else if (time_elapsed > remainingtime)
        {
            // log error
            break;
        }
    }

    destroyClk(false);
    return pid;
}

void handle_new_proc_arrival(int signum)
{
    char log_message2[100];
    sprintf(log_message2, "recieved interrupt from scheduler at %d ", getClk());
    destroyClk(false);
    signal(SIGUSR1, handle_new_proc_arrival);
    exit(0);
}