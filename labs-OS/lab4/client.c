#include <stdio.h>
#include <unistd.h>
#include "threadpool.h"

struct data
{
    int a;
    int b;
};

void add(void *param)
{
    struct data *temp = (struct data*)param;
    printf("Вычисляю %d + %d = %d\n", temp->a, temp->b, temp->a + temp->b);
}

int main(void)
{
    struct data tasks[5];
    
    printf("Инициализация пула потоков...\n");
    pool_init();
    
    printf("Отправка 5 задач в пул...\n");
    for (int i = 0; i < 5; i++)
    {
        tasks[i].a = i * 10;
        tasks[i].b = i * 5;
        printf("Задача %d: %d + %d\n", i+1, tasks[i].a, tasks[i].b);
        pool_submit(&add, &tasks[i]);
    }
    
    pool_shutdown();
    
    printf("Программа завершена\n");
    return 0;
}