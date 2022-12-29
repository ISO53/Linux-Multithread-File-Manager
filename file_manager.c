#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0
#define MAX_THREAD_CAPACITY 5
#define MAX_FILE_CAPACITY 10
#define MAX_QUEUE_SIZE 256
#define MAX_BUFFER_LENGTH 128
#define MAIN_FIFO_NAME "MAIN_FIFO"

typedef struct
{
	int front;
	int rear;
	int size;
	char data[MAX_QUEUE_SIZE][MAX_BUFFER_LENGTH];
} Queue;
Queue *create_queue();
int isEmpty(Queue *queue);
int isFull(Queue *queue);
int enqueue(Queue *queue, char *str);
char *dequeue(Queue *queue);

typedef struct
{
	int size;
	int capacity;
	void **items; // array of void pointers
} ArrayList;
ArrayList *create_list(int);
void destroy_list(ArrayList *);
int add_item(ArrayList *, void *);
void remove_item(ArrayList *, int);
void *get_item(ArrayList *, int);

void *readMainNamedPipe();
void *workerThread();
void *readerThread(void *);
void createWorkerThreads(int);
int tokenizeInput(char *, char **, int);
int equals(char *, char *);
int writeToPipe(char *, char *);
int readFromPipe(char *, char *);

pthread_t workerThreadList[MAX_THREAD_CAPACITY];
pthread_t readerThreadList[MAX_THREAD_CAPACITY];
ArrayList *fileList;
Queue *workQueue;
int currentThreadIndex;

/**
 * @brief Main function
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char const *argv[])
{
	fileList = create_list(MAX_FILE_CAPACITY);
	workQueue = create_queue();
	currentThreadIndex = 1;

	// Delete the main named pipe if it has been created previosuly
	unlink(MAIN_FIFO_NAME);

	// Main named pipe for newly created clients to connect
	int result = mkfifo(MAIN_FIFO_NAME, 0666);
	if (result < 0)
	{
		perror("Error occured while creating named pipe!\n");
		exit(EXIT_FAILURE);
	}

	// Listen main pipe in another thread concurrently
	pthread_t mainThread;
	pthread_create(&mainThread, NULL, readMainNamedPipe, NULL);

	// Create worker and reader threads for clients
	createWorkerThreads(MAX_THREAD_CAPACITY);

	// ARADAKİ İŞLEMŞLER
	// adsa
	// asdasd

	for (int i = 0; i < MAX_THREAD_CAPACITY; i++)
	{
		pthread_join(workerThreadList[i], NULL);
		pthread_join(readerThreadList[i], NULL);
	}
	pthread_join(mainThread, NULL);

	destroy_list(fileList);

	unlink(MAIN_FIFO_NAME);

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

//**************************************** Queue Functions ***************************************
Queue *create_queue()
{
	Queue *queue = (Queue *)malloc(sizeof(Queue));
	queue->front = queue->rear = -1;
	queue->size = 0;
	return queue;
}

int isEmpty(Queue *queue)
{
	return (queue->size == 0);
}

int isFull(Queue *queue)
{
	return (queue->size == MAX_QUEUE_SIZE);
}

int enqueue(Queue *queue, char *str)
{
	if (isFull(queue))
	{
		return -1;
	}
	queue->rear = (queue->rear + 1) % MAX_QUEUE_SIZE;
	// queue->data[queue->rear] = str;
	strcpy(queue->data[queue->rear], str);
	queue->size++;
}

char *dequeue(Queue *queue)
{
	if (isEmpty(queue))
	{
		return NULL;
	}
	queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
	char *tempData = queue->data[queue->front];
	queue->size--;
	return tempData;
}
//************************************* Queue Functions Ends *************************************

/**
 * @brief Creates reader and worker threads that reads client pipes concurrently. Every input that
 * has sent by client added to the queue by reader threads. Worker threads reads queue concurrently.
 *
 * @param maxThreadcapacity
 * @return void*
 */
void createWorkerThreads(int maxThreadCapacity)
{
	for (int i = 0; i < maxThreadCapacity; i++)
	{
		// Create unique name for every thread
		int size = log10(__INT_MAX__) + 5 + 1;
		char pipeName[size];
		sprintf(pipeName, "pipe%d", (i + 1));

		// Create a unique pipe with that name
		int result = mkfifo(pipeName, 0666);
		if (result < 0)
		{
			// There maybe a pipe already. Delete it and create it again.
			unlink(pipeName);

			// Now let's try again
			result = mkfifo(pipeName, 0666);
			if (result < 0)
			{
				perror("Something wrong!\n");
				exit(EXIT_FAILURE);
			}
		}

		// Create a new worker and reader thread
		pthread_create(&workerThreadList[i], NULL, workerThread, NULL);
	}
}

/**
 * @brief Reads data from queue concurrently. If the queue is not empty, takes the dequeue, analyzes
 * the command and executes it.
 *
 * @return void*
 */
void *workerThread()
{
	while (TRUE)
	{
		// burada mutex lock kullanmak lazım aga

		// Get a work from queue if exists
		char *work = dequeue(workQueue);

		if (work == NULL)
		{
			// Sleep for 100ms then rewind
			usleep(100000);
			continue;
		}

		printf("Work is: %s\n", work);
	}
}

/**
 * @brief Reads data from given pipe name concurrently. If there is a data in the pipe, concantanates
 * pipe name into that data and sends that to queue where worker threads can read and execute.
 *
 * @param pipeName
 * @return void*
 */
void *readerThread(void *param)
{
	char *pipeName = (char *)param;
	printf("pipeName: %s\n", pipeName);
	exit(1);

	while (TRUE)
	{
		// Read data from the unique named pipe
		char buffer[MAX_BUFFER_LENGTH];
		readFromPipe(buffer, pipeName);

		if (buffer == NULL || strlen(buffer) <= 0)
		{
			continue;
		}

		printf("Data read from the pipe named [%s] is [%s].\n", pipeName, buffer);

		// Add command (buffer) to command queue for worker threads with pipeName attached to it
		int size = strlen(pipeName) + strlen(buffer) + 2;
		char commandWithPipeName[size];
		strcpy(commandWithPipeName, pipeName);
		strcat(commandWithPipeName, "_");
		strcat(commandWithPipeName, buffer);
		enqueue(workQueue, commandWithPipeName);
	}
}

/**
 * @brief Reads the main pipe concurrently. If any client tries to connect check if there are
 * available thread. If there are return the available pipe's name to client.
 *
 * @return void*
 */
void *readMainNamedPipe()
{
	while (TRUE)
	{
		// Read data from the main named pipe
		char buffer[MAX_BUFFER_LENGTH];
		readFromPipe(buffer, MAIN_FIFO_NAME);

		if (buffer == NULL || strlen(buffer) <= 0)
		{
			continue;
		}

		// A client wrote 'connect' to the pipe.
		printf("Client wrote: %s\n", buffer);

		// Check if there is available thread
		if (currentThreadIndex > MAX_THREAD_CAPACITY)
		{
			// There are no available thread
			printf("There are no available threads.\n");
			continue;
		}

		// Available thread pipe name
		int size = log10(__INT_MAX__) + 4 + 1;
		char pipeName[size];
		sprintf(pipeName, "pipe%d", currentThreadIndex++);

		// Send that pipe name to client
		writeToPipe(pipeName, MAIN_FIFO_NAME);

		// Create a reader thread for that client
		pthread_create(&readerThreadList[currentThreadIndex - 2], NULL, readerThread, &pipeName);
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
	ftruncate(fd, 0);
	int n = write(fd, str, strlen(str) + 1);
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
	int n = read(fd, str, MAX_BUFFER_LENGTH);
	if (n < 0)
	{
		perror("Error occured while reading from named pipe!\n");
		return FALSE;
	}

	// Finished writing. Close the named pipe.
	close(fd);

	return TRUE;
}
