#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <queue>

using std::sort; using std::queue;

#define MAX_NUM_THREADS 8

struct array
{
	int *begin;
	int *end;
};
// struct denoting an array's range, starting from *begin until but
// not including *end

queue<int> job_queue;

int shared_array[1500000];
int shared_array2[1500000];
sem_t sem[16];
//!!!!!!!!!!!!!!!!!!!! sem[0] is for round!!!!!!!!!!!!!!
sem_t call_main[8];
//thread 8~15 use call_main to notice main report result
array Array[16];
//Array store sub array's range

sem_t mutex_job_queue;
sem_t mutex_start_job;


void* thread_HW4(void *task);
void* thread_start(void* p);
void swap(int* const first, int* const second);
int* select_pivot(int *begin, int *end);
void output_file(int num_of_elements, const char *file);
void cal_time(int num_of_elements, int pool_size);
void bubble_sort(int *begin, int *end);
void switch_output_size(int num_of_elements, int pool_size);

int main(int argc, char const *argv[])
{

	for(int pool_size = 1; pool_size <= 8; pool_size++)
	{
		FILE * fp = fopen("input.txt","r");
		//file name
		if(!fp)
		{
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

		fclose(fp);



		//HW4: case not using threads to sort, need modity interface.
		if(num_of_elements < 1000)
		{
			cal_time(num_of_elements, pool_size);
			continue;
		}



		// initial semaphore
		// first element is for round variable!!!!!!!
		/*
		for(int i = 0; i < MAX_NUM_THREADS; ++i)
		{
			sem_init(sem + i, 0, 0);
			i < 8 ? sem_init(call_main + i, 0, 0) : false;
		}*/
		//HW4: HW4 may not need these semaphores


		//create thread pool
		pthread_t tid[ MAX_NUM_THREADS ];
		for(int i = 0; i < pool_size; ++i)
		{
			int check = 0;
			if( check = pthread_create(tid+i, NULL, thread_HW4, NULL ) )
			{
				perror("Can't create thread\n");
				exit(-1);
			}
		}


		timeval start;
		gettimeofday(&start, 0);
		//printf("thread all created!!\n");

		Array[1].begin = shared_array;
		Array[1].end = shared_array + num_of_elements;
		//sem_post(sem+1);
		// this sem_post can let thread 1 start


		job_queue.push(1);
		sem_post(&mutex_start_job);
		sem_post(&mutex_job_queue);
		//push first job
		//and notice first thread to start working

		//printf("main thread signal!!\n");

		for(int i = 0; i < 8; ++i)
		{
			sem_wait(call_main + i);
			//printf("call main %d finished!!\n", i);
		}
		// HW4: main in this HW4 may need another way
		// to check that array is sorted

		//printf("main thread finish waiting 8~15 thread!!\n");
	    
	    timeval end;
	    gettimeofday(&end, 0);
	    int sec,usec;
	    sec = end.tv_sec - start.tv_sec;
	    usec = end.tv_usec - start.tv_usec;
	    printf("pool_size: %d\n", pool_size);
	    printf("Elapsed time: %f ms \n", (sec*1000+(usec/1000.0)));
		switch_output_size(num_of_elements, pool_size);


		for(int i = 0; i < pool_size; i++)
		{
			pthread_cancel(*(tid+i));
		}
	}
	return 0;
}


void* thread_HW4(void *p)
{
	while(1)
	{
		sem_wait(&mutex_start_job);
		sem_wait(&mutex_job_queue);
		int i = job_queue.front();
		job_queue.pop();
		sem_post(&mutex_job_queue);
		
		thread_start((void*)(&i));
		
		sem_wait(sem);
		//same trick in HW3
		
		if(i < 8)
		{
			sem_wait(&mutex_job_queue);
			job_queue.push(2*i);
			job_queue.push(2*i + 1);
			sem_post(&mutex_start_job);
			sem_post(&mutex_start_job);
			sem_post(&mutex_job_queue);
		}
	}
}

//function to select pivot
void* thread_start(void* p)
{
	int round = *((int*)p);
	sem_post(sem);
	//sem_post above unlock pointer p
	//printf("round: %d\n", round);
	//sem_wait(sem + round);
	//HW4: I think line above is not reqeired.
	//printf("round wait finished: %d\n", round);
	if(round < 8)
	{
		//printf("thread %d start select_pivot!!\n", round);
		int *pivot = select_pivot(Array[round].begin, Array[round].end);
		//printf("thread %d select finished\n", round);
		//printf("%d pivot!!!\n", *pivot);
		//select pivot and signal such that child can sort
		Array[2 * round].begin = Array[round].begin;
		Array[2 * round].end = pivot;
		//sem_post(sem + 2 * round);
		//HW4: I think line above is not reqeired.
		Array[2 * round + 1].begin = pivot + 1;
		Array[2 * round + 1].end = Array[round].end;
		//sem_post(sem + 2 * round + 1);
		//HW4: I think line above is not reqeired.
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
int* select_pivot(int *begin, int *end)
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
		//fprintf(stdout, "%d", shared_array[i]);
		fprintf(fout, "%d", shared_array[i]);
		if (i + 1 <num_of_elements)
		{
			//fprintf(stdout, " ");
			fprintf(fout, " ");
		}
		else
		{
			//fprintf(stdout, "\n");
			fprintf(fout, "\n");
		}
	}
}

void cal_time(int num_of_elements, int pool_size)
{
	int sec,usec;
	timeval start, end;
	gettimeofday(&start, 0);
    bubble_sort(shared_array, shared_array + num_of_elements);
    gettimeofday(&end, 0);
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    printf("pool size: %d\n", pool_size);
    printf("Elapsed time: %f ms \n", (sec*1000+(usec/1000.0)));
	switch_output_size(num_of_elements, pool_size);
}

void bubble_sort(int *begin, int *end)
{
	int size = end - begin;
	for(int i = size; i-- ;)
	{
		for(int j = 0; j < i-1; j++)
		{
			if(*(begin+j) > *(begin+j+1))
			{
				swap(begin+i, begin+j);
			}
		}
	}
}

void switch_output_size(int num_of_elements, int pool_size)
{
	switch(pool_size){
		case 1:
			output_file(num_of_elements, "output_1.txt");
			break;
		case 2:
			output_file(num_of_elements, "output_2.txt");
			break;
		case 3:
			output_file(num_of_elements, "output_3.txt");
			break;
		case 4:
			output_file(num_of_elements, "output_4.txt");
			break;
		case 5:
			output_file(num_of_elements, "output_5.txt");
			break;
		case 6:
			output_file(num_of_elements, "output_6.txt");
			break;
		case 7:
			output_file(num_of_elements, "output_7.txt");
			break;
		case 8:
			output_file(num_of_elements, "output_8.txt");
			break;
		}
}