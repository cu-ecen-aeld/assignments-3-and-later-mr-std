/* This is the C writer for the Assignment 2 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>


int main(int argc, char **argv)
{
	openlog(NULL, 0, LOG_USER);

	if(argc != 3)
	{
		syslog(LOG_ERR, "Invalid Number of arguments: %d", argc);
		return 1;
	}
	
	char *fileName = argv[1];
	char *str_to_write = argv[2];

	/* Open the file and write to it */
	FILE *file_desc = fopen(fileName, "w");

	if(file_desc == NULL)
	{
		syslog(LOG_ERR, "Error opening the file %s", fileName);
		return 1;
	}
	
	syslog(LOG_DEBUG, "Writing %s to %s", str_to_write, fileName);
	fprintf(file_desc, "%s", str_to_write);

	/* Closing the file */
	fclose(file_desc);

	closelog();

	return 0;
}
