#include<stdlib.h>
#include<stdio.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include<wait.h>
#include<sys/time.h>
#include<sys/resource.h>

#define RECORD_SIZE 4096
#define KEY_SIZE 4
int search(char* addr , char *keyword, int start, int end, int num_case)
{
    //int low=0,  high= (leng/RECORD_SIZE)-1, ret;
    int low=0,  high= num_case-1, ret;
    static char buffer[4100];
    static char tmp[5];
    tmp[4] = '\0';
    while (low <= high)
    {
        int mid = (low + high) / 2;
        strncpy(tmp, (addr + mid * KEY_SIZE) , 4);
        ret = strcmp(tmp, keyword);
        if (ret == 0)
        {
            memset(buffer, '\0', sizeof(buffer));
            //strncpy(buffer, addr + RECORD_SIZE * mid + 4 + start,  end - start + 1);
            strncpy(buffer, addr + (4 * num_case) + 1 + RECORD_SIZE * mid + 4 + start,  end - start + 1);
            printf("key %s found : %s\n", keyword, buffer);
            return mid;
        } else if (ret > 0)
        {
            high = mid - 1;
        } else 
        {
            low = mid + 1;
        }
    }
    printf("key %s not found\n", keyword);
    return -1;
}
void print_max_rss()
{
    struct rusage r_stat;
    getrusage(RUSAGE_SELF, &r_stat);
    printf("max rss = %ld\n", r_stat.ru_maxrss);    
}

int main(int argc, char* argv[])
{
    char *addr;
    int dataset_fd;
    struct stat sb;
    int N, start, end, ret;
    FILE *testcase ;
    char key[5];
    key[4] = '\0';
    dataset_fd = open("new_data.txt", O_RDWR);
    testcase = fopen(argv[1], "r");
    fstat(dataset_fd, &sb);
    N = sb.st_size; 
    addr =  mmap(NULL, N , PROT_READ|PROT_WRITE , MAP_PRIVATE, dataset_fd, 0);
    // first parameter NULL let kernel select appropriate starting address

    char num_case_string[11];
    int num_case_leng = 0;
    while(addr[num_case_leng] != '\n')
   	{
   		num_case_string[num_case_leng] = addr[num_case_leng];
   		++num_case_leng;
   	}
   	int num_case = atoi(num_case_string);

   	char *start_key_cluster = addr + num_case_leng + 1 /* newline */;

    while( fscanf(testcase, "%4s %d %d\n", key, &start, &end)!= EOF)
    {
        search(start_key_cluster, key, start , end, num_case);
    }
    munmap(addr, N);
    print_max_rss();
    close(dataset_fd);
    return 0;

}
