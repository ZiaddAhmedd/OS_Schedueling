#include "headers.h"
#include <signal.h>

// Handlers
void schedulerHandler(int signum);
// void childHandler(int signum);
////////////

struct Queue Queue;
struct Queue FinishedQueue;
struct memTree *MemoryTree;

process *CurrentRunning = NULL;

// int time, nexttime;
int MemoryStart;
int TA;
int quantum;
float WTA;
FILE *fptr;
FILE *memfptr;
process data;
int allRunningTime = 0;
int TotalExecution = 0;
int recProcess = 0;
int finishedProcess = 0;
int TotalRunnung = 0;
int PidCount = 0;
int arrPids[100];
int TotalFinished1 = 0;
int TotalFinished2 = 0;
float TotalTA = 0;
float TotalWTA = 0;
int TotailWaiting = 0;
int main(int argc, char *argv[])
{
    initClk();
    MemoryTree = create_memTree();
    signal(SIGINT, schedulerHandler);
    // signal(SIGCHLD, childHandler);
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

    fptr = fopen("Schedular_HPF.log", "w"); // For Files
    memfptr = fopen("Memory_HPF.log", "w"); // For Files
    fprintf(fptr, "#At time x process y state arr w total z remain y wait k \n");
    fprintf(memfptr, "#At time x allocated y bytes for process z from i to j \n");
    fflush(fptr);

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
            if (rec_val == -1 && isEmpty_Queue(&Queue) == 1)
            {
                rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, !IPC_NOWAIT);

                // printf("process Recieved: %d  %d  %d  %d\n", msg.proc.processId, msg.proc.arrivalTime, msg.proc.runTime, msg.proc.priority);
            }
            if (rec_val != -1)
            {

                MemoryStart = allocateProcess(MemoryTree, msg.proc.memory, msg.proc.processId);
                msg.proc.mem_start = MemoryStart;
                enqueue(&Queue, msg.proc);
                int total_size = pow(2, ceil(log2(msg.proc.memory)));

                TotalExecution += msg.proc.executionTime;

                fprintf(memfptr, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), msg.proc.memory, msg.proc.processId, MemoryStart, MemoryStart + total_size);
                printf("process: %d recieved at: %d\n", msg.proc.processId, getClk());
                recProcess++;
            }
            // printf("CurrentTime: %d\n",getClk());

        } while (rec_val != -1);

        nextTime = getClk();
        if (nextTime >= time)
        {

            time = getClk();
            int pid, status;
        Here:
            if (CurrentRunning == NULL)
            {
                data = peek_Queue(&Queue);
                printf("I'm process %d\n Clock is %d\n", data.processId, getClk());

                dequeue(&Queue); // get the process and dequeue it
                CurrentRunning = &data;

                // printf("%d\n" ,CurrentRunning->state);
            }

            char buffer1[5];
            sprintf(buffer1, "%d", CurrentRunning->runTime); // pass the remianing time
            if (CurrentRunning->state == WAITING)
            {
                CurrentRunning->startTime = getClk(); // start time of the process added to the node
                int pid = fork();                     // fork the process
                CurrentRunning->state = RUNNING;
                if (pid == 0)
                {
                    char *ar[] = {"./process.out", buffer1, NULL, 0};
                    execve(ar[0], &ar[0], NULL);
                }
                else // it is the first time for the process to run
                {
                    CurrentRunning->waitingTime = CurrentRunning->startTime - CurrentRunning->arrivalTime; // get waiting time
                    // fptr = fopen("schedular.log", "a+");
                    fprintf(fptr, "At time  %d  process %d started arr %d total %d remian %d wait %d \n", getClk(), CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->executionTime, CurrentRunning->runTime, CurrentRunning->waitingTime);
                    fflush(fptr);
                    // fclose(fptr);
                    CurrentRunning->PID = pid;
                }
            }
            else if (CurrentRunning->state == PREEMPTED) // process is resumed
            {
                printf("Preempted time is %d\n", getClk());
                CurrentRunning->waitingTime = getClk() - CurrentRunning->contextSwitchTime + CurrentRunning->waitingTime;
                // fptr = fopen("schedular.log", "a+");
                fprintf(fptr, "At time  %d  process %d resumed arr %d total %d remian %d wait %d \n", getClk(), CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->executionTime, CurrentRunning->runTime, CurrentRunning->waitingTime);
                fflush(fptr);
                // fclose(fptr);
                CurrentRunning->startTime = getClk();
                CurrentRunning->state = RUNNING;
                printqueue(&Queue);
                kill(CurrentRunning->PID, SIGCONT); // continue the process
                printf("exectime: %d startTime: %d waitingTime %d RunnungTime %d\n", CurrentRunning->executionTime, CurrentRunning->startTime, CurrentRunning->waitingTime, CurrentRunning->runTime);
            }
            if (FinishedProcesses == processCount - 1)
            {
                if (CurrentRunning->state == RUNNING)
                {
                    sleep(CurrentRunning->runTime);
                    // printf("el7777777777777777777777,clk:%d\n", getClk());
                    FinishedProcesses++;
                    CurrentRunning->runTime = 0;
                    CurrentRunning->state = FINISHED;
                    TA = getClk() - CurrentRunning->arrivalTime;
                    WTA = (float)TA / CurrentRunning->executionTime;
                    TotalTA += TA;
                    TotalWTA += WTA;
                    TotailWaiting += CurrentRunning->waitingTime;
                    CurrentRunning->finishTime = getClk();
                    fprintf(fptr, "At time  %d  process %d finished arr %d total %d remian %d wait %d TA %d WTA %.2f \n", getClk(), CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->executionTime, CurrentRunning->runTime, CurrentRunning->waitingTime, TA, WTA);
                    fflush(fptr);
                    deallocateProcess(MemoryTree, CurrentRunning->processId);
                    int total_size = pow(2, ceil(log2(CurrentRunning->memory)));
                    fprintf(memfptr, "At time %d freed %d bytes for process %d from %d to %d\n", getClk(), CurrentRunning->memory, CurrentRunning->processId, CurrentRunning->mem_start, CurrentRunning->mem_start + total_size);
                    // calc utilisation
                    TotalFinished1 = getClk();
                    float AvgWTA = 0;
                    float AvgWaiting = 0;
                    // TotalFinished1 = (TotalFinished1 >= TotalFinished2) ? TotalFinished1 : TotalFinished2;
                    float Utilization = ((float)TotalExecution / (TotalFinished1)) * 100;
                    AvgWTA = TotalWTA / (float)processCount;
                    AvgWaiting = TotailWaiting / (float)processCount;
                    FILE *perfPtr;
                    perfPtr = fopen("Scheduler_HPF.perf", "w");
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
                    CurrentRunning = NULL;
                    killpg(getpid(), SIGKILL);
                    destroyClk(true);
                    msgctl(msgid, IPC_RMID, (struct msqid_ds *)0);
                    return 0;
                }
            }
            // printf("ana process 3\n");
            // printf("Clock is %d, Expected to finish is %d \n",getClk(),(CurrentRunning->startTime + CurrentRunning->executionTime + CurrentRunning->waitingTime));
            if (isEmpty_Queue(&Queue) == 0 && CurrentRunning->priority > peek_Queue(&Queue).priority)
            {

                CurrentRunning->contextSwitchTime = getClk();
                CurrentRunning->runTime -= (getClk() - CurrentRunning->startTime); // decrease the remaing time by quantum
                // fptr = fopen("schedular.log", "a+");
                if (CurrentRunning->runTime == 0)
                {
                    goto checkfinish;
                }
                kill(CurrentRunning->PID, SIGSTOP); // stop the process after the quantum
                fprintf(fptr, "At time  %d  process %d stopped arr %d total %d remain %d wait %d \n", CurrentRunning->contextSwitchTime, CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->executionTime, CurrentRunning->runTime, CurrentRunning->waitingTime);
                fflush(fptr);
                // fclose(fptr);
                printf("STOP\n");
                CurrentRunning->state = PREEMPTED;

                enqueue(&Queue, *CurrentRunning);
                CurrentRunning = NULL;
                goto Here;
            }
        checkfinish:
            if (getClk() >= (CurrentRunning->startTime + CurrentRunning->runTime))
            {
                printf("clock now yarab:%d, %d da el wait", getClk(), CurrentRunning->waitingTime);
                CurrentRunning->runTime = 0; // decrease the remaing time by quantum
                CurrentRunning->state = FINISHED;
                TA = getClk() - CurrentRunning->arrivalTime;
                WTA = (float)TA / CurrentRunning->executionTime;
                TotalTA += TA;
                TotalWTA += WTA;
                TotailWaiting += CurrentRunning->waitingTime;
                CurrentRunning->finishTime = getClk();
                // fptr = fopen("schedular.log", "a+");
                fprintf(fptr, "At time  %d  process %d finished arr %d total %d remian %d wait %d TA %d WTA %.2f \n", getClk(), CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->executionTime, CurrentRunning->runTime, CurrentRunning->waitingTime, TA, WTA);
                fflush(fptr);
                // fclose(fptr);
                FinishedProcesses++;
                // TotalRunnung += CurrentRunning->executionTime;
                // //enqueue(&FinishedQueue,*CurrentRunning );
                deallocateProcess(MemoryTree, CurrentRunning->processId);
                int total_size = pow(2, ceil(log2(CurrentRunning->memory)));
                fprintf(memfptr, "At time %d freed %d bytes for process %d from %d to %d\n", getClk(), CurrentRunning->memory, CurrentRunning->processId, CurrentRunning->mem_start, CurrentRunning->mem_start + total_size);
                CurrentRunning = NULL;
                printf("FINISHED\n");
            }
        }
    }
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