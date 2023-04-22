#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>

float EPS = 0.01;
float fun_param_1;
float fun_param_2;
float fun_param_3;

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

int counter(void *ptrl, void *ptrr, sem_t *sem)
{
	float left;
	float right;
	do
	{
		float left = (char *)ptrl;
		float right = (char *)ptrr;
		sem_post(sem);
		double area = q_integral(left, right, f(left), f(right), (f(left) + f(right)) * (right - left) / 2);
		printf("%lf\n", area);
	} while (left != -1 && right != -1);
	sprintf(ptrl, "%f", left);
	sprintf(ptrr, "%f", right);
	exit(0);
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Less then 1 argument");
		return 0;
	}
	const char *sem_name = "/semaphore";
	sem_t *sem;
	if ((sem = sem_open(sem_name, O_CREAT, 0666, 0)) == 0)
	{
		perror("sem_open: Can not create admin semaphore");
		exit(-1);
	};
	float a = 0;
	float b = 2;
	int temp = 0, sum = 0;
	// int count = atoi(argv[1]);
	int part_count = 10;
	int SIZE = 256;

	const char *namel = "left";
	const char *namer = "right";
	/* shared memory file descriptor */
	int shm_fdl;
	int shm_fdr;

	/* pointer to shared memory object */
	void *ptrl;
	void *ptrr;
	/* create the shared memory object */
	shm_fdl = shm_open(namel, O_CREAT | O_RDWR, 0666);
	shm_fdl = shm_open(namer, O_CREAT | O_RDWR, 0666);

	/* configure the size of the shared memory object */
	ftruncate(shm_fdl, SIZE);
	ftruncate(shm_fdr, SIZE);
	/* memory map the shared memory object */
	ptrl = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fdl, 0);
	ptrl = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fdr, 0);

	FILE *input = fopen("input.txt", "rt");

	fscanf(input, "%f", &fun_param_1);
	fscanf(input, "%f", &fun_param_2);
	fscanf(input, "%f", &fun_param_3);
	fclose(input);

	if (fork())
	{
		counter(ptrl, ptrr, sem);
	}
	if (fork())
	{
		counter(ptrl, ptrr, sem);
	}

	for (size_t i = 0; i < part_count; i++)
	{
		float left = ((b - a) / part_count) * i + a;
		float right = ((b - a) / part_count) * (i + 1) + a;

		sprintf(ptrl, "%f", left);
		sprintf(ptrr, "%f", right);
		sem_wait(sem);
	}
	sleep(3);
	FILE *output = fopen("output.txt", "w");

	//	fprintf(output, "%f", q_integral(a, b, f(a), f(b), (f(a) + f(b)) * (b - a) / 2));

	fclose(output);

	if (sem_close(sem) == -1)
	{
		perror("sem_close: Incorrect close of busy semaphore");
		exit(-1);
	};
	if (sem_unlink(sem_name) == -1)
	{
		perror("sem_unlink: Incorrect unlink of full semaphore");
		// exit(-1);
	};

	return 0;
}