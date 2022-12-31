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
#define MAX_BUFFER_LENGTH 256
#define MAX_TOKEN_SIZE 10
#define MAIN_FIFO_NAME "MAIN_FIFO"

typedef struct
{
	int front;
	int rear;
	int size;
	char data[MAX_QUEUE_SIZE][MAX_BUFFER_LENGTH];
} Queue;
Queue *create_queue();
int isEmpty(Queue *);
int isFull(Queue *);
int enqueue(Queue *, char *);
int dequeue(Queue *, char *);

typedef struct
{
	int size;
	int capacity;
	char **items; // array of void pointers
} ArrayList;
ArrayList *create_list(int);
void destroy_list(ArrayList *);
int add_item(ArrayList *, char *);
int is_exists(ArrayList *, char *);
void remove_item(ArrayList *, char *);
void *get_item(ArrayList *, int);

void *readMainNamedPipe();
void *workerThread();
void *readerThread(void *);
void createWorkerThreads(int);
int tokenizeInput(char *, char **, int);
int equals(char *, char *);
int writeToPipe(char *, char *);
int readFromPipe(char *, char *);
int getLastDigitAsInt(char *);
void readLineFromFile(char *, int, char *);
void writeToFile(char *, char *);

pthread_t workerThreadList[MAX_THREAD_CAPACITY];
pthread_t readerThreadList[MAX_THREAD_CAPACITY];
int isWorking[MAX_THREAD_CAPACITY];
ArrayList *fileList;
Queue *workQueue;
int currentThreadIndex;
pthread_mutex_t mutex;

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
	for (int i = 0; i < MAX_THREAD_CAPACITY; i++)
	{
		isWorking[i] = FALSE;
	}

	pthread_mutex_init(&mutex, NULL);

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
	list->items = malloc(sizeof(char *) * list->capacity);

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

int add_item(ArrayList *list, char *item)
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

int is_exists(ArrayList *list, char *item)
{
	for (int i = 0; i < list->capacity; i++)
	{
		if (list->items[i] == NULL)
		{
			continue;
		}

		if (equals(list->items[i], item))
		{
			return TRUE;
		}
	}
	return FALSE;
}

void remove_item(ArrayList *list, char *item)
{
	for (int i = 0; i < list->capacity; i++)
	{
		if (list->items[i] == NULL)
		{
			continue;
		}

		if (equals(item, list->items[i]))
		{
			list->items[i] = NULL;
			return;
		}
	}
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

int dequeue(Queue *queue, char *str)
{
	if (isEmpty(queue))
	{
		return -1;
	}
	queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
	strcpy(str, queue->data[queue->front]);
	queue->size--;
	return 1;
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
		// Sleep for 100ms
		usleep(100000);

		// Get a work from queue if exists
		char work[MAX_BUFFER_LENGTH];

		pthread_mutex_lock(&mutex);
		int status = dequeue(workQueue, work);
		pthread_mutex_unlock(&mutex);

		if (status < 0)
		{
			continue;
		}

		printf("Work is: [%s]\n", work);

		// tokens[0] = pipeName, tokens[1] = command, tokens[2] = parameter...
		char *tokens[MAX_TOKEN_SIZE];
		tokenizeInput(work, tokens, MAX_TOKEN_SIZE);
		int pipeNumber = getLastDigitAsInt(tokens[0]);

		if (equals(tokens[1], "create"))
		{
			if (tokens[2] == NULL || strlen(tokens[2]) <= 0)
			{
				writeToPipe("File name missing!", tokens[0]);
			}
			else if (is_exists(fileList, tokens[2]))
			{
				writeToPipe("File already exist!", tokens[0]);
			}
			else
			{
				FILE *fp = fopen(tokens[2], "a+");
				add_item(fileList, tokens[2]);
				writeToPipe("File created.", tokens[0]);
				fclose(fp);
			}
		}
		else if (equals(tokens[1], "delete"))
		{
			if (tokens[2] == NULL || strlen(tokens[2]) <= 0)
			{
				writeToPipe("File name missing!", tokens[0]);
			}
			else if (!is_exists(fileList, tokens[2]))
			{
				writeToPipe("File doesn't exist!", tokens[0]);
			}

			unlink(tokens[2]);
			remove_item(fileList, tokens[2]);
			writeToPipe("File deleted.", tokens[0]);
		}
		else if (equals(tokens[1], "read"))
		{
			if (tokens[2] == NULL || strlen(tokens[2]) <= 0)
			{
				writeToPipe("File name missing!", tokens[0]);
			}
			else if (!is_exists(fileList, tokens[2]))
			{
				writeToPipe("File doesn't exist!", tokens[0]);
			}
			else if (tokens[3] == NULL || strlen(tokens[3]) <= 0)
			{
				writeToPipe("File index line missing!", tokens[0]);
			}
			char line[MAX_BUFFER_LENGTH];
			readLineFromFile(tokens[2], atoi(tokens[3]), line);
			writeToPipe(line, tokens[0]);
		}
		else if (equals(tokens[1], "write"))
		{
			if (tokens[2] == NULL || strlen(tokens[2]) <= 0)
			{
				writeToPipe("File name missing!", tokens[0]);
			}
			else if (!is_exists(fileList, tokens[2]))
			{
				writeToPipe("File doesn't exist!", tokens[0]);
			}
			else if (tokens[3] == NULL || strlen(tokens[3]) <= 0)
			{
				writeToPipe("File index line missing!", tokens[0]);
			}
			writeToFile(tokens[2], tokens[3]);
			writeToPipe("Successfully writen to file.", tokens[0]);
		}
		else if (equals(tokens[1], "exit"))
		{
			writeToPipe("exit.", tokens[0]);
		}
		else
		{
			writeToPipe("Entered wrong command!.", tokens[0]);
		}

		isWorking[pipeNumber - 1] = FALSE;
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
	char *tempPipeName = (char *)param;
	char pipeName[MAX_BUFFER_LENGTH];
	strcpy(pipeName, tempPipeName);
	printf("pipeName: %s\n", pipeName);
	free(param);
	int pipeNumber = getLastDigitAsInt(pipeName);

	while (TRUE)
	{
		if (isWorking[pipeNumber - 1] == TRUE)
		{
			// Client sent a task to manager and manager is still working on that task. Until
			// the compilation of that task, reader cannot read any other task from client.
			continue;
		}

		// Read data from the unique named pipe
		char buffer[MAX_BUFFER_LENGTH];
		readFromPipe(buffer, pipeName);

		if (buffer == NULL || strlen(buffer) <= 0)
		{
			continue;
		}

		// Add command (buffer) to command queue for worker threads with pipeName attached to it
		int size = strlen(pipeName) + strlen(buffer) + 2;
		char commandWithPipeName[size];
		strcpy(commandWithPipeName, pipeName);
		strcat(commandWithPipeName, " ");
		strcat(commandWithPipeName, buffer);
		enqueue(workQueue, commandWithPipeName);
		// commandWithPipeName -> "pipeName buffer"

		isWorking[pipeNumber - 1] = TRUE;
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
		char *pipeName = (char *)malloc(sizeof(char) * size + 1);
		sprintf(pipeName, "pipe%d", currentThreadIndex++);

		// Send that pipe name to client
		writeToPipe(pipeName, MAIN_FIFO_NAME);

		// Create a reader thread for that client
		pthread_create(&readerThreadList[currentThreadIndex - 2], NULL, readerThread, (void *)pipeName);
	}
}

/**
 * @brief Tokenizes the given input into an array of strings and returns the number
 * of tokenized elements. If the number doesnt match the length, than there is
 * less token than the given arrays size.
 *
 * @param input
 * @param tokens
 * @param maxLength
 * @return int
 */
int tokenizeInput(char *input, char **tokens, int maxLength)
{
	int counter = 0;
	char *temp = strtok(input, " ");

	while (temp != NULL && counter < maxLength)
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
		printf("Error occured while opening named pipe [%s] for reading!\n", pipeName);
		perror("");
		return FALSE;
	}

	// Write to pipe
	ftruncate(fd, 0);
	int n = write(fd, str, strlen(str) + 1);
	if (n < 0)
	{
		printf("Error occured while writing to named pipe [%s]!\n", pipeName);
		perror("");
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
		printf("Error occured while opening named pipe [%s] for reading!\n", pipeName);
		perror("");
		return FALSE;
	}

	// Start reading the pipe
	int n = read(fd, str, MAX_BUFFER_LENGTH);
	if (n < 0)
	{
		printf("Error occured while reading from named pipe [%s]!\n", pipeName);
		perror("");
		return FALSE;
	}

	// Finished writing. Close the named pipe.
	close(fd);

	return TRUE;
}

/**
 * @brief Returns last digit of a string in int type
 *
 * @param str
 * @return int
 */
int getLastDigitAsInt(char *str)
{
	int length = strlen(str);
	char last = str[length - 1];
	int num = (int)(last - '0');
	return num;
}

/**
 * @brief Reads a file from given name and copies the n-th line to given string.
 *
 * @param fileName
 * @param lineIndex
 * @param str
 * @return void
 */
void readLineFromFile(char *fileName, int lineIndex, char *str)
{
	FILE *fp;
	fp = fopen(fileName, "r");
	if (fp == NULL)
	{
		perror("An error occured while opening file for reading.\n");
		return;
	}

	int lineCount = 1;
	char buffer[MAX_BUFFER_LENGTH];
	while (fgets(buffer, MAX_BUFFER_LENGTH, fp) != NULL)
	{
		if (lineCount == lineIndex)
		{
			strcpy(str, buffer);
			return;
		}
		lineCount++;
	}

	fclose(fp);
}

/**
 * @brief Writes given string to file with a given name in append mode
 *
 * @param fileName
 * @param str
 * @return void
 */
void writeToFile(char *fileName, char *str)
{
	FILE *fp;
	fp = fopen(fileName, "a");
	if (fp == NULL)
	{
		perror("An error occured while opening file for writing!\n");
		return;
	}

	fprintf(fp, "%s\n", str);
	fclose(fp);
}
