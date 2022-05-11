#include "headers.h"

void clearResources(int);
int msgid;
int send_to_sch;

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    FILE *ptr = fopen(argv[1], "r");
    // done el file nakhdo wa2t el run khaliha argv

    if (ptr == NULL)
    {
        printf("no such file.");
        return 0;
    }
    fscanf(ptr, "%*[^\n]\n");

    int buf;
    int processCount = 0;

    // [done] traverse el file to know 3adad el processes

    // Extract characters from file and store in character c
    for (char c = getc(ptr); c != EOF; c = getc(ptr))
        if (c == '\n') // Increment count if this character is newline
            processCount++;

    // Close the file
    fclose(ptr);
    printf("The file has %d lines\n", processCount);

    ptr = fopen(argv[1], "r"); // todo el file nakhdo wa2t el run khaliha argv
    fscanf(ptr, "%*[^\n]\n");

    int ids[processCount];
    int arrivals[processCount];
    int runtimes[processCount];
    int priorities[processCount];

    for (int i = 0; i < processCount; i++)
    {
        fscanf(ptr, "%d", &buf);
        ids[i] = buf;

        fscanf(ptr, "%d", &buf);
        arrivals[i] = buf;

        fscanf(ptr, "%d", &buf);
        runtimes[i] = buf;

        fscanf(ptr, "%d", &buf);
        priorities[i] = buf;
    }
    fclose(ptr);

    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    int algorithim_type, quantum;
    algorithim_type = atoi(argv[3]);

    quantum = atoi(argv[5]);

    // 3. Initiate and create the scheduler and clock processes.

    int fork_pid_clock = fork();

    if (fork_pid_clock == 0)
    {
        // clk code
        // clk.out
        char *argv[] = {"./clk.out", 0};
        execve(argv[0], &argv[0], NULL); // mmkn nst3ml execve lw masht8ltsh!!
    }

    int fork_pid_schedule = fork();

    if (fork_pid_schedule == 0)
    {
        if (algorithim_type == SJF)
        {
            // SJF.out

            // char *argv[] = {"./scheduler.SJF.out", 0};
            // execve(argv[0], &argv[0], NULL);
            char buffer1[20];
            sprintf(buffer1, "%d", processCount);
            argv[1] = buffer1;
            // argv[1] = process_count ; needs to be fixeds
            if (execv("./scheduler.SJF.out", argv) == -1)
                perror("failed to execv");
        }
        else if (algorithim_type == HPF)
        {
            // HPF.out
            char buffer1[20];
            char buffer2[20];
            sprintf(buffer1, "%d", processCount);
            argv[1] = buffer1;
            if (execv("./scheduler.HPF.out", argv) == -1)
                perror("failed to execv");
        }
        else if (algorithim_type == RR)
        {
            // RR

            char buffer1[20];
            char buffer2[20];
            sprintf(buffer1, "%d", processCount);
            sprintf(buffer2, "%d", quantum);
            argv[1] = buffer1;
            argv[2] = buffer2;

            // argv[1] = process_count ; needs to be fixeds
            if (execv("./scheduler.RR.out", argv) == -1)
                perror("failed to execv");
        }
        else if (algorithim_type == MLFL)
        {
            char quantum_char[10];
            sprintf(quantum_char, "%d", quantum);

            char *argv[] = {"./scheduler.MLFL.out", quantum_char, 0};
            execve(argv[0], &argv[0], NULL);
        }
    }
    // char *arr[] = {"./scheduler.c", NULL};
    // execv("./scheduler.o", arr);
    /// char *args[] = {"./scheduler.c", NULL};
    // execv(args[0], args);

    key_t key = ftok("./clk.c", 'Z');
    msgid = msgget(key, IPC_CREAT | 0666);
    struct msgbuffer msg;
    if (msgid == -1)
    {
        perror("msgget");
        return 1;
    }
    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    int currentProcessIndex = 0;
    int x;
    while (currentProcessIndex < processCount)
    {
        x = getClk();
        sleep(1);
        x = getClk();
       // printf("Current Time is %d\n", x);
        while (arrivals[currentProcessIndex] == x)
        {

            msg.mtype = 1;
            msg.proc.processId = ids[currentProcessIndex];
            msg.proc.arrivalTime = arrivals[currentProcessIndex];
            msg.proc.runTime = runtimes[currentProcessIndex];
            msg.proc.size = processCount;
            if (algorithim_type == SJF)
            {
                msg.proc.priority = runtimes[currentProcessIndex];
            }
            else if (algorithim_type == RR)
            {
                msg.proc.priority = 20;
            }
            else
            {
                msg.proc.priority = priorities[currentProcessIndex];
            }
            send_to_sch = msgsnd(msgid, &msg, sizeof(msg.proc), !IPC_NOWAIT);
            if (send_to_sch == 0)
            {
                //printf("message successful at time %d \n", x);
               // printf ("sent: %d %d %d %d\n",ids[currentProcessIndex],arrivals[currentProcessIndex], runtimes[currentProcessIndex],priorities[currentProcessIndex] );
            }
            // printf("sent process %d\n", currentProcessIndex);
            // 2. Increment the current process index.
            currentProcessIndex++;
        }
    }
    int status;
    fork_pid_schedule = wait(&status);
    if (WIFEXITED(status))
    {
        int msgq_del;
        msgq_del = msgctl(msgid, IPC_RMID, 0);
        destroyClk(true);
        exit(0);
    }
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    // //   destroyClk(true);
    return 0;
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
    printf("The process generator has stopped\n");
    int msgq_del;
    msgq_del = msgctl(msgid, IPC_RMID, 0);
    destroyClk(true);
    exit(0);
}