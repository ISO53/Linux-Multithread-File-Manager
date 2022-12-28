#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <readline/readline.h>

#define TRUE 1
#define FALSE 0
#define MAIN_FIFO_NAME "MAIN_FIFO"
#define MAX_BUFFER_LENGTH 128
#define NUMBER_OF_TOKENS 2
#define INPUT_STR "Your Input -> "
#define CONNECT "connect"

void* readUserInput();
void* readServiceInput();
void handleUserInputs(char *userInput);
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
    int status = writeToPipe(CONNECT, MAIN_FIFO_NAME);
    if (status != TRUE)
    {
        exit(EXIT_FAILURE);
    }

    // Wait for new unique pipe name from manager
    readFromPipe(pipeName, MAIN_FIFO_NAME);

    printf("We have a unique pipe name-> %s\n", pipeName);    

    pthread_t userInputThread, serviceInputThread;

    // Create two threads
    pthread_create(&userInputThread, NULL, readUserInput, NULL);
    pthread_create(&serviceInputThread, NULL, readServiceInput, NULL);

    // Wait for the threads to finish
    pthread_join(userInputThread, NULL);
    pthread_join(serviceInputThread, NULL);

    printf("Program finished!\n");

    return 0;
}

void* readUserInput()
{
    char *userInput;

    while (userThreadControl)
    {
        userInput = readline(INPUT_STR);

        if (strlen(userInput) < 0)
        {
            // User didn't enter any input. Carry on to the endless loop.
            continue;
        }

        // User entered an input! Input is in {userInput}.
        handleUserInputs(userInput);
    }
}

void* readServiceInput() {}

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
        writeToPipe("exit\n", pipeName);
        userThreadControl = FALSE;
        return;
    }
    else
    {
        printf("Wrong input! Try Again.\n");
    }
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
