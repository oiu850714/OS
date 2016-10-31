#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>


const int MAX_SIZE = 256001;
bool pipe_flag = 0, redirection_flag = 0, not_wait_flag = 0, no_program = 0;
const char *PIPE = "|", *redirection = ">", *not_wait = "&";

void reset_flag()
{
	pipe_flag = 0, redirection_flag = 0, not_wait_flag = 0, no_program = 0;
}

// read input, and delete last '\n' character
void read_input(char *input)
{
	printf(">");
	fgets(input, MAX_SIZE, stdin);
	input[strlen(input)-1]='\0';
	//input[strlen(input)-1] is '\n'
}

int parse_arg(char *input, char **arg)
{
	read_input(input);
	char *token = strtok(input, " \t");
	if(token == NULL)
	{
		no_program = 1;
		return 0;
	}
	else if(strcmp(token, not_wait) == 0)
	{
		not_wait_flag = 1;
		return 0;
	}
	arg[0] = new char[100];
	strcpy(arg[0], token);
	int i = 1;
	while(true)
	{
		token = strtok(NULL, " \t");
		if(token == NULL)
		{
			return i;
		}
		//case when last argument is "&"
		else if(strcmp(token, not_wait) == 0)
		{
			not_wait_flag = 1;
			return i;
		}
		else
		{
			arg[i] = new char[100];
			strcpy(arg[i], token);
			++i;
		}
	}
}

void delete_arg(char **arg)
{
	int i = 0;
	while(true)
	{
		if(arg[i] != NULL)
			delete arg[i];
		else
			break;
		++i;
	}
}

int main(int argc, char const *argv[])
{
	// The handler deals with zombie process.
	signal (SIGCHLD,SIG_IGN);
	while(true)
	{
		char input[MAX_SIZE], *arg[1000] = {NULL};
		const int last_arg = parse_arg(input, arg);
		pid_t pid = fork();
		if(pid < 0)
		{
			fprintf(stderr, "Fork failed\n");
			exit(-1);
		}
		else if(pid == 0)
		{
			if(no_program == 1)
			{
				exit(0);
			}
			else
			{
				execvp(arg[0], arg);
			}
		}
		else
		{
			if( !not_wait_flag )
			{
				waitpid(pid, NULL, 0);
			}
			reset_flag();
			delete_arg(arg);
		}
	}
	return 0;
}