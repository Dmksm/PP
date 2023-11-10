#include <iostream>
#include <omp.h>

int main()
{
    int x = 44;
    #pragma omp parallel for private(x)
    for (int i = 0; i <= 10; i++) {
        printf("Thread number: %d x: %d\n", omp_get_thread_num(), x);
        x = i;
    }
    printf("x is %d\n", x);

    int y = 44;
    #pragma omp parallel for firstprivate(y)
    for (int i = 0; i <= 10; i++) {
        printf("Thread number: %d x: %d\n", omp_get_thread_num(), y);
        y = i;
    }
    printf("y is %d\n", y);

    int z = 44;
    #pragma omp parallel for lastprivate(z)
    for (int i = 0; i <= 10; i++) {
        printf("Thread number: %d z: %d\n", omp_get_thread_num(), z);
        z = i;
    }
    printf("z is %d\n", z);
}
