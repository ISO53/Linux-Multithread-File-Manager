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
#define INPUT_STR "Your Input -> "

void readUserInput();
void readServiceMessage();
int equals(char *str1, char *str2);

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

    // Open the main named pipe for writing
    int fd = open(MAIN_FIFO_NAME, O_WRONLY);
    if (fd < 0)
    {
        perror("Error occured while opening named pipe for reading!\n");
        exit(1);
    }

    // Write connect to the named pipe
    char *connect = "connect";
    int n = write(fd, connect, strlen(connect));
    if (n < 0)
    {
        perror("Error writing to named pipe!\n");
        exit(1);
    }

    // Finished writing. Close the named pipe.
    close(fd);

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

int equals(char *str1, char *str2)
{
    return strcmp(str1, str2) == 0;
}

void handleUserInputs(char *userInput)
{

    if (equals(userInput, "create"))
    {
    }
    else if (equals(userInput, "delete"))
    {
    }
    else if (equals(userInput, "read"))
    {
    }
    else if (equals(userInput, "write"))
    {
    }
    else if (equals(userInput, "exit"))
    {
    }
    else
    {
        printf("Wrong input! Try Again.\n");
    }
}