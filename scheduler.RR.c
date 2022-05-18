#include "headers.h"
#include <signal.h>

// Handlers
void schedulerHandler(int signum);
////////////

struct Queue Queue;
struct Queue FinishedQueue;
struct Queue RunningQueue;
struct memTree *MemoryTree;
process *CurrentRunning = NULL;

// int time, nexttime;
int TotailWaiting = 0;
int MemoryStart;
int TA;
int totalProcessor = 0;
int quantum;
float WTA;
float TotalTA = 0;
float TotalWTA = 0;
FILE *fptr;
FILE *memfptr;
process data;
int allRunningTime = 0;
int recProcess = 0;
int finishedProcess = 0;
int TotalRunning = 0;
int TotalExecution = 0;
int TotalFinished1 = 0;
int TotalFinished2 = 0;
int PidCount = 0;
int arrPids[100];

int main(int argc, char *argv[])
{
    initClk();
    MemoryTree = create_memTree();
    signal(SIGINT, schedulerHandler);
    Queue = createQueue();
    FinishedQueue = createQueue();
    int FinishedProcesses = 0;

    int rec_val, msgid;
    key_t key_id;
    int processCount = atoi(argv[1]);

    quantum = atoi(argv[2]);
    key_id = ftok("./clk.c", 'Z');            // create unique key
    msgid = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id

    int time, nextTime;
    time = getClk();

    fptr = fopen("Scheduler_RR.log", "w"); // For Files
    memfptr = fopen("Memory_RR.log", "w"); // For Files
    fprintf(fptr, "#At time x process y state arr w total z remain y wait k \n");
    fprintf(memfptr, "#At time x allocated y bytes for process z from i to j \n");

    // For Stats

    if (msgid == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    struct msgbuffer msg;
    while (FinishedProcesses < processCount)
    {
        do
        {

            /* receive all types of messages */
            rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, IPC_NOWAIT); // shouldn't wait for msg
            if (rec_val == -1 && isEmpty_Queue(&Queue) == 1 && isEmpty_Queue(&RunningQueue) == 1)
            {
                printf("YARAAABBB\n");
                rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, !IPC_NOWAIT);

                // printf("process Recieved: %d  %d  %d  %d\n", msg.proc.processId, msg.proc.arrivalTime, msg.proc.runTime, msg.proc.priority);
            }
            if (rec_val != -1)
            {

                MemoryStart = allocateProcess(MemoryTree, msg.proc.memory, msg.proc.processId);
                msg.proc.mem_start = MemoryStart;
                enqueue(&Queue, msg.proc);

                TotalExecution += msg.proc.executionTime;
                int total_size = pow(2, ceil(log2(msg.proc.memory)));
                fprintf(memfptr, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), msg.proc.memory, msg.proc.processId, MemoryStart, MemoryStart + total_size);
                printf("process: %d recieved at: %d\n", msg.proc.processId, getClk());
                recProcess++;
            }

        } while (rec_val != -1);

        nextTime = getClk();
        if (nextTime >= time)
        {

            time = getClk();
            int pid, status;

            if (CurrentRunning == NULL)
            {
                printf("Time is: %d\n", getClk());
                data = peek_Queue(&Queue);

                dequeue(&Queue); // get the process and dequeue it
                CurrentRunning = &data;
                printf("I'm process %d \n", CurrentRunning->processId);
                printf("State is %d, remaining time is %d \n", CurrentRunning->state, CurrentRunning->runTime);
            }

            char buffer1[5];
            sprintf(buffer1, "%d", CurrentRunning->runTime); // pass the remaining time

            if (CurrentRunning->state == WAITING)
            {
                CurrentRunning->startTime = getClk(); // start time of the process added to the node
                int pid = fork();                     // fork the process
                CurrentRunning->state = RUNNING;
                enqueue(&RunningQueue, *CurrentRunning);
                if (pid == 0)
                {
                    char *ar[] = {"./process.out", buffer1, NULL, 0};
                    execve(ar[0], &ar[0], NULL);
                }
                else // it is the first time for the process to run
                {
                    CurrentRunning->waitingTime = CurrentRunning->startTime - CurrentRunning->arrivalTime; // get waiting time
                    // fptr = fopen("schedular.log", "a+");
                    fprintf(fptr, "At time  %d  process %d started arr %d total %d remain %d wait %d \n", getClk(), CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->executionTime, CurrentRunning->runTime, CurrentRunning->waitingTime);
                    // fclose(fptr);
                    arrPids[PidCount] = pid;
                    PidCount++;
                }
            }
            else if (CurrentRunning->state == PREEMPTED) // process is resumed
            {
                CurrentRunning->waitingTime = getClk() - CurrentRunning->contextSwitchTime + CurrentRunning->waitingTime;
                // fptr = fopen("schedular.log", "a+");
                fprintf(fptr, "At time  %d  process %d resumed arr %d total %d remain %d wait %d \n", getClk(), CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->executionTime, CurrentRunning->runTime, CurrentRunning->waitingTime);
                // fclose(fptr);
                CurrentRunning->startTime = getClk();
                printf("Start time is %d\n", CurrentRunning->startTime);
                CurrentRunning->state = RUNNING;
                enqueue(&RunningQueue, *CurrentRunning);
                kill(arrPids[CurrentRunning->processId - 1], SIGCONT); // continue the process
            }

            if (CurrentRunning->runTime > quantum)
            {
                // printf("process %d bigger qunatum\n", p->process_id);
                // printf(" running time process %d", p->running_time);
                // printf("process %d resumed\n", data.processId);
                if (FinishedProcesses == processCount)
                {
                    // printf("process id %d is done", p->process_id);
                    CurrentRunning->state = FINISHED;
                    TotalFinished1 = getClk();
                    dequeue(&RunningQueue);

                    deallocateProcess(MemoryTree, CurrentRunning->processId);
                    int total_size = pow(2, ceil(log2(CurrentRunning->memory)));
                    fprintf(memfptr, "At time %d freed %d bytes for process %d from %d to %d\n", getClk(), CurrentRunning->memory, CurrentRunning->processId, CurrentRunning->mem_start, CurrentRunning->mem_start + total_size);

                    TA = getClk() - CurrentRunning->arrivalTime + CurrentRunning->runTime - 1;
                    WTA = (float)TA / CurrentRunning->executionTime;
                    TotalTA += TA;
                    TotalWTA += WTA;
                    TotailWaiting += CurrentRunning->waitingTime;
                    CurrentRunning->finishTime = getClk() + CurrentRunning->runTime - 1;
                    // fptr = fopen("schedular.log", "a+");
                    totalProcessor = getClk() + CurrentRunning->runTime - 1;
                    fprintf(fptr, "At time  %d  process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f \n", getClk() + CurrentRunning->runTime - 1, CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->executionTime, CurrentRunning->runTime, CurrentRunning->waitingTime, TA, WTA);
                    CurrentRunning->runTime = 0; // decrease the remaing time by quantum
                    // fclose(fptr);
                    FinishedProcesses++;

                    if (!CurrentRunning)
                    {
                        enqueue(&FinishedQueue, *CurrentRunning);
                    }
                    if (FinishedProcesses != processCount)
                    {
                        CurrentRunning = NULL;
                    }
                }
                else if (getClk() - CurrentRunning->startTime == quantum)
                {
                    kill(arrPids[data.processId - 1], SIGSTOP); // stop the process after the quantum
                    CurrentRunning->contextSwitchTime = getClk();
                    CurrentRunning->runTime -= quantum; // decrease the remaing time by quantum
                    // fptr = fopen("schedular.log", "a+");
                    fprintf(fptr, "At time  %d  process %d stopped arr %d total %d remain %d wait %d \n", CurrentRunning->contextSwitchTime, CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->executionTime, CurrentRunning->runTime, CurrentRunning->waitingTime);
                    // fclose(fptr);
                    printf("STOP\n");
                    CurrentRunning->state = PREEMPTED;
                    dequeue(&RunningQueue);

                    enqueue(&Queue, *CurrentRunning);
                    CurrentRunning = NULL;
                    printqueue(&Queue);
                }
            }
            else if (CurrentRunning->runTime <= quantum)
            {

                if ((getClk() - CurrentRunning->startTime) == CurrentRunning->runTime)
                {
                    printf("process %d finished at %d\n", CurrentRunning->processId, getClk());
                    CurrentRunning->runTime = 0; // decrease the remaing time by quantum
                    CurrentRunning->state = FINISHED;
                    TotalFinished2 = getClk();
                    dequeue(&RunningQueue);

                    deallocateProcess(MemoryTree, CurrentRunning->processId);
                    int total_size = pow(2, ceil(log2(CurrentRunning->memory)));
                    fprintf(memfptr, "At time %d freed %d bytes for process %d from %d to %d\n", getClk(), CurrentRunning->memory, CurrentRunning->processId, CurrentRunning->mem_start, CurrentRunning->mem_start + total_size);
                    TA = getClk() - CurrentRunning->arrivalTime;
                    WTA = (float)TA / CurrentRunning->executionTime;
                    TotalTA += TA;
                    TotalWTA += WTA;
                    TotailWaiting += CurrentRunning->waitingTime;
                    CurrentRunning->finishTime = getClk();
                    // fptr = fopen("schedular.log", "a+");
                    fprintf(fptr, "At time  %d  process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f \n", getClk(), CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->executionTime, CurrentRunning->runTime, CurrentRunning->waitingTime, TA, WTA);
                    // fclose(fptr);
                    FinishedProcesses++;
                    // printqueue(&Queue);
                    if (!CurrentRunning)
                    {
                        enqueue(&FinishedQueue, *CurrentRunning);
                    }
                    if (FinishedProcesses != processCount)
                    {

                        CurrentRunning = NULL;
                    }
                }
            }
        }
    }

    float AvgWTA = 0;
    float AvgWaiting = 0;
    TotalFinished1 = (TotalFinished1 >= TotalFinished2) ? TotalFinished1 : TotalFinished2;
    float Utilization = ((float)TotalExecution / (TotalFinished1)) * 100;
    AvgWTA = TotalWTA / (float)processCount;
    AvgWaiting = TotailWaiting / (float)processCount;
    FILE *perfPtr;
    perfPtr = fopen("Scheduler_RR.perf", "w");
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