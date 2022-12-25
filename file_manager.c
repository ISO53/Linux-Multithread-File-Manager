#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define TRUE 1
#define FALSE 0
#define MAX_THREAD_CAPACITY 5
#define MAX_FILE_CAPACITY 10
#define MAIN_FIFO_NAME "MAIN_FIFO"
#define MAX_BUFFER_LENGTH 100

typedef struct
{
	int size;
	int capacity;
	void **items; // array of void pointers
} ArrayList;
ArrayList *create_list(int maxCapacity);
void destroy_list(ArrayList *list);
int add_item(ArrayList *list, void *item);
void remove_item(ArrayList *list, int index);
void *get_item(ArrayList *list, int index);

void readMainNamedPipe();
void readClientThread();
void uniquePipeName(char *pipeName);

ArrayList *threadList;
ArrayList *fileList;

int main(int argc, char const *argv[])
{

	threadList = create_list(MAX_THREAD_CAPACITY);
	fileList = create_list(MAX_FILE_CAPACITY);

	// Main named pipe for newly created processes to connect
	int result = mkfifo(MAIN_FIFO_NAME, 0666);
	if (result < 0)
	{
		perror("Error occured while creating named pipe!\n");
		exit(EXIT_FAILURE);
	}

	while (TRUE)
	{
	}

	// pthread_t mainThread;
	// pthread_create(&mainThread, NULL, listenUntilExit, NULL);

	destroy_list(threadList);
	destroy_list(fileList);

	return 0;
}

//************************************* Array List Functions *************************************
ArrayList *create_list(int maxCapacity)
{
	ArrayList *list = malloc(sizeof(ArrayList));
	list->size = 0;
	list->capacity = maxCapacity;
	list->items = malloc(sizeof(void *) * list->capacity);

	for (int i = 0; i < list->capacity; i++)
	{
		list->items[i] = NULL;
	}

	return list;
}

void destroy_list(ArrayList *list)
{
	free(list->items);
	free(list);
}

int add_item(ArrayList *list, void *item)
{
	for (int i = 0; i < list->capacity; i++)
	{
		if (list->items[i] == NULL)
		{
			list->items[i] = item;
			return 1;
		}
	}
	return 0;
}

void remove_item(ArrayList *list, int index)
{
	list->items[index] = NULL;
}

void *get_item(ArrayList *list, int index)
{
	return list->items[index];
}
//********************************** Array List Functions Ends ***********************************

/**
 * Constantly read the main named pipe to check if any process trying to connect. If so, create
 * another pipe just for that process and start listen that pipe on another thread.
 */
void readMainNamedPipe()
{
	// Open the main named pipe for reading
	int fd = open(MAIN_FIFO_NAME, O_RDONLY);
	if (fd < 0)
	{
		perror("Error occured while opening named pipe for reading!\n");
		exit(1);
	}

	while (TRUE)
	{
		// Read data from the main named pipe
		char buffer[MAX_BUFFER_LENGTH];
		int n = read(fd, buffer, MAX_BUFFER_LENGTH);
		if (n < 0)
		{
			perror("Error occured while reading from named pipe!\n");
			return EXIT_FAILURE;
		}

		if (strlen(buffer) > 0)
		{
			// A client wrote something to the pipe.
			printf("Client wrote: %s\n", buffer);

			// Create a named pipe for that client
			char pipeName[1];
			uniquePipeName(pipeName);

			int result = mkfifo(pipeName, 0666);
			if (result < 0)
			{
				perror("Error occured while creating named pipe!\n");
				exit(EXIT_FAILURE);
			}

			// Send that pipe name back to the client to listen
			printf("New Pipe Name: %s\n", pipeName);

			// First close the named pipe that has been opened for reading
			close(fd);

			// Then open it in write mode
			int fd = open(MAIN_FIFO_NAME, O_WRONLY);
			if (fd < 0)
			{
				perror("Error occured while opening named pipe for reading!\n");
				exit(1);
			}
			// Write data to the named pipe
			int n = write(fd, buffer, strlen(buffer));
			if (n < 0)
			{
				perror("Error writing to named pipe");
				exit(1);
			}

			// Create a new thread for that client
			pthread_t thread;
			pthread_create(&thread, NULL, readClientThread, NULL);
			add_item(threadList, &thread);
		}
	}

	// Close the named pipe
	close(fd);
}

void readClientThread()
{
}

void uniquePipeName(char *pipeName)
{
	int numberOfThread = threadList->size;
	sprintf(pipeName, "%d", numberOfThread);
	pipeName[1] = '\0';
}