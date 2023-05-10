# Project Description

The project aims to increase proficiency in named pipe and multi-thread programming. Within the scope of the project, the use of named pipe, multi-thread, and synchronization mechanisms is required. Additionally, simple C language structures should not be used.

The project requires the implementation of two programs: **file_manager** and **file_client**.

File_manager is the main program and works like a service. It contains a thread and an array of file lists. It communicates with file_client through a named pipe called "file_manager_named_pipe". The connected client waits in the thread until the "exit" command is received.

# Project Functions

+ Create: If the file name does not exist in the File_List, it adds the file name to the empty sequence and creates the file in the system.
+ Delete: If the file name exists in the File_List, it deletes the file from the system and the File_List (index).
+ Read: If the file name exists in the File_List, it reads the line from the file with the corresponding index and sends it to the relevant File_Client.
+ Write: If the file name exists in the File_List, it writes the data received from the File_client to the file.
+ Exit: Ends the relevant File_Client thread and terminates the communication. The thread continues to listen until the "exit" command is received.
After each operation, a response about the operation status should be returned to the File_Client.

# Important Parts

+ Lock mechanisms are used when reading or writing to the File_List with multiple threads.
+ Only one thread performs an operation when writing to or reading from a file. A lock mechanism also added here.
+ The program implemented in the C language and include the necessary structures, functions, threads, and lock mechanisms.
+ This program uses some Linux-only libraries.

# How to Run the Program
To run the file_manager program, open a terminal and navigate to the project directory. The program consists of two subprograms. Manager and client. First of all you should compile the the program by typing `make` to the terminal. Now you can call the Manager program by typing `./manager` to the terminal. When you start the manager program it will listen incoming clients. You can call up to 5 client program. To do that open a different terminal and navigate to the project directory. And then simply run `./client`. Now on client these are the commands that you can use.
```
create [file_name]
```
This command will create a file so you can write inside it.

```
delete [file_name]
```
This command will delete the created file.

```
read [file_name] [line_number]
```
This command prints out the specified line in the specified file.

```
write [file_name] [some_string]
```
This command will append specified string to the file.

```
exit
```
This command shuts down the client program. You can also use this command in manager to shut down manager program.
