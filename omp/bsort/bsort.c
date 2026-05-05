#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>

double time_diff_sec(struct timeval st, struct timeval et)
{
    return (double)(et.tv_sec-st.tv_sec)+(et.tv_usec-st.tv_usec)/1000000.0;
}

int init(double *data, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        data[i] = (double)rand() / RAND_MAX;
   }
    return 0;
}

int print(double *data, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        printf("%.4lf ", data[i]);
    }
    printf("\n");
    return 0;
}

static inline void swap(double *x, double *y)
{
    double stp=*x; *x=*y; *y=stp;
    return;
}

void bitonic_0(double *num, int bit, int hb, int lb)
{
    int A=1<<(bit-hb-1), B=1<<(hb-lb-1), C=1<<lb;
    #pragma omp for collapse(3)
    for(int i=0; i<A; i++)
    {
        for(int j=0; j<B; j++)
        {
            for(int k=0; k<C; k++)
            {
                int now=(i<<(hb+1))|(j<<(lb+1))|k;
                if(num[now]>num[now|C]) swap(num+now, num+(now|C));
            }
        }
    }
    return;
}

void bitonic_1(double *num, int bit, int hb, int lb)
{
    int A=1<<(bit-hb-1), B=1<<(hb-lb-1), C=1<<lb;
    #pragma omp for collapse(3)
    for(int i=0; i<A; i++)
    {
        for(int j=0; j<B; j++)
        {
            for(int k=0; k<C; k++)
            {
                int now=(i<<(hb+1))|(j<<(lb+1))|k|(1<<hb);
                if(num[now]<num[now|C]) swap(num+now, num+(now|C));
            }
        }
    }
    return;
}

void sort(double *data, int N)
{
    int n=1, bit=0;
    double inf=39, *num;
    while(n<N) n<<=1, bit++;
    num=malloc(sizeof(double)*n);
    #pragma omp parallel for
    for(int i=0; i<N; i++) num[i]=data[i];
    #pragma omp parallel for
    for(int i=N; i<n; i++) num[i]=inf;
    #pragma omp parallel
    {
        for(int i=1; i<bit; i++)
        {
            for(int j=i-1; j>=0; j--)
            {
                bitonic_0(num, bit, i, j);
                bitonic_1(num, bit, i, j);
            }
        }
        for(int i=bit-1; i>=0; i--)
        {
            bitonic_0(num, bit+1, bit, i);
        }
    }
    #pragma omp parallel for
    for(int i=0; i<N; i++) data[i]=num[i];
    free(num);
    return;
}

int check(double *data, int n)
{
    int i;
    int flag = 0;
    for (i = 0; i < n-1; i++) {
        if (data[i] > data[i+1] || data[i]>1.39) {
            printf("Error: data[%d]=%.4lf, data[%d]=%.4lf\n",
                   i, data[i], i+1, data[i+1]);
            flag++;
        }
    }
    if (flag == 0) {
        printf("Data are sorted\n");
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int n = 1000039;
    double *data, average_time=0;
    int i;

    if (argc >= 2) {
        n = atol(argv[1]);
    }

    int cpu_threads = omp_get_num_procs();
    omp_set_num_threads(cpu_threads);
    printf("%d\n",cpu_threads);
    int epochs = 39;

    data = malloc(sizeof(double)*n);
    for (i = 0; i < epochs; i++) {
        struct timeval st;
        struct timeval et;
        double sec;

        init(data, n);
        /*print(data, n);*/
        gettimeofday(&st, NULL); /* get start time */
        sort(data, n);
        gettimeofday(&et, NULL); /* get start time */
        sec = time_diff_sec(st, et);
        average_time += sec;

        printf("sorting %d data took %lf sec\n",
               n, sec);

        check(data, n);
        /*print(data, n);*/
    }
    average_time /= epochs;
    printf("average sorting time: %lf sec\n", average_time);
    free(data);

    return 0;
}
