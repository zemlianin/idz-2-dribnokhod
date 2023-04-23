#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_THREADS 4

// Глобальный семафор
sem_t mySemaphore;

void *threadFunc(void *arg)
{
    int threadId = *((int *)arg);
    printf("Поток #%d ожидает семафор...\n", threadId);
    sem_wait(&mySemaphore); // Ожидаем семафор

    printf("Поток #%d получил семафор и выполняет свою работу.\n", threadId);

    // sem_post(&mySemaphore); // Освобождаем семафор

    return NULL;
}

int main()
{
    pthread_t threads[NUM_THREADS];
    int threadIds[NUM_THREADS];

    // Инициализируем семафор с начальным значением 1
    sem_init(&mySemaphore, 0, 0);

    printf("Старт программы.\n");

    // Создаем потоки
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threadIds[i] = i + 1;
        pthread_create(&threads[i], NULL, threadFunc, &threadIds[i]);
    }
    sleep(4);
    sem_post(&mySemaphore);
    // Ожидаем завершения всех потоков
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        pthread_join(threads[i], NULL);
    }
    // Уничтожаем семафор
    sem_destroy(&mySemaphore);

    printf("Программа завершила работу.\n");

    return 0;
}
