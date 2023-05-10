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
To run the file_manager program, open a terminal and navigate to the project directory. Then, run the following command:
