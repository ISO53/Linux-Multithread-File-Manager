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
	char *data[MAX_QUEUE_SIZE];
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
void *readerThread(char *);
void createWorkerThreads(int);
int tokenizeInput(char *, char **, int);
int equals(char *, char *);
int writeToPipe(char *, char *);
int readFromPipe(char *, char *);

ArrayList *workerThreadList;
ArrayList *readerThreadList;
ArrayList *fileList;
Queue *workQueue;
int currentThreadIndex;

int main(int argc, char const *argv[])
{
	workerThreadList = create_list(MAX_THREAD_CAPACITY);
	readerThreadList = create_list(MAX_THREAD_CAPACITY);
	fileList = create_list(MAX_FILE_CAPACITY);
	workQueue = create_queue();
	currentThreadIndex = 1;

	// Create worker threads for clients
	createWorkerThreads(MAX_THREAD_CAPACITY);

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

	// ARADAKİ İŞLEMŞLER
	// adsa
	// asdasd

	pthread_join(mainThread, NULL);
	for (int i = 0; i < MAX_THREAD_CAPACITY; i++)
	{
		pthread_t tempWorkerThread = *(pthread_t *)(get_item(workerThreadList, i));
		pthread_join(tempWorkerThread, NULL);

		pthread_t tempReaderThread = *(pthread_t *)(get_item(readerThreadList, i));
		pthread_join(tempReaderThread, NULL);
	}

	destroy_list(workerThreadList);
	destroy_list(readerThreadList);
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
	queue->front = queue->rear = NULL;
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
	queue->data[queue->rear] = str;
	queue->size++;
}

char *dequeue(Queue *queue)
{
	if (isEmpty(queue))
	{
		return -1;
	}
	queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
	char *tempData = queue->data[queue->front];
	queue->size--;
	return tempData;
}
//************************************* Queue Functions Ends *************************************

void createWorkerThreads(int maxThreadCapacity)
{
	for (int i = 0; i < maxThreadCapacity; i++)
	{
		// Create unique name for every thread
		int size = log10(i + 1) + 1;
		char pipeName[size];
		sprintf(pipeName, "pipe_%d", (i + 1));

		// Create a unique pipe with that name
		int result = mkfifo(pipeName, 0666);
		if (result < 0)
		{
			perror("Error occured while creating named pipe! Trying to delete file...\n");
			unlink(pipeName);

			// Now let's try again
			result = mkfifo(pipeName, 0666);
			if (result < 0)
			{
				perror("Something wrong!\n");
				exit(EXIT_FAILURE);
			}
			printf("Error prevented.\n");
		}

		// Create a new worker
		pthread_t var_workerThread;
		pthread_create(&var_workerThread, NULL, workerThread, NULL);
		add_item(workerThreadList, &var_workerThread);

		// Create a new reader thread
		pthread_t var_readerThread;
		pthread_create(&var_readerThread, NULL, readerThread(pipeName), NULL);
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
	}
}

/**
 * @brief Reads data from given pipe name concurrently. If there is a data in the pipe, concantanates
 * pipe name into that data and sends that to queue where worker threads can read and execute.
 *
 * @param pipeName
 * @return void*
 */
void *readerThread(char *pipeName)
{
	while (TRUE)
	{
		// Read data from the unique named pipe
		char buffer[MAX_BUFFER_LENGTH];
		readFromPipe(buffer, pipeName);

		printf("Data read from the pipe named [%s] is [%s].\n", pipeName, buffer);

		// Add command (buffer) to command queue for worker threads with pipeName attached to command
		strcat(pipeName, "-");
		strcat(pipeName, buffer);
		enqueue(workQueue, pipeName);
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

		// A client wrote 'connect' to the pipe.
		printf("Client wrote: %s\n", buffer);

		// Check if there is available thread
		if (currentThreadIndex > MAX_THREAD_CAPACITY)
		{
			// There are no available thread
			continue;
		}

		// Available thread pipe name
		char pipeName[6];
		sprintf(pipeName, "pipe_%d", currentThreadIndex++);

		// Send that pipe name to client
		writeToPipe(pipeName, MAIN_FIFO_NAME);
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
