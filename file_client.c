#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <readline/readline.h>

#define TRUE 1
#define FALSE 0
#define MAIN_FIFO_NAME "MAIN_FIFO"
#define MAX_BUFFER_LENGTH 100
#define NUMBER_OF_TOKENS 2
#define INPUT_STR "Your Input -> "

void readUserInput();
void readServiceMessage();
int equals(char *, char *);
int startsWith(char *, char *);

char *pipeName;

int main(int argc, char const *argv[])
{
    // Main named pipe to connect service (manager)
    int result = mkfifo(MAIN_FIFO_NAME, 0666);
    if (result < 0)
    {
        perror("Error occured while creating named pipe!\n");
        exit(EXIT_FAILURE);
    }

    char *connect = "connect";
    int status = writeToPipe(connect, MAIN_FIFO_NAME);
    if (status != TRUE) {
        exit(EXIT_FAILURE);
    }

    

    // Start reading the main pipe for upcoming new pipe name
    char buffer[MAX_BUFFER_LENGTH];
    int n = read(fd, buffer, MAX_BUFFER_LENGTH);
    if (n < 0)
    {
        perror("Error occured while reading from named pipe!\n");
        return EXIT_FAILURE;
    }

    // If data in pipe changed
    if (strcmp(buffer, "connect") != 0)
    {
        // We have a unique pipe name!
        strcpy(pipeName, buffer);
    }

    // Unique pipe to connect service (manager)
    int result = mkfifo(pipeName, 0666);
    if (result < 0)
    {
        perror("Error occured while creating named pipe!\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void readUserInput()
{
    char userInput[MAX_BUFFER_LENGTH];

    while (TRUE)
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

void readServiceMessage() {}

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

void handleUserInputs(char *userInput)
{

    if (startsWith(userInput, "create"))
    {
    }
    else if (startsWith(userInput, "delete"))
    {
    }
    else if (startsWith(userInput, "read"))
    {
    }
    else if (startsWith(userInput, "write"))
    {
    }
    else if (startsWith(userInput, "exit"))
    {
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
int writeToPipe(char *str, char *pipeName)
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

