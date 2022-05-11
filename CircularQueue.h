#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Node
{
    process data;
    struct Node *next;
};

struct CircularQueue
{
    struct Node *front;
    struct Node *rear;
};

struct CircularQueue initializeQueue()
{
    struct CircularQueue queue;
    queue.front = NULL;
    queue.rear = NULL;
    return queue;
}

int isEmpty_queue(struct CircularQueue *queue)
{
    return queue->front == NULL;
}

void push_queue(struct CircularQueue *queue, process data)
{
    struct Node *temp = (struct Node *)malloc(sizeof(struct Node));
    temp->data = data;
    temp->next = NULL;

    if (isEmpty_queue(queue))
    {
        queue->front = temp;
        queue->rear = temp;
    }
    else
    {
        queue->rear->next = temp;
        queue->rear = temp;
    }
}

void pop_queue(struct CircularQueue *queue)
{
    if (isEmpty_queue(queue))
    {
        return;
    }

    //
    struct Node *temp = queue->front;
    queue->front = queue->front->next;

    // if empty make front == rear == NULL
    if (queue->front == NULL)
    {
        queue->rear = NULL;
    }

    free(temp);
}

process peek_queue(struct CircularQueue *queue)
{
    if (isEmpty_queue(queue))
    {

        process data;
        data.arrivalTime = -1;
        return data;
    }
    return queue->front->data;
}