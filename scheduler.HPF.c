#include "headers.h"

int main()
{
    struct Queue *HPF_Queue;
    struct Queue *HPF_Queue_Finished;
    HPF_Queue = createQueue();
    HPF_Queue_Finished = createQueue();
    int total_processes = 0;
    int rec_val = 0;
    int processRunning = 0;
    while (rec_val != -1) // meaning message queue is empty
    {
        struct msgbuffer msg;
        rec_val = msgrcv(msgid, (struct msgbuffer *)&msg, sizeof(msg.proc), 0, IPC_NOWAIT);
        

        if (rec_val != -1) // there's something to recover from queue
        {
            // add process to queue
            struct process proc = msg.proc;
            printf("%d", proc.priority);
            enqueue(&HPF_Queue, proc);
            total_processes++;
        }
    }

    if (processRunning == 0)
    {
        schedule_process();
        // here no need to check if scheduling failed due to empty queue since i just inserted now
        processRunning = 1;
    }
    return 0;
}