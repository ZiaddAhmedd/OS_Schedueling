#include "headers.h"
#include <signal.h>

// Handlers
void schedulerHandler(int signum);
////////////

struct Queue Queue;
struct Queue FinishedQueue;
process *CurrentRunning = NULL; 


// int time, nexttime;

int TA;
int quantum;
float WTA;
FILE *fptr;
process data;
int allRunningTime = 0;
int recProcess = 0;
int finishedProcess = 0;
int TotalRunnung = 0;
int PidCount = 0;
int arrPids [100];

int main(int argc, char *argv[])
{
    initClk();
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

    fptr = fopen("Schedular.log", "w"); // For Files
    fprintf(fptr, "#At time x process y state arr w total z remain y wait k \n");

    // For Stats

    if (msgid == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    struct msgbuffer msg;
    while (isEmpty_Queue(&Queue) == 0 || finishedProcess != processCount)
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
                process dataa = peek_Queue(&Queue);
                recProcess++;
                printf("process: %d recieved at: %d\n",msg.proc.processId,getClk());
                
               
              //  printf("Process recieved: %d %d %d %d\n", msg.proc.processId, msg.proc.arrivalTime, msg.proc.runTime, msg.proc.priority);
            }
            printf("CurrentTime: %d\n",getClk());

            
        } while (rec_val != -1);

        nextTime = getClk();
        if (nextTime >= time)
        {

            time = getClk();
            int pid, status;
            Here:
            if(CurrentRunning==NULL)
            {
                data = peek_Queue(&Queue);
                printf("I'm process %d\n Clock is %d\n",data.processId,getClk());
                
                dequeue(&Queue); // get the process and dequeue it
                CurrentRunning = &data;
                printf("%d\n" ,CurrentRunning->state);
            }

            char buffer1[5];
            sprintf(buffer1, "%d", CurrentRunning->runTime); // pass the remianing time
            
            if (CurrentRunning->state == WAITING)
            {
                CurrentRunning->startTime = getClk(); // start time of the process added to the node
                int pid = fork();         // fork the process
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
                    fprintf(fptr, "At time  %d  process %d started arr %d total %d remian %d wait %d \n", getClk(),CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->executionTime, CurrentRunning->runTime, CurrentRunning->waitingTime);
                    // fclose(fptr);
                    CurrentRunning->PID = pid;
                }
            }
            else if (CurrentRunning->state == PREEMPTED) // process is resumed
            {
              printf("Preempted time is %d\n",getClk());
                CurrentRunning->waitingTime = getClk() - CurrentRunning->contextSwitchTime + CurrentRunning->waitingTime;
                // fptr = fopen("schedular.log", "a+");
                fprintf(fptr, "At time  %d  process %d resumed arr %d total %d remian %d wait %d \n", getClk(),CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->executionTime, CurrentRunning->runTime, CurrentRunning->waitingTime);
                // fclose(fptr);
                //CurrentRunning->startTime=getClk();
                CurrentRunning->state = RUNNING;
                kill(CurrentRunning->PID , SIGCONT); // continue the process
                printf("exectime: %d startTime: %d waitingTime %d\n",CurrentRunning->executionTime,CurrentRunning->startTime,CurrentRunning->waitingTime);
            }
            if(isEmpty_Queue(&Queue)==0)
            {
              //printf("ana process 3\n");
              //printf("Clock is %d, Expected to finish is %d \n",getClk(),(CurrentRunning->startTime + CurrentRunning->executionTime + CurrentRunning->waitingTime));
              if (CurrentRunning->priority > peek_Queue(&Queue).priority)
              { 
                  kill(CurrentRunning->PID, SIGSTOP); // stop the process after the quantum
                  CurrentRunning->contextSwitchTime= getClk();
                  CurrentRunning->runTime =CurrentRunning->executionTime - CurrentRunning->contextSwitchTime + CurrentRunning->waitingTime +1;  // decrease the remaing time by quantum
                  // fptr = fopen("schedular.log", "a+");
                  fprintf(fptr, "At time  %d  process %d stopped arr %d total %d remain %d wait %d \n", CurrentRunning->contextSwitchTime, CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->executionTime, CurrentRunning->runTime, CurrentRunning->waitingTime);
                  // fclose(fptr);
                  printf("STOP\n");
                  CurrentRunning->state = PREEMPTED;
                  
                  enqueue(&Queue, *CurrentRunning);
                  CurrentRunning = NULL;
                  goto Here;
                  // printqueue(&Queue);
                  
              }
              else if (getClk()==(CurrentRunning->startTime + CurrentRunning->executionTime + CurrentRunning->waitingTime))
                {

                    CurrentRunning->runTime = 0; // decrease the remaing time by quantum
                    CurrentRunning->state = FINISHED;
                    TA = getClk() - CurrentRunning->arrivalTime;
                    WTA = (float)TA / CurrentRunning->executionTime;
                    CurrentRunning->finishTime = getClk();
                    // fptr = fopen("schedular.log", "a+");
                    fprintf(fptr, "At time  %d  process %d finished arr %d total %d remian %d wait %d TA %d WTA %.2f \n", getClk(),CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->executionTime, CurrentRunning->runTime, CurrentRunning->waitingTime, TA, WTA);
                    // fclose(fptr);
                    FinishedProcesses++;
                    TotalRunnung += CurrentRunning->executionTime;
                    // printqueue(&Queue);
                    enqueue(&FinishedQueue,*CurrentRunning );
                    CurrentRunning =NULL;
                    printf("FINISHED\n");
                }
          
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
