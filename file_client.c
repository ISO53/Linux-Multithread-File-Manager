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
#define MAX_BUFFER_LENGTH 256
#define INPUT_STR "\nYour Input -> "
#define CONNECT "connect"

void readUserInput();
void *waitingForResponse();
void handleServiceInputs(char *);
int readFromPipe(char *, char *);
int writeToPipe(char *, char *);

char pipeName[MAX_BUFFER_LENGTH];
int userThreadControl;
int serviceThreadControl;
int waiting;

int main(int argc, char const *argv[])
{
    // Infinite loop control for threads
    userThreadControl = TRUE;
    waiting = FALSE;

    // Main named pipe to connect service (manager)
    writeToPipe(CONNECT, MAIN_FIFO_NAME);

    // Wait for new unique pipe name from manager
    readFromPipe(pipeName, MAIN_FIFO_NAME);
    printf("We have a unique pipe name-> %s\n", pipeName);

    // Read user input until user want to exit
    readUserInput();

    printf("\nProgram finished!\n");

    return 0;
}

/**
 * @brief Reads terminal concurrently. If user has entered any input, handles it.
 *
 * @return void*
 */
void readUserInput()
{
    char *userInput;

    while (userThreadControl)
    {
        userInput = readline(INPUT_STR);

        if (userInput == NULL || strlen(userInput) <= 0)
        {
            continue;
        }

        // User entered an input! Input is in {userInput}. Send input to manager.
        writeToPipe(userInput, pipeName);

        pthread_t waitingThread;
        waiting = TRUE;
        pthread_create(&waitingThread, NULL, waitingForResponse, NULL);

        // Expect a message from manager
        char serviceInput[MAX_BUFFER_LENGTH];
        int status = readFromPipe(serviceInput, pipeName);
        waiting = FALSE;
        pthread_join(waitingThread, NULL);

        if (status < 0)
        {
            continue;
        }

        if (serviceInput == NULL || strlen(serviceInput) <= 0)
        {
            continue;
        }

        // Manager sent an input! Input is in {sendInput}.
        char *sendInput = (char *)malloc(sizeof(char) * MAX_BUFFER_LENGTH + 1);
        strcpy(sendInput, serviceInput);
        handleServiceInputs(sendInput);
    }
}

void *waitingForResponse()
{
    while (waiting)
    {
        printf("\rWaiting for response.  ");
        usleep(200000);
        fflush(stdout);
        printf("\rWaiting for response.. ");
        usleep(200000);
        fflush(stdout);
        printf("\rWaiting for response...");
        usleep(200000);
        fflush(stdout);
    }
    printf("\n");
}

/**
 * @brief Handles service input readed from named pipe
 *
 * @param serviceInput
 */
void handleServiceInputs(char *serviceInput)
{
    char input[MAX_BUFFER_LENGTH];
    strcpy(input, serviceInput);
    free(serviceInput);
    printf("\nMessage came from manager: %s\n", input);
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
