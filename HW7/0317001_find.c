#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include <string.h>

#define MAX_FILE_LENGTH 256


char path_name[MAX_FILE_LENGTH];

int size_min_max_flag;
int size;
// 1 = max, 2 = min
	
int inode_flag;
int inode_num;

int file_flag;
char file_name[MAX_FILE_LENGTH];

void find_file(char *dir_path)
{
	DIR *dp = opendir(dir_path);
	assert(dp != NULL);
	struct dirent *d;
	while ((d = readdir(dp)) != NULL) 
	{
		printf("%d %s\n", (int) d->d_ino, d->d_name);
	}
	closedir(dp);
}

int main(int argc, char *argv[]) 
{
	/*
	DIR *dp = opendir(".");
	assert(dp != NULL);
	struct dirent *d;
	while ((d = readdir(dp)) != NULL) 
	{
		printf("%d %s\n", (int) d->d_ino, d->d_name);
	}
	closedir(dp);
	*/

	assert(argc >= 2 && argc % 2 == 0);
	//assert num of args(including program name) is even (>=2)

	for(int i = 2; i < argc; i++)
	{
		switch(argv[i][0])
		{
			case 'i':
				//case in -inode
				inode_flag = 1;
				inode_num = atoi(argv[i+1]);
				break;
			case 'n':
				//case in -name
				file_flag = 1;
				strncpy(file_name, argv[i+1], MAX_FILE_LENGTH);
				break;
			case 's':
				//case in -size
				if(argv[i][7] == 'i')
				{	
					size_min_max_flag = 1;
					//case in -size_min
				}
				else
				{
					size_min_max_flag = 2;
					//case in -size_max
				}
				size = atoi(argv[i+1]);
				break;
		}
	}


	////// recursive function
	find_file(argv[1]);


	return 0;
}

