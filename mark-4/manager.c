#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>

float EPS = 0.01;
float fun_param_1;
float fun_param_2;
float fun_param_3;
pthread_mutex_t mutex;

double f(double x)
{
	return fun_param_1 * x * x * x + fun_param_2 * x * x + fun_param_3 * x;
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
	sem_t *sem;
	sem_t *semc1;
	sem_t *semf;

	sem = sem_open("sem", O_CREAT, 0666, 1);
	semc1 = sem_open("semc1", O_CREAT, 0666, 1);
	semf = sem_open("semf", O_CREAT, 0666, 1);

	if (sem == SEM_FAILED)
	{
		perror("sem_open");
		exit(EXIT_FAILURE);
	}

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
		pthread_mutex_lock(&mutex);
		if (left == -1 || right == -1)
		{
			pthread_mutex_unlock(&mutex);
			break;
		}
		sem_wait(semc1);
		float left = atof((char *)ptrl);
		float right = atof((char *)ptrr);
		sem_post(sem);
		if (left == -1 || right == -1)
		{
			pthread_mutex_unlock(&mutex);
			break;
		}

		pthread_mutex_unlock(&mutex);
		double area = q_integral(left, right, f(left), f(right), (f(left) + f(right)) * (right - left) / 2);
		sum += area;
		printf("%f\t%f\n", left, right);
	} while (left != -1 && right != -1);

	printf("I was exit\n");

	float t;
	sscanf(ptrs1, "%f", &t);
	sprintf(ptrs1, "%f", t + sum);
	sscanf(ptrs2, "%f", &t);
	sprintf(ptrs2, "%f", t + 1);
	fflush(NULL);
	sem_post(semc1);
	sem_close(semc1);
	shm_unlink(namer);
	shm_unlink(namel);
	sem_post(semf);
	sem_close(semf);
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Less then 1 argument");
		return 0;
	}
	const char *sem_name = "sem";
	pthread_mutex_init(&mutex, NULL);
	sem_t *sem;
	sem_t *semc1;
	sem_t *semf;
	semc1 = sem_open("semc1", O_CREAT, 0666, 1);
	semf = sem_open("semf", O_CREAT, 0666, 1);
	if ((sem = sem_open(sem_name, O_CREAT, 0666, 0)) == 0)
	{
		perror("sem_open: Can not create admin semaphore");
		exit(-1);
	};
	float a = 0;
	float b = 2;
	int temp = 0, sum = 0;
	int part_count = atoi(argv[1]);
	// int part_count = 10;
	int SIZE = 256;

	const char *namel = "left";
	const char *namer = "right";
	const char *names1 = "s1";
	const char *names2 = "s2";
	/* shared memory file descriptor */
	int shm_fdl;
	int shm_fdr;
	int shm_fds1;
	int shm_fds2;

	/* pointer to shared memory object */
	void *ptrl;
	void *ptrr;
	void *ptrs1;
	void *ptrs2;
	/* create the shared memory object */
	shm_fdl = shm_open(namel, O_CREAT | O_RDWR, 0666);
	shm_fdr = shm_open(namer, O_CREAT | O_RDWR, 0666);
	shm_fds1 = shm_open(names1, O_CREAT | O_RDWR, 0666);
	shm_fds2 = shm_open(names2, O_CREAT | O_RDWR, 0666);

	/* configure the size of the shared memory object */
	ftruncate(shm_fdl, SIZE);
	ftruncate(shm_fdr, SIZE);
	ftruncate(shm_fds1, SIZE);
	ftruncate(shm_fds2, SIZE);

	/* memory map the shared memory object */
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

	for (size_t i = 0; i < 2; i++)
	{
		if (fork())
		{
			counter();
			exit(0);
			sem_post(sem);
			sem_close(sem);
		}
	}

	for (size_t i = 0; i < part_count + 1; i++)
	{
		float left = ((b - a) / part_count) * i + a;
		float right = ((b - a) / part_count) * (i + 1) + a;
		if (i != 0)
		{
			sem_wait(sem);
		}
		sleep(1);
		sprintf(ptrl, "%f", left);
		sprintf(ptrr, "%f", right);
		sem_post(semc1);
	}
	float left = -1;
	float right = -1;
	sprintf(ptrl, "%f", left);
	sprintf(ptrr, "%f", right);

	float res;
	while (atof((char *)ptrs2) != 2)
	{
		sleep(1);
	}

	sscanf(ptrs1, "%f", &res);
	printf("%f", res);
	FILE *output = fopen("output.txt", "w");

	fprintf(output, "%f", res);

	fclose(output);

	if (sem_close(sem) == -1 && sem_close(semf) == -1 && sem_close(semc1) == -1)
	{
		perror("sem_close: Incorrect close of busy semaphore");
		exit(-1);
	};
	if (sem_unlink(sem_name) == -1 && sem_unlink("semc1") == -1 && sem_unlink("semf") == -1)
	{
		perror("sem_unlink: Incorrect unlink of full semaphore");
		exit(-1);
	};
	shm_unlink(namer);
	shm_unlink(namel);
	return 0;
}