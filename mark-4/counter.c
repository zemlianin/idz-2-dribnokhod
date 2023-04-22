#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

double fun_param_1;
double fun_param_2;
double fun_param_3;
double EPS;

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

int main(int argc, char **argv)
{
    if (argc < 7)
    {
        printf("Less then 6 argument in counter");
        return 0;
    }
    double left = atof(argv[1]);
    double right = atof(argv[2]);
    fun_param_1 = atof(argv[3]);
    fun_param_2 = atof(argv[4]);
    fun_param_3 = atof(argv[5]);
    EPS = atof(argv[6]);
    double area = q_integral(left, right, f(left), f(right), (f(left) + f(right)) * (right - left) / 2);

    printf("%lf\n", area);
}