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

int *shared_array;
sem_t sem[16];
//!!!!!!!!!!!!!!!!!!!! sem[0] is for round!!!!!!!!!!!!!!
sem_t call_main[8];
//thread 8~15 use call_main to notice main report result
array Array[16];
//Array store sub array's range

sem_t mutex_job_queue;
sem_t mutex_start_job;

void merge(int* begin_1, int* end_1, int* begin_2, int* end_2);

void* thread_HW4(void *p);
//thread's entry point

void* thread_start(void* p);
//HW3's entry point

void swap(int* const first, int* const second);
//swapping two elements pointed by first and second

int* select_pivot(int *begin, int *end);
//selecting the pivot in the array 
//beginning with begin ending with end(not encluded)
//and partition this array

void output_file(int num_of_elements, const char *file);
// output result to FILE *file

void cal_time(int num_of_elements, int pool_size);
//used in case when num_of_elements < 1000

void bubble_sort(int *begin, int *end);
//fucking bubble sort..

void switch_output_size(int num_of_elements, int pool_size);
//using pool_size in switch case to know what filename should use

int main(int argc, char const *argv[])
{

	for(int pool_size = 1; pool_size <= 8; pool_size++)
	{
		sem_init(&mutex_job_queue, 0, 1);
		sem_init(&mutex_start_job, 0, 0);
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
		shared_array = (int*)malloc(num_of_elements * sizeof(int));

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

		sem_wait(&mutex_job_queue);
		job_queue.push(1);
		sem_post(&mutex_job_queue);
		sem_post(&mutex_start_job);
		//push first job
		//and notice first thread to start working

		//printf("main thread signal!!\n");

		for(int i = 0; i < 8; ++i)
		{
			sem_wait(call_main + i);
			//HW4 not print: printf("job %d finished!!\n", i + 8);
		}
		// HW4: main in this HW4 may need another way
		// to check that array is sorted

		for(int i = 9; i <= 15; i++)
		{
			//HW4 not print: printf("merge %dth!!!!!!!!!!\n", i-8);
			merge(Array[8].begin, Array[i-1].end, Array[i].begin, Array[i].end);
		}
		//printf("main thread finish waiting 8~15 thread!!\n");
	    
	    timeval end;
	    gettimeofday(&end, 0);
	    int sec,usec;
	    sec = end.tv_sec - start.tv_sec;
	    usec = end.tv_usec - start.tv_usec;
	    printf("pool_size: %d\n", pool_size);
	    printf("Elapsed time: %f ms \n", (sec*1000+(usec/1000.0)));
		switch_output_size(num_of_elements, pool_size);

		//HW4 not print: printf("poolsize %d finished\n", pool_size);
		for(int i = 0; i < pool_size; i++)
		{
			pthread_cancel(*(tid+i));
		}
		for(int i = 0; i < 16; i++)
		{
			Array[i].begin = Array[i].end = NULL;	
		}
		free(shared_array);
	}
	return 0;
}


void* thread_HW4(void *p)
{
	while(1)
	{
		sem_wait(&mutex_start_job);
		sem_wait(&mutex_job_queue);
		int round = job_queue.front();
		job_queue.pop();
		//printf("take job %d and pop it!\n", round);
		sem_post(&mutex_job_queue);
		
		//HW4:thread_start((void*)(&i));
		
		//HW4:sem_wait(sem);
		//same trick in HW3
		if(round < 8)
		{
			//printf("thread %d start select_pivot!!\n", round);
			int empty_flag = 0;
			if(Array[round].begin == Array[round].end)
			{
				//HW4 not print: printf("job %d empty!!!\n", round);
				Array[2*round].begin = Array[2*round].end = Array[round].begin;
				Array[2*round+1].begin = Array[2*round+1].end = Array[round].begin;
				empty_flag =1;
			}
			if(!empty_flag)
			{
				int *pivot = Array[round].begin + (Array[round].end - Array[round].begin)/2;
				//int *pivot = select_pivot(Array[round].begin, Array[round].end);
				//HW4 not print: printf("round : %d pivot: %d begin: %d end: %d\n", round, pivot, Array[round].begin, Array[round].end );
				//printf("thread %d select finished\n", round);
				//printf("%d pivot!!!\n", *pivot);
				//select pivot and signal such that child can sort
				Array[2 * round].begin = Array[round].begin;
				Array[2 * round].end = pivot;
				//sem_post(sem + 2 * round);
				//HW4: I think line above is not reqeired.
				Array[2 * round + 1].begin = pivot;
				Array[2 * round + 1].end = Array[round].end;
				//sem_post(sem + 2 * round + 1);
				//HW4: I think line above is not reqeired.
			}
			sem_wait(&mutex_job_queue);
			job_queue.push(2*round);
			job_queue.push(2*round + 1);
			sem_post(&mutex_job_queue);
			sem_post(&mutex_start_job);
			sem_post(&mutex_start_job);
			//must put above lines here, otherwise..
			//there maybe this case:
			//thread_A and thread_B will push round, A pushes before B,
			//but B finishes working before A, and when B finishes, A not
			//yet, then B's two child thread will take A's 
			//not-yet-partitioned Array, which make all things fucking shit.
			//In short, you must be finished job before pushing new job...
			//printf("job %d start pushing new job %d\n", round, 2*round);
			//sem_post(&mutex_start_job);
			//printf("job %d start pushing new job %d\n", round, 2*round+1);
			//sem_post(&mutex_start_job);
			//sem_post(&mutex_job_queue);
			//HW4 not print: printf("job %d push new jobs in queue!\n",round );
			//printf("%d really has sorted array\n", round);
		}
		else
		{
			//HW4 not print: printf("job %d start sorting\n", round );
			/*
			int *tmp_array = (int*)malloc((Array[round].end - Array[round].begin) * sizeof(int));
			for(int i = 0; i < (Array[round].end - Array[round].begin); i++)
			{
				tmp_array[i] = *(Array[round].begin + i);
			}
			bubble_sort(tmp_array, tmp_array + (Array[round].end - Array[round].begin));
			//printf("thread %d really sorting\n", round);
			for (int i = 0; i < (Array[round].end - Array[round].begin); ++i)
			{
				*(Array[round].begin + i) = tmp_array[i];
			}
			free(tmp_array);
			*/
			bubble_sort(Array[round].begin, Array[round].end);
			sem_post(call_main + round - 8);
			//use lib sort and signal such that mother thread can report
		}
		if(round < 8)
		{
			
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
	int * pindex = begin;
	if(end - begin >= 3)
	{
		int a = *(end-1), b = *(end-2), c = *(end-3);
		int first, second, third;
		if(a < b)
		{
			third = a;
			second = b;
		}
		else
		{
			second = a;
			third = b;
		}
		if(second < c)
		{
			first = c;
		}
		else
		{
			first = second;
			if(third < c)
			{
				second = c;
			}
			else
			{
				second = third;
				third = c;
			}
		}
		*(end - 1) = second;
		*(end - 2) = third;
		*(end - 3) = first;
	}
	//the last element is *(end -1);
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
    sort(shared_array, shared_array + num_of_elements);
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
	for(int i = size; i; --i)
	{
		for(int j = 0; j < i-1; j++)
		{
			if(*(begin+j) > *(begin+j+1))
			{
				swap(begin+j, begin+j+1);
				//counter++;
				//counter == 100 ? printf("%d",round), counter = 0 : false;
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


void merge(int* begin_1, int* end_1, int* begin_2, int* end_2)
{
	int length_l = end_1 - begin_1;
	int length_r = end_2 - begin_2;
	int i = 0, j = 0, k = 0;
	int* tmp_array = (int*)malloc(sizeof(int) *( length_l + length_r ) );
	while(i < length_l && j < length_r)
	{
		if( begin_1[i] <= begin_2[j])
		{
			tmp_array[k] = begin_1[i++];
		}
		else
		{
			tmp_array[k] = begin_2[j++];
		}
		++k;
	}
	while(i < length_l)
	{
		tmp_array[k] = begin_1[i++];
		++k;
	}
	while(j < length_r)
	{
		tmp_array[k] = begin_2[j++];
		++k;
	}
	for(int i = 0; i < length_l; i++)
	{
		begin_1[i] = tmp_array[i];
		//HW4 not print: printf("%d ", begin_1[i]);
	}
	//HW4 not print: printf("\n");
	for(int i = 0; i < length_r; i++)
	{
		begin_2[i] = tmp_array[length_l+i];
		//HW4 not print: printf("%d ", begin_2[i]);
	}
	//HW4 not print: printf("\n");
	free(tmp_array);
}