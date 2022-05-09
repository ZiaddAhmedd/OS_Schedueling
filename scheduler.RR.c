#include "headers.h"

int start_time;
void sig_child_handler(int);
void sig_processGen_finish(int);
void sig_processGen_handler(int);
void sig_int_handler(int);

int quantum;
int msgid;
int isFinished_ProcGen = 0;
struct CircularQueue myQueue;

int last_run_time = 0, total_processes = 0;
float Ex = 0, Ex2 = 0; // used to calculate std dev of WTA
                       // using the formula: Ex = E[x^2] - E[x]^2
float total_wait_time = 0;
float total_useful_time = 1; // because we start from t = 1;

process currentlyProcessing; // will hold processes currently processing

int processRunning; // 0 if no process currently running
                    // 1 if there is currently a process running

int main(int argc, char *argv[])
{

    signal(SIGUSR1, sig_processGen_handler);
    signal(SIGUSR2, sig_processGen_finish);

    quantum = atoi(argv[1]);
    myQueue = initializeQueue();
    key_t key = ftok("./clk.c", 'a');
    msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1)
    {

        perror("msgget");
        return 1;
    }

    initClk();

    signal(SIGCHLD, sig_child_handler);

    // handling signal sent by process generator made it SIGCONT(and sigcont sent by child will be handled by SIGCHILD)

    signal(SIGINT, sig_int_handler);

    processRunning = 0; // initially no process is running

    // init message queue

    while (1)
    {
        // parent code
        pause();
    }
}

// return 1 if scheduling process succeeded; 0 if no process in queue so failed
int schedule_process()
{
    process picked_proc = peek_queue(&myQueue);

    if (picked_proc.arrivalTime != -1)
    {
        // Queue is not empty
        // After the schedule picks the process
        // it will be executed

        // editing before entering fork !!
        int timeForProcess = (quantum < picked_proc.remainingTime) ? quantum : picked_proc.remainingTime;

        // start
        int wait_time = 0;
        char resumed_started[100];
        if (picked_proc.state == WAITING)
        {

            wait_time = getClk() - picked_proc.arrivalTime;

            initiate_process(&picked_proc, wait_time, &total_wait_time);
            strcpy(resumed_started, "started");
            int total_size = pow(2, ceil(log2(picked_proc.size)));
            char log_message2[100];
            
        }
        else if (picked_proc.state == PAUSED)
        {
            wait_time = (getClk() - picked_proc.finishTime);
            strcpy(resumed_started, "resumed");

            initiate_process(&picked_proc, wait_time, &total_wait_time);
        }

        char log_message[100];
        //sprintf(log_message, "At time %d process %d %s arr %d total %d remain %d wait %d", picked_proc.startTime, picked_proc.processId, resumed_started, picked_proc.arrivalTime, picked_proc.runTime, picked_proc.remainingTime, picked_proc.waitingTime);
        //write_to_file(log_file, log_message);
        // end

        currentlyProcessing = copyProcess(picked_proc);
        pop_queue(&myQueue);
        currentlyProcessing.remainingTime -= timeForProcess;
        last_run_time = getClk();
        int pid = fork();

        if (pid == -1)
            perror("error in fork");

        else if (pid == 0)
        {
            // child code
            // add parameters to child process
            char scheduler_id[15];
            char scheduler_name[5];
            char remaining_time[15];
            char process_id[15];
            sprintf(process_id, "%d", picked_proc.processId);
            sprintf(scheduler_id, "%d", getppid());
            sprintf(scheduler_name, "%d", RR);
            sprintf(remaining_time, "%d", timeForProcess);
            // decreasing remaining time of process which will enter by either quantum or (its life time if smaller)
            char *argv[] = {"./process.out", remaining_time, scheduler_id, scheduler_name, process_id, 0};
            execve(argv[0], &argv[0], NULL);
        }
        return 1;
    }
    return 0;
}

void sig_int_handler(int signum)
{
    // scheduler cleanup
    print_statistics(Ex, Ex2, total_processes, total_wait_time, total_useful_time);
    destroyClk(false);

    exit(0);
}

void sig_child_handler(int signum)
{

    int pid, status;
    pid = wait(&status);
    if (WIFEXITED(status))
    {

        processRunning = 0; // meaning no process is running now
        int finish_time = getClk();

        if (WEXITSTATUS(status) == currentlyProcessing.processId &&
            currentlyProcessing.remainingTime != 0)
        {
            // we use finished time in wait time calculation
            // after process is paused
            // and set it to the correct value when it finishes
            total_useful_time += quantum;
            currentlyProcessing.finishTime = finish_time;
            currentlyProcessing.state = PAUSED;
            char log_message[100];
            sprintf(log_message, "At time %d process %d stopped arr %d total %d remain %d wait %d", currentlyProcessing.finishTime, currentlyProcessing.processId, currentlyProcessing.arrivalTime, currentlyProcessing.runTime, currentlyProcessing.remainingTime, currentlyProcessing.waitingTime);
            //write_to_file(log_file, log_message);
            push_queue(&myQueue, currentlyProcessing);
        }
        else
        {
            // discard process
            currentlyProcessing.finishTime = finish_time;

            total_useful_time += (finish_time - last_run_time);
            int TA = currentlyProcessing.finishTime - currentlyProcessing.arrivalTime;
            float WTA = (float)TA / currentlyProcessing.runTime;
            Ex += WTA;
            Ex2 += WTA * WTA;

            char log_message[100];
            sprintf(log_message, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f",
                    currentlyProcessing.finishTime, currentlyProcessing.processId, currentlyProcessing.arrivalTime, currentlyProcessing.runTime, currentlyProcessing.remainingTime, currentlyProcessing.waitingTime, TA, WTA);
            //write_to_file(log_file, log_message);

            int total_size = pow(2, ceil(log2(currentlyProcessing.size)));
            char log_message2[100];
            //write_to_file(mem_file, log_message2);
        }

        if (peek_queue(&myQueue).arrivalTime == -1 && // queue is empty
            isFinished_ProcGen == 1)
        {
            raise(SIGINT);
        }
        int succeeded = schedule_process();
        if (succeeded)
        {
            processRunning = 1;
        }
    }
}

void sig_processGen_finish(int signum)
{
    isFinished_ProcGen = 1;
}

void sig_processGen_handler(int signum)
{

    // recieving processes will be here in handler
    int rec_val = 0;
    while (rec_val != -1) // meaning message queue is empty
    {
        struct msgbuffer msg;
        rec_val = msgrcv(msgid, (struct msgbuffer *)&msg, sizeof(msg.proc), 0, IPC_NOWAIT);

        if (rec_val != -1) // there's something to recover from queue
        {
            // add process to queue
            struct process proc = msg.proc;
            push_queue(&myQueue, proc);
            total_processes++;
        }
    }

    if (processRunning == 0)
    {
        schedule_process();
        // here no need to check if scheduling failed due to empty queue since i just inserted now
        processRunning = 1;
    }
}