#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>

void handle_signal(int sig)
{
    printf("Received signal %d\n", sig);
    exit(1);
}

int main()
{

    signal(SIGINT, handle_signal);
    while (1)
    {
    }
}