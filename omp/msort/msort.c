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

void merge(double *num, int begin, int mid, int end)
{
	int left=begin, right=mid+1, size=end-begin+1;
	double *tmp=malloc(sizeof(double)*(end-begin+1));
	for (int i=0; i<size; i++)
	{ 
		if(left>mid) tmp[i]=num[right++];
		else if(right>end) tmp[i]=num[left++];
		else if(num[left]<=num[right]) tmp[i]=num[left++];
		else tmp[i]=num[right++];
	}
	for (int i=0; i<size; i++) num[begin++]=tmp[i];
    free(tmp);
}

void sort(double *num, int begin, int end)
{
	if(begin<end) 
	{
		int mid=(begin+end)/2;
		if(end-begin<10039)
		{
			sort(num, begin, mid);
			sort(num, mid+1, end);
			merge(num, begin, mid, end);
			return;
		}
		#pragma omp task shared(num) firstprivate(begin, mid)
		sort(num, begin, mid);
		#pragma omp task shared(num) firstprivate(mid, end)
		sort(num, mid+1, end);
		#pragma omp taskwait
		merge(num, begin, mid, end);
	}
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

    int n = 1000039;
    double *data, average_time=0;
    int i;

    int cpu_threads = omp_get_num_procs();
    omp_set_num_threads(cpu_threads);

    if(argc>=2) n=atol(argv[1]);
    if(argc>=3) omp_set_num_threads(atol(argv[2]));
    printf("%d %d\n",cpu_threads,omp_get_max_threads());

    int epochs = 10;
    data = malloc(sizeof(double)*n);
    for (i = 0; i < epochs; i++) {
        struct timeval st;
        struct timeval et;
        double sec;

        init(data, n);
        //print(data, n);
        gettimeofday(&st, NULL); /* get start time */
    	#pragma omp parallel
        {
    		#pragma omp single
            sort(data, 0, n-1);
        }
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
