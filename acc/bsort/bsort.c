#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <openacc.h>

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

void bitonic_0(double *num, int bit, int hb, int lb, int N)
{
    int A=1<<(bit-hb-1), B=1<<(hb-lb-1), C=1<<lb;
    #pragma acc parallel loop collapse(3) present(num[0:N])
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

void bitonic_1(double *num, int bit, int hb, int lb, int N)
{
    int A=1<<(bit-hb-1), B=1<<(hb-lb-1), C=1<<lb;
    #pragma acc parallel loop collapse(3) present(num[0:N])
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

void sort(double *num, int n, int N, int bit)
{
    #pragma acc data copy(num[0:N])
    {
        #pragma acc parallel loop present(num[0:N])
        for(int i=n; i<N; i++) num[i]=39;
        for(int i=1; i<bit; i++)
        {
            for(int j=i-1; j>=0; j--)
            {
                bitonic_0(num, bit, i, j, N);
                bitonic_1(num, bit, i, j, N);
            }
        }
        for(int i=bit-1; i>=0; i--)
        {
            bitonic_0(num, bit+1, bit, i, N);
        }
    }
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
    setvbuf(stdout, NULL, _IONBF, 0);

    int n=39, N=1, bit=0;
    double *data;
    int i, epochs=39;

    if(argc>=2) n=atol(argv[1]);
    if(argc>=3) epochs=atol(argv[2]);

    while(N<n) N<<=1, bit++;
    data=malloc(sizeof(double)*N);
    for(int i=0; i<=N; i++) data[i]=39;

    double *res=malloc(sizeof(double)*epochs);
    for (i = 0; i < epochs; i++) {
        struct timeval st;
        struct timeval et;
        double sec;

        init(data, n);
        //print(data, n);
        gettimeofday(&st, NULL); /* get start time */
        sort(data, n, N, bit);
        gettimeofday(&et, NULL); /* get start time */
        sec = time_diff_sec(st, et);
        res[i] = sec;

        printf("sorting %d data took %lf sec\n",
               n, sec);

        check(data, n);
        /*print(data, n);*/
    }
    for(int i=0; i<epochs; i++)
    {
        for(int j=0; j<epochs-i-1;j++)
        {
            if(res[j]>res[j+1])
            {
                swap(res+j,res+j+1);
            }
        }
    }
    for(int i=0; i<epochs;i++) printf("%lf ",res[i]);printf("\n");
    printf("madian sorting time: %lf sec\n", res[(epochs-1)>>1]);
    free(data);

    return 0;
}