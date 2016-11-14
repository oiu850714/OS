#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

using std::sort;

#define NUM_THREADS 16

struct array
{
	int *begin;
	int *end;
};
// struct denoting an array's range, starting from *begin until but
// not including *end

int shared_array[150000];
int shared_array2[150000];
sem_t sem[16];
//!!!!!!!!!!!!!!!!!!!! sem[0] is for round!!!!!!!!!!!!!!
sem_t call_main[8];
//thread 8~15 use call_main to let main report result
array Array[16];
//Array store sub array's range


void* thread_start(void* p);
void swap(int* const first, int* const second);
int* select_pivot(int *begin, int *end, int round);
void output_file(int num_of_elements, const char *file);
void cal_time(int num_of_elements, const char *mode, const char *file, int choose_aray);
void bubble_sort(int *begin, int *end);

int main(int argc, char const *argv[])
{
	char file[257];
	scanf("%s", file);
	FILE * fp = fopen(file,"r");
	//file name
	if(!fp){
        printf("file doesn't exist\n");
        return 1;
    }

    int num_of_elements;
	fscanf(fp, "%d", &num_of_elements);
	//scan number of elements 

	for(int i = 0; i < num_of_elements; ++i)
	{
		fscanf(fp, "%d", shared_array + i);
	}
	for(int i = 0; i < num_of_elements; ++i)
	{
		shared_array2[i] = shared_array[i];
	}
	//scan all elements
	fclose(fp);

	if(num_of_elements < 1000)
	{
		cal_time(num_of_elements, "multi thread", "output1.txt", 0);
		cal_time(num_of_elements, "signal thread", "output2.txt", 1);
		return 0;
	}
	// initial semaphore
	// first element is for round variable!!!!!!!
	for(int i = 0; i < NUM_THREADS; ++i)
	{
		sem_init(sem + i, 0, 0);
		i < 8 ? sem_init(call_main + i, 0, 0) : false;
	}


	timeval start;
	gettimeofday(&start, 0);

	//create thread ID array
	pthread_t tid[ NUM_THREADS ];
	//i start from 1, because I don't use first element
	for(int i = 1; i < NUM_THREADS; ++i)
	{
		int check = 0;
		if( check = pthread_create(tid+i, NULL, thread_start, (void*)&i) )
		{
			perror("Can't create thread\n");
			exit(-1);
		}
		sem_wait(sem);
		//this wait is very important!!!
		//that let thread just be created can read i's correct content 
		//printf("what the hell?? i is? %d\n", i);
	}

	//printf("thread all created!!\n");

	Array[1].begin = shared_array;
	Array[1].end = shared_array + num_of_elements;
	sem_post(sem+1);
	// this sem_post can let thread 1 start

	//printf("main thread signal!!\n");

	for(int i = 0; i < 8; ++i)
	{
		sem_wait(call_main + i);
		//printf("call main %d finished!!\n", i);
	}
	//printf("main thread finish waiting 8~15 thread!!\n");
    
    timeval end;
    gettimeofday(&end, 0);
    int sec,usec;
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    printf("multi thread\n");
    printf("Elapsed time: %f ms \n", (sec*1000+(usec/1000.0)));
	output_file(num_of_elements, "output1.txt");

	cal_time(num_of_elements, "signal thread", "output2.txt", 1);
	return 0;
}


//function to select pivot
void* thread_start(void* p)
{
	int round = *((int*)p);
	sem_post(sem);
	//sem_post above unlock pointer p
	//printf("round: %d\n", round);
	sem_wait(sem + round);
	//printf("round wait finished: %d\n", round);
	if(round < 8)
	{
		//printf("thread %d start select_pivot!!\n", round);
		int *pivot = select_pivot(Array[round].begin, Array[round].end, round);
		//printf("thread %d select finished\n", round);
		//printf("%d pivot!!!\n", *pivot);
		//select pivot and signal such that child can sort
		Array[2 * round].begin = Array[round].begin;
		Array[2 * round].end = pivot;
		sem_post(sem + 2 * round);
		Array[2 * round + 1].begin = pivot + 1;
		Array[2 * round + 1].end = Array[round].end;
		sem_post(sem + 2 * round + 1);
	}
	else
	{
		bubble_sort(Array[round].begin, Array[round].end);
		//printf("thread %d really sorting\n", round);
		sem_post(call_main + round - 8);
		//use lib sort and signal such that mother thread can report
	}
	//printf("thread %d finished and signaled!!\n", round);
}

//swap
void swap(int* const first, int* const second)
{
	int temp = *first;
	*first = *second;
	*second = temp;
}

//fuction to sort 8 arrays
int* select_pivot(int *begin, int *end, int round)
{
	int * const pivot = end - 1;
	//the last element is *(end -1);
	int *pindex = begin;
	for(int *i = begin; i != end -1; ++i)
	{
		if(*i <= *pivot)
		{
			swap(pindex, i);
			++pindex;
		}
		if( i == end -1)
		{
			//printf("last element!!!!\n");
		}
	}
	//printf("safe!!!!!!!\n");
	swap(pindex, pivot);
	//printf("round %d in select_pivot finished!!!!!\n", round);
	return pindex;
}

void output_file(int num_of_elements, const char* file)
{
	FILE *fout = fopen(file,"w");
	for(int i = 0; i < num_of_elements ; ++i)
	{
		fprintf(fout, "%d", shared_array[i]);
		if (i + 1 <num_of_elements)
		{
			fprintf(fout, " ");
		}
		else
		{
			fprintf(fout, "\n");
		}
	}
}

void cal_time(int num_of_elements, const char * mode, const char * file, int choose_aray)
{
	int sec,usec;
	timeval start, end;
	gettimeofday(&start, 0);
    choose_aray == 0 ? 
    	bubble_sort(shared_array, shared_array + num_of_elements):
    	bubble_sort(shared_array2, shared_array2 + num_of_elements);
    gettimeofday(&end, 0);
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    printf("%s\n", mode);
    printf("Elapsed time: %f ms \n", (sec*1000+(usec/1000.0)));
	output_file(num_of_elements, file);
}

void bubble_sort(int *begin, int *end)
{
	int size = end - begin;
	for(int i = 0; i < size; i++)
	{
		for(int j = 0; j < i; j++)
		{
			if(*(begin+i) > *(begin+j))
			{
				swap(begin+i, begin+j);
			}
		}
	}
}