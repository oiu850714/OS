#include <stdlib.h>
#include <stdio.h>
int main(int argc, char const *argv[])
{
	char buffer[4097];
	FILE *f_old_data = fopen("data.txt", "r");
	int line = 0;
	while( fgets(buffer, 4097, f_old_data) )
	{
		line++;
	}
	FILE *f_new_data = fopen("new_data.txt", "w");
	fprintf(f_new_data, "%d\n", line);
	//count line of data.txt and write line to new_data.txt's first line
	
	fseek(f_old_data, 0, SEEK_SET);
	while( fgets(buffer, 4097, f_old_data) )
	{
		buffer[4] = '\0';
		fprintf(f_new_data, "%s", buffer);
	}
	fprintf(f_new_data, "\n");
	//from second line, write all keys altogether in the begin of new_data.txt
	//and print newline at last

	fseek(f_old_data, 0, SEEK_SET);
	while( fgets(buffer, 4097, f_old_data) )
	{
		fprintf(f_new_data, "%s", buffer);
	}
	//print data.txt's content after new_data.txt's key cluster
	return 0;
}