#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

float EPS = 0.01;
float fun_param_1;
float fun_param_2;
float fun_param_3;
pthread_mutex_t mutex;

int semid;
int semid2;

double f(double x)
{
	return abs(fun_param_1 * x * x * x + fun_param_2 * x * x + fun_param_3 * x);
}

double q_integral(double left, double right, double f_left, double f_right, double intgrl_now)
{

	double mid = (left + right) / 2;
	double f_mid = f(mid); // Аппроксимация по левому отрезку

	double l_integral = (f_left + f_mid) * (mid - left) / 2; // Аппроксимация по правому отрезку

	double r_integral = (f_mid + f_right) * (right - mid) / 2;
	if (abs((l_integral + r_integral) - intgrl_now) > EPS)
	{ // Рекурсия для интегрирования обоих значений
		l_integral = q_integral(left, mid, f_left, f_mid, l_integral);
		r_integral = q_integral(mid, right, f_mid, f_right, r_integral);
	}

	return (l_integral + r_integral);
}

void counter()
{

	const int SIZE = 4096;
	const char *namel = "left";
	int shm_fdl;
	void *ptrl;
	shm_fdl = shm_open(namel, O_RDONLY | O_RDWR, 0666);
	ptrl = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fdl, 0);

	const char *namer = "right";
	int shm_fdr;
	void *ptrr;
	shm_fdr = shm_open(namer, O_RDONLY | O_RDWR, 0666);
	ptrr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fdr, 0);

	const char *names1 = "s1";
	int shm_fds1;
	void *ptrs1;
	shm_fds1 = shm_open(names1, O_RDONLY | O_RDWR, 0666);
	ptrs1 = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fds1, 0);

	const char *names2 = "s2";
	int shm_fds2;
	void *ptrs2;
	shm_fds2 = shm_open(names2, O_RDONLY | O_RDWR, 0666);
	ptrs2 = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fds2, 0);

	float left;
	float right;
	double sum = 0;
	do
	{
		struct sembuf parent_buf =
			{.sem_num = 0, .sem_op = -1, .sem_flg = 0};

		if (semop(semid, &parent_buf, 1) < 0)
		{
			printf("Can\'t sub 1 from semaphor\n");
			exit(-1);
		}

		float left = atof((char *)ptrl);
		float right = atof((char *)ptrr);

		if (left == -1 || right == -1)
		{
			break;
		}

		struct sembuf child_buf =
			{.sem_num = 0, .sem_op = 1, .sem_flg = 0};

		if (semop(semid2, &child_buf, 1) < 0)
		{
			printf("Can\'t add 1 to semaphor\n");
			exit(-1);
		}

		double area = q_integral(left, right, f(left), f(right), (f(left) + f(right)) * (right - left) / 2);
		printf("%f\t%f\t : %f\n", left, right, area);
		sum += area;
	} while (left != -1 && right != -1);

	float t;
	sscanf(ptrs1, "%f", &t);
	sprintf(ptrs1, "%f", t + sum);

	sscanf(ptrs2, "%f", &t);
	sprintf(ptrs2, "%f", t + 1);

	shm_unlink(namer);
	shm_unlink(namel);
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Less then 1 argument");
		return 0;
	}

	const char *sem_name = "sem1";
	const char *semc1_name = "sem2";

	srand(time(NULL));
	key_t key = ftok(sem_name, rand());
	key_t key2 = ftok(semc1_name, rand());

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
	semctl(semid2, 0, SETVAL, 1);

	float a = 0;
	float b = 2;
	int sum = 0;
	int part_count = atoi(argv[1]);
	int process_count = (part_count / 2);

	int SIZE = 256;

	const char *namel = "left";
	const char *namer = "right";
	const char *names1 = "s1";
	const char *names2 = "s2";

	int shm_fdl;
	int shm_fdr;
	int shm_fds1;
	int shm_fds2;

	void *ptrl;
	void *ptrr;
	void *ptrs1;
	void *ptrs2;

	shm_fdl = shm_open(namel, O_CREAT | O_RDWR, 0666);
	shm_fdr = shm_open(namer, O_CREAT | O_RDWR, 0666);
	shm_fds1 = shm_open(names1, O_CREAT | O_RDWR, 0666);
	shm_fds2 = shm_open(names2, O_CREAT | O_RDWR, 0666);

	ftruncate(shm_fdl, SIZE);
	ftruncate(shm_fdr, SIZE);
	ftruncate(shm_fds1, SIZE);
	ftruncate(shm_fds2, SIZE);

	ptrl = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fdl, 0);
	ptrr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fdr, 0);
	ptrs1 = mmap(0, SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fds1, 0);
	ptrs2 = mmap(0, SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fds2, 0);

	FILE *input = fopen("input.txt", "rt");

	fscanf(input, "%f", &fun_param_1);
	fscanf(input, "%f", &fun_param_2);
	fscanf(input, "%f", &fun_param_3);
	fclose(input);

	sprintf(ptrs1, "%f", 0.0);
	sprintf(ptrs2, "%f", 0.0);
	int flag = 0;

	for (size_t i = 0; i < process_count; i++)
	{
		if (fork())
		{
			counter();
			exit(0);
		}
	}

	for (size_t i = 0; i < part_count; i++)
	{
		struct sembuf parent_buf =
			{.sem_num = 0, .sem_op = -1, .sem_flg = 0};

		if (semop(semid2, &parent_buf, 1) < 0)
		{
			printf("Can\'t sub 1 from semaphor\n");
			exit(-1);
		}

		float left = ((b - a) / part_count) * i + a;
		float right = ((b - a) / part_count) * (i + 1) + a;
		sprintf(ptrl, "%f", left);
		sprintf(ptrr, "%f", right);

		struct sembuf child_buf =
			{.sem_num = 0, .sem_op = 1, .sem_flg = 0};

		if (semop(semid, &child_buf, 1) < 0)
		{
			printf("Can\'t add 1 to semaphor\n");
			exit(-1);
		}
	}

	struct sembuf parent_buf =
		{.sem_num = 0, .sem_op = -1, .sem_flg = 0};

	if (semop(semid2, &parent_buf, 1) < 0)
	{
		printf("Can\'t sub 1 from semaphor\n");
		exit(-1);
	}

	sprintf(ptrl, "%f", -1.0);
	sprintf(ptrr, "%f", -1.0);
	for (size_t i = 0; i < part_count; i++)
	{
		struct sembuf child_buf =
			{.sem_num = 0, .sem_op = 1, .sem_flg = 0};

		if (semop(semid, &child_buf, 1) < 0)
		{
			printf("Can\'t add 1 to semaphor\n");
			exit(-1);
		}
	}

	float res;
	while (atof((char *)ptrs2) != process_count)
	{
		sleep(1);
	}

	sscanf(ptrs1, "%f", &res);
	FILE *output = fopen("output.txt", "w");

	fprintf(output, "%f", res);

	fclose(output);

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

	shm_unlink(namer);
	shm_unlink(namel);
	return 0;
}