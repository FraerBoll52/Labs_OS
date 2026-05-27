/**
 * Implementation of thread pool.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "threadpool.h"

#define QUEUE_SIZE 10
#define NUMBER_OF_THREADS 3

#define TRUE 1

typedef struct task_node
{
    void (*function)(void *p);
    void *data;
    struct task_node *next;
} task;

task *queue_head = NULL;
task *queue_tail = NULL;

int queue_count = 0;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t queue_sem;

pthread_t workers[NUMBER_OF_THREADS];
int pool_active = 1;

int enqueue(task t)
{
    task *new_task = (task*)malloc(sizeof(task));
    if (new_task == NULL)
        return 1; 

    new_task->function = t.function;
    new_task->data = t.data;
    new_task->next = NULL;

    pthread_mutex_lock(&queue_mutex);

    if (queue_tail == NULL)
    {
        queue_head = new_task;
        queue_tail = new_task;
    }
    else
    {
        queue_tail->next = new_task;
        queue_tail = new_task;
    }
    queue_count++;

    pthread_mutex_unlock(&queue_mutex);

    sem_post(&queue_sem);

    return 0;
}


task dequeue()
{
    pthread_mutex_lock(&queue_mutex);

    task *temp = queue_head;
    task work;
    work.function = temp->function;
    work.data = temp->data;

    queue_head = queue_head->next;
    if (queue_head == NULL)
        queue_tail = NULL;

    queue_count--;

    pthread_mutex_unlock(&queue_mutex);

    free(temp);
    return work;
}


void *worker(void *param)
{
    while (1)
    {
        
        sem_wait(&queue_sem);

        pthread_mutex_lock(&queue_mutex);
        int should_exit = (!pool_active && queue_head == NULL);
        pthread_mutex_unlock(&queue_mutex);

        if (should_exit)
        {
            sem_post(&queue_sem);
            pthread_exit(0);
        }

        task mytask = dequeue();

        execute(mytask.function, mytask.data);
    }

    pthread_exit(0);
}


void execute(void (*somefunction)(void *p), void *p)
{
    (*somefunction)(p);
}


int pool_submit(void (*somefunction)(void *p), void *p)
{
    task newtask;
    newtask.function = somefunction;
    newtask.data = p;

    return enqueue(newtask);
}


void pool_init(void)
{
    sem_init(&queue_sem, 0, 0);

    for (int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        pthread_create(&workers[i], NULL, worker, NULL);
    }
}


void pool_shutdown(void)
{
    
    pthread_mutex_lock(&queue_mutex);
    pool_active = 0;
    pthread_mutex_unlock(&queue_mutex);

    
    for (int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        sem_post(&queue_sem);
    }

    
    for (int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        pthread_join(workers[i], NULL);
    }

    
    sem_destroy(&queue_sem);
    pthread_mutex_destroy(&queue_mutex);
}