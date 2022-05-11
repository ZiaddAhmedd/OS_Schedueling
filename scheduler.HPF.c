#include "headers.h"

struct Queue Queue;
key_t key_id;
int rec_val, msgid;
int main(int argc, char *argv[])
{

  Queue = createQueue();
  initClk();

  int rec_process = 0;
  int processCount = atoi(argv[1]);
  key_id = ftok("./clk.c", 'Z');            // create unique key
  msgid = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id

  if (msgid == -1)
  {
    perror("Error in create");
    exit(-1);
  }
  printf("Message Queue ID = %d\n", msgid);

  struct msgbuffer msg;
  printf("%d\n", processCount);
  while (isEmpty_Queue(&Queue) == 0 || rec_process < processCount)
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
        printf("\nProcess recieved: %d %d %d %d\n", data.processId, data.arrivalTime, data.runTime, data.priority);
      }

      rec_process++;
    } while (rec_val != -1);
  }

  msgctl(msgid, IPC_RMID, (struct msqid_ds *)0);
  return 0;
}