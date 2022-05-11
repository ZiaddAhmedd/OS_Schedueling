#include "headers.h"

// Handlers
void schedulerHandler(int signum);
void childHandler(int signum);
////////////

struct Queue Queue;
struct Queue FinishedQueue;

// int time, nexttime;

int TA;
float WTA;
FILE *fptr;
process data;
int allRunningTime = 0;
int recProcess = 0;
int finishedProcess = 0;
int main(int argc, char *argv[])
{
    initClk();
    signal(SIGINT, schedulerHandler);
    signal(SIGCHLD, childHandler);
    Queue = createQueue();
    FinishedQueue = createQueue();

    int rec_val, msgid;
    key_t key_id;
    int processCount = atoi(argv[1]);
    key_id = ftok("./clk.c", 'Z');            // create unique key
    msgid = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id

    int time, nextTime;
    time = getClk();

    fptr = fopen("Schedular.log", "w"); // For Files
    fprintf(fptr, "#At time x process y state arr w total z remain y wait k \n");

    // For Stats

    if (msgid == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    printf("Message Queue ID = %d\n", msgid);

    struct msgbuffer msg;
    printf("%d\n", processCount);
    while (isEmpty_Queue(&Queue) == 0 || recProcess < processCount)
    {
        do
        {
            /* receive all types of messages */
            rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, IPC_NOWAIT); // shouldn't wait for msg
            if (rec_val == -1 && isEmpty_Queue(&Queue) == 1)
            {
                rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, !IPC_NOWAIT);
                // printf("process Recieved: %d  %d  %d  %d\n", msg.proc.processId, msg.proc.arrivalTime, msg.proc.runTime, msg.proc.priority);
            }
            if (rec_val != -1)
            {
                enqueue(&Queue, msg.proc);
                process data = peek_Queue(&Queue);
                // printf("\nProcess recieved: %d %d %d %d\n", data.processId, data.arrivalTime, data.runTime, data.priority);
            }

            recProcess++;
        } while (rec_val != -1);

        nextTime = getClk();
        if (nextTime > time)
        {
            int pid, status;
            data = peek_Queue(&Queue);
            dequeue(&Queue);
            pid = fork();
            data.startTime = getClk();
            data.waitingTime = getClk() - data.arrivalTime;
            if (pid == 0)
            {
                char buffer[20];
                char *sendToProcess[2];
                sprintf(buffer, "%d", data.runTime);
                char *argv[] = {"./process.out", buffer, NULL, 0};
                execve(argv[0], &argv[0], NULL);
            }
            pid = wait(&status);
            if (WIFEXITED(status))
            {
                fprintf(fptr, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f \n", data.finishTime, data.processId, data.arrivalTime, data.runTime, 0, data.waitingTime, TA, WTA);
            }
        }
    }
    float Utilization = ((float)allRunningTime / getClk()) * 100;
    printf("%d\n",getClk());
    float AvgWTA = 0;
    float AvgWaiting = 0;
    for (int i = 0; i < processCount; i++)
    {
        process done = peek_Queue(&FinishedQueue);
        dequeue(&FinishedQueue);
        AvgWTA += (float)(done.finishTime - done.arrivalTime) / done.runTime;
        AvgWaiting += done.waitingTime;
    }
    AvgWTA = AvgWTA / (float)processCount;
    AvgWaiting = AvgWaiting / (float)processCount;
    FILE *perfPtr;
    perfPtr = fopen("Scheduler.perf", "w");
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

    destroyClk(true);
    msgctl(msgid, IPC_RMID, (struct msqid_ds *)0);
    return 0;
}

void schedulerHandler(int signum)
{
    printf("The Scheduler has stopped\n");
    destroyClk(true);
    exit(0);
}

void childHandler(int signum)
{
    finishedProcess++;
    allRunningTime += getClk() - data.startTime;
    data.finishTime = getClk();
    TA = data.finishTime - data.arrivalTime;
    WTA = (float)TA / (float)data.runTime;
    fprintf(fptr, "At time %d process %d started arr %d total %d remain %d wait %d \n", data.startTime, data.processId, data.arrivalTime, data.runTime, data.runTime, data.waitingTime);

    enqueue(&FinishedQueue, data);
}