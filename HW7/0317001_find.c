#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#define MAX_FILE_LENGTH 255
#define MiB 1048576

char path_name[MAX_FILE_LENGTH + 1];
char path_name_cwd[MAX_FILE_LENGTH + 1];

int size_min_flag;
int size_max_flag;
int size_min;
int size_max;

	
int inode_flag;
int inode_num;

int file_flag;
char file_name[MAX_FILE_LENGTH + 1];

void find_file(char *dir_path)
{
	DIR *dp = opendir(dir_path);
	assert(dp != NULL);
	struct dirent *d;
	while ((d = readdir(dp)) != NULL) 
	{
		struct stat tmp_stat;
		char tmp_file_name[MAX_FILE_LENGTH + 1];
		strncpy(tmp_file_name, path_name_cwd, MAX_FILE_LENGTH + 1);
		strncat(tmp_file_name, d->d_name, MAX_FILE_LENGTH + 1 - strlen(path_name_cwd) - strlen(d->d_name));
		lstat(tmp_file_name, &tmp_stat);
		//if not want to chdir, need to pass ${cwd}${d->d_name} in lstat()
		
		if((tmp_stat.st_mode & S_IFMT) == S_IFDIR && 
			strncmp(d->d_name, ".", MAX_FILE_LENGTH + 1) &&
			strncmp(d->d_name, "..", MAX_FILE_LENGTH + 1))
		{
			strncat(path_name_cwd, d->d_name, MAX_FILE_LENGTH + 1 - strlen(path_name_cwd));
			strncat(path_name_cwd, "/", MAX_FILE_LENGTH + 1 - strlen(path_name_cwd));
			// ${cwd} become ${cwd}${d->d_name}/
			
			find_file(path_name_cwd);
			
			path_name_cwd[strlen(path_name_cwd) - strlen(d->d_name) - 1] = '\0';
		}
		

		if(inode_flag)
		{
			if((int)tmp_stat.st_ino != inode_num)
				continue;
		}
		if(file_flag)
		{
			if(strncmp(d->d_name, file_name, MAX_FILE_LENGTH + 1))
				continue;
		}
		if(size_max_flag)
		{
			if((unsigned)tmp_stat.st_size > size_max * MiB )
				continue;
		}
		if(size_min_flag)
		{
			if((unsigned)tmp_stat.st_size < size_min * MiB )
				continue;
		}
		
		printf("%s %d %u\n", tmp_file_name, (int)tmp_stat.st_ino, (unsigned)tmp_stat.st_size/MiB);
	}
	closedir(dp);
}

int main(int argc, char *argv[]) 
{
	assert(argc >= 2 && argc % 2 == 0);
	//assert num of args(including program name) is even (>=2)

	strncpy(path_name, argv[1], MAX_FILE_LENGTH + 1);

	for(int i = 2; i < argc; i+=2)
	{
		switch(argv[i][1])
		{
			case 'i':
				//case in -inode
				inode_flag = 1;
				inode_num = atoi(argv[i+1]);
				break;
			case 'n':
				//case in -name
				file_flag = 1;
				strncpy(file_name, argv[i+1], MAX_FILE_LENGTH + 1);
				break;
			case 's':
				//case in -size
				if(argv[i][7] == 'a')
				{	
					size_max_flag = 1;
					size_max = atoi(argv[i+1]);
					//case in -size_min
				}
				else
				{
					size_min_flag = 1;
					size_min = atoi(argv[i+1]);
					//case in -size_max
				}
				break;
		}
	}

	////// recursive function
	strncpy(path_name_cwd, path_name, MAX_FILE_LENGTH + 1);
	find_file(path_name_cwd);

	return 0;
}

