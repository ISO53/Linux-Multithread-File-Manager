#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <readline/readline.h>

#define TRUE 1
#define FALSE 0
#define MAIN_FIFO_NAME "MAIN_FIFO"
#define MAX_BUFFER_LENGTH 128
#define INPUT_STR "\nYour Input -> "
#define CONNECT "connect"

void* readUserInput();
void* readServiceInputs();
void handleUserInputs(char *);
void handleServiceInputs(char *);
int equals(char *, char *);
int startsWith(char *, char *);
int readFromPipe(char *, char *);
int writeToPipe(char *, char *);

char pipeName[MAX_BUFFER_LENGTH];
int userThreadControl;
int serviceThreadControl;

int main(int argc, char const *argv[])
{
    // Infinite loop control for threads
    userThreadControl = TRUE;
    serviceThreadControl = TRUE;

    // Main named pipe to connect service (manager)
    writeToPipe(CONNECT, MAIN_FIFO_NAME);

    // Wait for new unique pipe name from manager
    readFromPipe(pipeName, MAIN_FIFO_NAME);

    printf("We have a unique pipe name-> %s\n", pipeName);
    //writeToPipe("something", pipeName);

    pthread_t userInputThread, serviceInputThread;

    // Create two threads
    pthread_create(&userInputThread, NULL, readUserInput, NULL);
    pthread_create(&serviceInputThread, NULL, readServiceInputs, NULL);

    // Wait for the threads to finish
    pthread_join(userInputThread, NULL);
    pthread_join(serviceInputThread, NULL);

    printf("Program finished!\n");

    return 0;
}

/**
 * @brief Reads terminal concurrently. If user has entered any input, handles it.
 *
 * @return void*
 */
void* readUserInput()
{
    char *userInput;

    while (userThreadControl)
    {
        userInput = readline(INPUT_STR);

        if (userInput == NULL || strlen(userInput) <= 0)
        {
            continue;
        }

        // User entered an input! Input is in {userInput}.
        handleUserInputs(userInput);
    }
}

/**
 * @brief Reads terminal concurrently. If user has entered any input, handles it.
 *
 * @return void*
 */
void* readServiceInputs()
{
    char *serviceInput;

    while (serviceThreadControl)
    {
        printf("pipeName: %s\n", pipeName);
        int status = readFromPipe(serviceInput, pipeName);
        
        if (status < 0) {
            continue;
        }

        if (serviceInput == NULL || strlen(serviceInput) <= 0) {
            continue;
        }

        // Manager sent an input! Input is in {serviceInput}.
        handleServiceInputs(serviceInput);
    }
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
 * @brief Return true if str1 starts with str2
 *
 * @param str1
 * @param str2
 * @return int
 */
int startsWith(char *str1, char *str2)
{
    return strncmp(str1, str2, strlen(str2)) == 0;
}

/**
 * @brief Handles user input readed from stdin (terminal)
 * 
 * @param userInput 
 */
void handleUserInputs(char *userInput)
{

    // create filename
    if (startsWith(userInput, "create"))
    {
        writeToPipe("create something", pipeName);
        return;
    }
    // delete filename
    else if (startsWith(userInput, "delete"))
    {
    }
    // read filename
    else if (startsWith(userInput, "read"))
    {
    }
    // write filename string
    else if (startsWith(userInput, "write"))
    {
    }
    // exit
    else if (equals(userInput, "exit"))
    {
        writeToPipe("exit", pipeName);
        userThreadControl = FALSE;
        serviceThreadControl = FALSE;
        return;
    }
    else
    {
        printf("Wrong input! Try Again.\n");
    }
}

/**
 * @brief Handles service input readed from named pipe
 * 
 * @param serviceInput
 */
void handleServiceInputs(char *serviceInput)
{
    printf("Message came from manager: %s", serviceInput);
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
        printf("Error occured while opening named pipe [%s] for writing!\n", pipeName);
        perror("");
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

    // Finished writing. Close the named pipe.
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
        return fd;
    }

    // Start reading the pipe
    int n = read(fd, str, MAX_BUFFER_LENGTH - 1);
    if (n < 0)
    {
        //printf("errno: %d\n", errno);
        perror("Error occured while reading from named pipe!\n");
        return n;
    }

    // Finished writing. Close the named pipe.
    close(fd);

    return TRUE;
}
