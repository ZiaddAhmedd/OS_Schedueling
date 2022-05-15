// A C program to demonstrate linked list based implementation of queue
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// A linked list (LL) node to store a queue entry
// struct Node
// {
//   process data;
//   struct Node *next;
// };

// The queue, front stores the front node of LL and rear stores the
// last node of LL
struct Queue
{
  struct Node *Head;
};

// A utility function to create an empty queue
struct Queue createQueue()
{
  struct Queue q;
  q.Head = NULL;
  return q;
}

// A utility function to create a new linked list node.
// struct Node *newNode(struct process data, int status)
// {
//   struct Node *temp = (struct Node *)malloc(sizeof(struct Node));
//   temp->data = data;
//   temp->next = NULL;
//   return temp;
// }

// The function to add a key k to q
void enqueue(struct Queue *q, process data)
{
  // Create a new LL node
  struct Node *temp = (struct Node *)malloc(sizeof(struct Node));
  temp->data = data;
  // If queue is empty, then new node is front and rear both
  if (q->Head == NULL)
  {
    q->Head = temp;
    return;
  }

  struct Node *p = q->Head;
  if (p->data.priority > temp->data.priority)
  {

    // Insert New Node before head
    temp->next = q->Head;
    q->Head = temp;
  }
  else
  {
    // Traverse the list and find a
    // position to insert new node
    while (p->next != NULL && p->next->data.priority <= temp->data.priority)
      p = p->next;

    // Either at the ends of the list
    // or at required position
    temp->next = p->next;
    p->next = temp;
  }
}

// Function to remove a key from given queue q
void dequeue(struct Queue *q)
{
  // If queue is empty, return NULL.
  if (q->Head == NULL)
    return;

  // Store previous front and move front one node ahead
  struct Node *temp = q->Head;

  q->Head = q->Head->next;

  // If front becomes NULL, then change rear also as NULL
  
}

// struct Node *dequeue(struct Queue *q)
// {
//   if (q->Head == NULL)
//     return NULL;
//   struct Node *temp = q->Head;
//   q->Head = q->Head->next;
//   return temp;
// }

void printqueue(struct Queue *q)
{
  struct Node *p = q->Head;
  while (p != NULL)
  {
    printf("process ID:%d ->", p->data.processId);
    p = p->next;
  }
  printf("NULL\n");
}

int isEmpty_Queue(struct Queue *q)
{
  return (q->Head == NULL) ? 1 : 0;
}

process peek_Queue(struct Queue *q)
{
  if (isEmpty_Queue(q))
  {
    process data;
    data.arrivalTime = -1;
    return data;
  }
  return q->Head->data;
}
