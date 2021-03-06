#include "headers.h"

// Handlers
void schedulerHandler(int signum);
void childHandler(int signum);
////////////

struct Queue Queue;
struct memTree *MemoryTree;
int MemoryStart;
process *CurrentProcess = NULL;
int TA;
float WTA;
FILE *fptr;
FILE *memfptr;
process data;
int allRunningTime = 0;
int recProcess = 0;
int finishedProcess = 0;
int processCount = 0;
int msgid;
float AvgWTA = 0;
float AvgWaiting = 0;

int main(int argc, char *argv[])
{
    initClk();
    MemoryTree = create_memTree();
    signal(SIGINT, schedulerHandler);
    signal(SIGCHLD, childHandler);
    Queue = createQueue();
    // RunningQueue = createQueue();
    // FinishedQueue = createQueue();

    int rec_val;
    key_t key_id;
    processCount = atoi(argv[1]);
    key_id = ftok("./clk.c", 'Z');            // create unique key
    msgid = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id

    int time, nextTime;
    time = getClk();

    fptr = fopen("Scheduler_SJF.log", "w"); // For Files
    memfptr = fopen("Memory_SJF.log", "w"); // For Files
    fprintf(fptr, "#At time x process y state arr w total z remain y wait k \n");
    fprintf(memfptr, "#At time x allocated y bytes for process z from i to j \n");

    // For Stats

    if (msgid == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    printf("Message Queue ID = %d\n", msgid);

    struct msgbuffer msg;
    printf("%d\n", processCount);
    while (finishedProcess != processCount)
    {
        do
        {
            /* receive all types of messages */
            rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, IPC_NOWAIT); // shouldn't wait for msg
            if (rec_val == -1 && isEmpty_Queue(&Queue) == 1)
            {
                rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, !IPC_NOWAIT);
            }
            if (rec_val != -1)
            {
                MemoryStart = allocateProcess(MemoryTree, msg.proc.memory, msg.proc.processId);
                msg.proc.mem_start = MemoryStart;
                enqueue(&Queue, msg.proc);
                int total_size = pow(2, ceil(log2(msg.proc.memory)));
                fprintf(memfptr, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), msg.proc.memory, msg.proc.processId, MemoryStart, MemoryStart + total_size);
            }
        } while (rec_val != -1);

        nextTime = getClk();
        if (nextTime >= time)
        {
            int pid, status;

            if (CurrentProcess == NULL && isEmpty_Queue(&Queue) == 0)
            {
                data = peek_Queue(&Queue);
                CurrentProcess = &data;
                printf("I am process %d\n", CurrentProcess->processId);
                CurrentProcess->state = RUNNING;
                dequeue(&Queue);
                printf("\nProcess Running: %d %d %d %d\n", CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->priority);
                CurrentProcess->startTime = getClk();
                CurrentProcess->waitingTime = getClk() - CurrentProcess->arrivalTime;
                pid = fork();
                if (pid == 0)
                {
                    char buffer[20];
                    char *sendToProcess[2];
                    sprintf(buffer, "%d", CurrentProcess->runTime);
                    char *argv[] = {"./process.out", buffer, NULL, 0};
                    execve(argv[0], &argv[0], NULL);
                }
                else
                {
                    fprintf(fptr, "At time %d process %d started arr %d total %d remain %d wait %d \n", CurrentProcess->startTime, CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->runTime, CurrentProcess->waitingTime);
                }
            }
        }
    }
    float Utilization = ((float)allRunningTime / (getClk())) * 100;

    AvgWTA = AvgWTA / (float)processCount;
    AvgWaiting = AvgWaiting / (float)processCount;
    FILE *perfPtr;
    perfPtr = fopen("Scheduler_SJF.perf", "w");
    if (!perfPtr)
    {
        printf("Error in opening file\n");
    }
    else
    {
        fprintf(perfPtr, "CPU utilization = %0.2f %% \n", Utilization);
        fprintf(perfPtr, "Avg WTA = %0.2f\n", AvgWTA);
        fprintf(perfPtr, "Avg Waiting = %0.2f\n", AvgWaiting);
    }
    fclose(perfPtr);
    fclose(memfptr);
    destroyClk(true);
    msgctl(msgid, IPC_RMID, (struct msqid_ds *)0);
    return 0;
}

void schedulerHandler(int signum)
{
    printf("The Scheduler has stopped\n");
    msgctl(msgid, IPC_RMID, NULL);
    destroyClk(true);
    exit(0);
}

void childHandler(int signum)
{
    printf("clk: %d start: %d run: %d\n", getClk(), CurrentProcess->startTime, CurrentProcess->runTime);
    // printf("\nProcess finished: %d %d %d %d\n", CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->priority);
    finishedProcess++;
    allRunningTime += getClk() - CurrentProcess->startTime;
    CurrentProcess->finishTime = getClk();
    TA = CurrentProcess->finishTime - CurrentProcess->arrivalTime;
    WTA = (float)TA / (float)CurrentProcess->runTime;
    AvgWaiting += CurrentProcess->startTime - CurrentProcess->arrivalTime;
    AvgWTA += WTA;
    CurrentProcess->state = FINISHED;
    fprintf(fptr, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f \n", CurrentProcess->finishTime, CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, 0, CurrentProcess->waitingTime, TA, WTA);

    deallocateProcess(MemoryTree, CurrentProcess->processId);
    int total_size = pow(2, ceil(log2(CurrentProcess->memory)));
    fprintf(memfptr, "At time %d freed %d bytes for process %d from %d to %d\n", getClk(), CurrentProcess->memory, CurrentProcess->processId, CurrentProcess->mem_start, CurrentProcess->mem_start + total_size);

    CurrentProcess = NULL;
    signal(SIGCHLD, childHandler);
}
