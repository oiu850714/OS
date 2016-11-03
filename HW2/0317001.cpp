#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

int create_seg_and_check(int);
void initialize_array(unsigned int *, int );
void cal_matrix(unsigned int *, unsigned int *,int , int , int, int);
void print_array(unsigned int *,int dimemsion, int i, int dimemsion_per_proc);
void clear_array(unsigned int *, int dimemsion);
int main(int argc, char const *argv[])
{
	//first process won't wait any child,
	//so must be deals with zombie process
	signal (SIGCHLD,SIG_IGN);

	//scan input
	printf("Input the matrix dimemsion: \n");
	int dimemsion;
	scanf("%d", &dimemsion); 

	// use shmget and shmat to allocate and attach shared memory
	int shmid_input = create_seg_and_check(dimemsion);
	int shmid_result = create_seg_and_check(dimemsion);
	unsigned int * array_input = (unsigned int *)shmat(shmid_input, 0, 0); 
	unsigned int * array_result = (unsigned int *)shmat(shmid_result, 0, 0);

	//initialize array's value
	initialize_array(array_input, dimemsion);

	pid_t pid[16];

	//computing...
	for(size_t num_of_process = 1; num_of_process < 17; ++num_of_process)
	{
		printf("Multiplying matrices using %zu process\n", num_of_process);
		struct timeval start;
		gettimeofday(&start, 0);

		int dimemsion_per_proc = dimemsion / num_of_process;
		for(size_t i = 0; i < num_of_process; ++i)
		{
			pid[i] = fork();

			//keep where in the matrix the child starts compute
			if(pid[i] < 0)
			{
				perror("fork gg\n");
				exit(-1);
			}
			else if(pid[i] == 0)
			{
				if( i == num_of_process-1 )
				//case that there may be some dimemsion will not be calculate
				//force last process to cal the rest of dimemsion
				{
					cal_matrix(array_input, array_result, dimemsion,
					dimemsion_per_proc, dimemsion_per_proc*i,
					dimemsion);
				}
				else
				{
					cal_matrix(array_input, array_result, dimemsion,
					dimemsion_per_proc, dimemsion_per_proc*i,
					dimemsion_per_proc*(i+1));
				}
				shmdt(array_input);
				shmdt(array_result);
				//print_array(array_result, dimemsion, i, dimemsion_per_proc);
				exit(0);
			}
		}
		for(int i = 0; i < num_of_process; ++i)
		{
			wait(NULL);
		}

		struct timeval end;
		gettimeofday(&end, 0);
		int sec,usec;
		sec = end.tv_sec - start.tv_sec;
        usec = end.tv_usec - start.tv_usec;
        printf("Elapsed time: %f s", (sec*1000+(usec/1000.0))/1000);
        unsigned int sum = 0;
        for (int i = 0; i < dimemsion*dimemsion; ++i)
        {
        	sum += array_result[i];
        }
        printf(",Checksum: %u\n", sum);

        //make sure each calculation is correct, so clear array everytime.
        clear_array(array_result, dimemsion);
	}


	shmdt(array_input);
	shmdt(array_result);
	shmctl(shmid_input, IPC_RMID, NULL);
	shmctl(shmid_result, IPC_RMID, NULL);	
	
	return 0;
}

int create_seg_and_check(int dimemsion)
{
	int shmid;
	if( (shmid = shmget(IPC_PRIVATE,sizeof(unsigned int) * dimemsion * dimemsion, SHM_W|SHM_R|IPC_CREAT)) == -1)
	{
		perror("fuck you!\n");
		exit(1);
	}
	else
		return shmid;
}

void initialize_array(unsigned int *A1, int dimemsion)
{
	for(size_t i = 0; i < dimemsion; ++i)
	{
		for(size_t j = 0; j < dimemsion; ++j)
		{
			A1[dimemsion*i + j] = dimemsion*i + j;
		}
	}
}

void cal_matrix(unsigned int * input, unsigned int * result, int dimemsion, int dimemsion_per_proc, int start, int end)
{
	for(int i = start; i < end; i++)
	{
		for(int j = 0; j < dimemsion; j++)
		{
			result[i*dimemsion + j] = 0;
			for(int k = 0; k < dimemsion; k++)
			{
				result[i*dimemsion + j] += 
					input[i*dimemsion + k] * input[k*dimemsion + j];
			}
		}
	}
}

void clear_array(unsigned int *array, int dimemsion)
{
	for(int i = 0; i < dimemsion*dimemsion; i++)
		array[i] = 0;
}