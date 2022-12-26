#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

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
void readClientThread(char *pipeName);

ArrayList *threadList;
ArrayList *fileList;
unsigned long uniqueId;

int main(int argc, char const *argv[])
{

	threadList = create_list(MAX_THREAD_CAPACITY);
	fileList = create_list(MAX_FILE_CAPACITY);
	uniqueId = 0;

	// Main named pipe for newly created processes to connect
	int result = mkfifo(MAIN_FIFO_NAME, 0666);
	if (result < 0)
	{
		perror("Error occured while creating named pipe!\n");
		exit(EXIT_FAILURE);
	}

	// Listen main pipe in another thread
	pthread_t mainThread;
	pthread_create(&mainThread, NULL, readMainNamedPipe, NULL);

	// ARADAKİ İŞLEMŞLER
	// adsa
	// asdasd

	pthread_join(mainThread, NULL);
	for (int i = 0; i < threadList->size; i++)
	{
		pthread_t tempThread = *(pthread_t *)get_item(threadList, i);
		pthread_join(tempThread, NULL);
	}

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
 * @brief Constantly read the main named pipe to check if any process trying to connect. If so, create
 * another pipe just for that process and start listen that pipe on another thread.
 *
 */
void readMainNamedPipe()
{
	while (TRUE)
	{
		// Read data from the main named pipe
		char buffer[MAX_BUFFER_LENGTH];
		readFromPipe(buffer, MAIN_FIFO_NAME);

		// Rewing if there isn't a client trying to connect
		if (!equals(buffer, "connect"))
		{
			continue;
		}

		// A client wrote 'connect' to the pipe.
		printf("Client wrote: %s\n", buffer);

		// Create a named pipe for that client
		int size = log10(uniqueId) + 1;
		char pipeName[size];
		sprintf(pipeName, "%d", uniqueId++);

		int result = mkfifo(pipeName, 0666);
		if (result < 0)
		{
			perror("Error occured while creating named pipe!\n");
			exit(EXIT_FAILURE);
		}

		// Send that pipe name back to the client to listen
		printf("New Pipe Name: %s\n", pipeName);

		// Write data to the named pipe
		writeToPipe(pipeName, MAIN_FIFO_NAME);

		// Create a new thread for that client
		pthread_t thread;
		pthread_create(&thread, NULL, readClientThread, NULL);
		add_item(threadList, &thread);
	}
}

/**
 * @brief Constantly read the given pipe for inputs. This function should be called for each client that
 * connects to this service.
 *
 * @param pipeName
 */
void readClientThread(char *pipeName)
{
	while (TRUE)
	{
		// Open the unique named pipe for reading
		int fd = open(pipeName, O_RDONLY);
		if (fd < 0)
		{
			perror("Error occured while opening named pipe for reading!\n");
			continue;
		}

		// Read data from the unique named pipe
		char buffer[MAX_BUFFER_LENGTH];
		int n = read(fd, buffer, MAX_BUFFER_LENGTH);
		if (n < 0)
		{
			perror("Error occured while reading from named pipe!\n");
			continue;
		}

		printf("%s: %s", pipeName, buffer);
	}
}

/**
 * @brief Tokenizes the given input into an array of strings and returns the number
 * of tokenized elements. If the number doesnt match the length, than there is
 * less token than the given arrays size.
 *
 * @param input
 * @param tokens
 * @param length
 * @return int
 */
int tokenizeInput(char *input, char **tokens, int length)
{
	int counter = 0;
	char *temp = strtok(input, " ");

	while (temp != NULL && counter < length)
	{
		tokens[counter++] = temp;
		temp = strtok(NULL, " ");
	}

	return counter;
}

/**
 * @brief Return true if str1 equals to str2
 *
 * @param str1
 * @param str2
 * @return int
 */
int equals(char *str1, char *str2)
{
	return strcmp(str1, str2) == 0;
}

/**
 * @brief Writes given string to given pipe
 *
 * @param str
 * @param pipeName
 * @return int
 */
int writeToPipe(char *str, char *pipeName)
{
	// Open the pipe for writing
	int fd = open(pipeName, O_WRONLY);
	if (fd < 0)
	{
		perror("Error occured while opening named pipe for reading!\n");
		return FALSE;
	}

	// Write to pipe
	int n = write(fd, str, strlen(str));
	if (n < 0)
	{
		perror("Error writing to named pipe!\n");
		return FALSE;
	}

	// Finished writing. Close the pipe.
	close(fd);

	return TRUE;
}

/**
 * @brief Reads given pipe and writes input to given string
 *
 * @param str
 * @param pipeName
 * @return int
 */
int readFromPipe(char *str, char *pipeName)
{
	// Open the pipe for reading
	int fd = open(pipeName, O_RDONLY);
	if (fd < 0)
	{
		perror("Error occured while opening named pipe for reading!\n");
		return FALSE;
	}

	// Start reading the pipe
	char buffer[MAX_BUFFER_LENGTH];
	int n = read(fd, buffer, MAX_BUFFER_LENGTH);
	if (n < 0)
	{
		perror("Error occured while reading from named pipe!\n");
		return FALSE;
	}

	// If data in pipe changed
	if (strcmp(buffer, "connect") != 0)
	{
		// We have a unique pipe name!
		strcpy(pipeName, buffer);
	}

	// Finished writing. Close the named pipe.
	close(fd);

	return TRUE;
}
