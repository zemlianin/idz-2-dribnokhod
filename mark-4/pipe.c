#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

double q_integral(double left, double right, double f_left, double f_right, double intgrl_now)
{

    double mid = (left + right) / 2;
    double f_mid = f(mid); // Аппроксимация по левому отрезку

    double l_integral = (f_left + f_mid) * (mid – left) / 2; // Аппроксимация по правому отрезку

    double r_integral = (f_mid + f_right) * (right - mid) / 2;
    if (abs((l_ integral + r_ integral) - intgrl_now) > EPS)
    { // Рекурсия для интегрирования обоих значений

        l_integral = q_integral(left, mid, f_left, f_mid, l_integral);
        r_integral =

            q_integral(mid, right, f_mid, f_right, r_integral);
    }
    return (l_integral + r_integral);
}

void child(int *pd, int semid, int semid2)
{
    for (size_t i = 0; i < 10; i++)
    {
        /* code */

        char result[20] = "\0";
        read(pd[0], result, 15);
        printf("CHILD: %s\n", result);
        struct sembuf child_buf =
            {.sem_num = 0, .sem_op = 1, .sem_flg = 0};

        if (semop(semid, &child_buf, 1) < 0)
        {
            printf("Can\'t add 1 to semaphor\n");
            exit(-1);
        }
        //        char *input;
        //        scanf("%s", input);
        write(pd[1], "inputChi", 15);
        struct sembuf parent_buf =
            {.sem_num = 0, .sem_op = -1, .sem_flg = 0};

        if (semop(semid2, &parent_buf, 1) < 0)
        {
            printf("Can\'t sub 1 from semaphor\n");
            exit(-1);
        }
    }
}

void parent(int *pd, int semid, int semid2)
{

    //    char *input;
    //    scanf("%s", input);
    for (size_t i = 0; i < 10; i++)
    {
        /* code */

        write(pd[1], "inputPar", 15);
        struct sembuf parent_buf =
            {.sem_num = 0, .sem_op = -1, .sem_flg = 0};

        if (semop(semid, &parent_buf, 1) < 0)
        {
            printf("Can\'t sub 1 from semaphor\n");
            exit(-1);
        }
        char result[20] = "\0";
        read(pd[0], result, 15);
        printf("PARENT: %s\n", result);
        struct sembuf child_buf =
            {.sem_num = 0, .sem_op = 1, .sem_flg = 0};

        if (semop(semid2, &child_buf, 1) < 0)
        {
            printf("Can\'t add 1 to semaphor\n");
            exit(-1);
        }
    }
}

int main()
{
    int fd[2];

    if (pipe(fd) < 0)
    {
        printf("Pipe error\n");
        return -1;
    }

    char pathname[] = "pipes";
    char pathname2[] = "pipes2";
    key_t key = ftok(pathname, 14);
    key_t key2 = ftok(pathname2, 15);
    int semid;
    int semid2;
    struct sembuf mybuf;

    if ((semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL)) < 0)
    {
        printf("Can\'t create semaphore1\n");
        return -1;
    }

    if ((semid2 = semget(key2, 1, 0666 | IPC_CREAT | IPC_EXCL)) < 0)
    {
        printf("Can\'t create semaphore2\n");
        return -1;
    }

    semctl(semid, 0, SETVAL, 0);
    semctl(semid2, 0, SETVAL, 0);

    pid_t pid = fork();

    if (pid < 0)
    {
        printf("Fork error\n");
        return -1;
    }
    else if (pid == 0)
    {
        child(fd, semid, semid2);
    }
    else
    {
        parent(fd, semid, semid2);
        wait(0);
        close(fd[0]);
        close(fd[1]);

        if (semctl(semid, 0, IPC_RMID, 0) < 0)
        {
            printf("Can\'t delete semaphore1\n");
            return -1;
        }

        if (semctl(semid2, 0, IPC_RMID, 0) < 0)
        {
            printf("Can\'t delete semaphore2\n");
            return -1;
        }
    }
    // if (semctl(semid, 0, IPC_RMID, 0) < 0) {
    //   printf("Can\'t delete semaphore\n");
    //   return -1;
    // }
    return 0;
}
