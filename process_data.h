typedef struct process
{
    int runTime;
    int priority;
    int processId;
    int arrivalTime;
    int size;

    int state;
    int waitingTime; 
    int executionTime; //mn3rfsh lesa este3malo ehtmal utilization
    int remainingTime;
    int contextSwitchTime; 
    int finishTime;
    int startTime;
    //int mem_start_position;
}process;

 process initializeProcess(
    int runtime, int priority,
    int processId, int arrivalTime,
    int size)
{
     process p;
    p.runTime = runtime;
    p.priority = priority;
    p.processId = processId;
    p.arrivalTime = arrivalTime;
    p.remainingTime = runtime;
    p.size = size;

    p.state = 0;
    p.waitingTime = 0;
    p.executionTime = 0;
    p.contextSwitchTime = 0;
    p.finishTime = -1;
    p.startTime = -1;
    //p.mem_start_position = -1;
    return p;
}

void initializeProcessPointer(
    process *p,
    int runtime, int priority,
    int processId, int arrivalTime,
    int size)
{
    p->runTime = runtime;
    p->priority = priority;
    p->processId = processId;
    p->arrivalTime = arrivalTime;
    p->remainingTime = runtime;
    p->size = size;

    p->state = 0;
    p->waitingTime = 0;
    p->executionTime = 0;
    p->contextSwitchTime = 0;
    p->finishTime = -1;
    p->startTime = -1;
    //p->mem_start_position = -1;

}
 process copyProcess( process input)
{
    return input;
};