#include "headers.h"

void clearResources(int);

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
    int processCount;

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
    printf("Enter algorithim type (1 for SJF, 2 for HPF, 3 for RR, 4 for MLFL):\n");
    scanf("%d", &algorithim_type);

    if ((algorithim_type == RR)||(algorithim_type==MLFL))
    {
        printf("Enter quantum: ");
        scanf("%d", &quantum);
    }

    // 3. Initiate and create the scheduler and clock processes.

    int fork_pid_clock = fork();

    if (fork_pid_clock == 0)
    {
        // clk code
        //clk.out
        char *argv[] = {"./clk.out", 0};
        execve(argv[0], &argv[0], NULL); //mmkn nst3ml execve lw masht8ltsh!!
    }

    int fork_pid_schedule = fork();

     if (fork_pid_schedule == 0)
    {
        if (algorithim_type == SJF)
        {
            // SJF.out

            char *argv[] = {"./scheduler.SJF.out", 0};
            execv(argv[0], &argv[0], NULL);
        }
        else if (algorithim_type == HPF)
        {
            // HPF.out
            char *argv[] = {"./scheduler.HPF.out", 0};
            execv(argv[0], &argv[0], NULL);
        }
        else if (algorithim_type == RR)
        {
            //RR

            char quantum_char[10];
            sprintf(quantum_char, "%d", quantum);

            char *argv[] = {"./scheduler.RR.out", quantum_char, 0};
            execv(argv[0], &argv[0], NULL);
        }
        else if (algorithim_type == MLFL)
        {
            char quantum_char[10];
            sprintf(quantum_char, "%d", quantum);

            char *argv[] = {"./scheduler.MLFL.out", quantum_char, 0};
            execv(argv[0], &argv[0], NULL);
        }
    }
    // char *arr[] = {"./scheduler.c", NULL};
    // execv("./scheduler.o", arr);
    /// char *args[] = {"./scheduler.c", NULL};
    // execv(args[0], args);


    // 4. Use this function after creating the clock process to initialize clock.
        initClk();

    // To get time use this function.
         int x = getClk();
         printf("Current Time is %d\n", x);
    

        key_t key = ftok("./clk.c", 'Z'); 
        int msgid = msgget(key, IPC_CREAT | 0666);
        if (msgid == -1)
        {
            perror("msgget");
            return 1;
        }
        int currentProcessIndex=0;
        while (currentProcessIndex < processCount )
        {

            struct msgbuffer msg;
            msg.mtype = 1;
            msg.proc.processId = ids[currentProcessIndex];
            msg.proc.arrivalTime = arrivals[currentProcessIndex];
            msg.proc.runTime = runtimes[currentProcessIndex];
            if (algorithim_type ==SJF)
            {
                msg.proc.priority = runtimes[currentProcessIndex];
            }else if (algorithim_type ==RR)
            {
                msg.proc.priority = 20;
            }else 
            {
                msg.proc.priority = priorities[currentProcessIndex];
            }
            
            if (msgsnd(msgid, &msg, sizeof(msg.proc), 0) == -1) // 0 means IPC_NOWAIT
            {
                perror("msgsnd");
                exit(1);
            }
            // printf("sent process %d\n", currentProcessIndex);
            // 2. Increment the current process index.
            currentProcessIndex++;
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
}