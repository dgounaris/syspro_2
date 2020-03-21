Implementation of a file sharing system for the 2nd system programming assignment.

In order to compile the program run "make" inside the directory. Run by running the command "./main" in the same directory.
You can pass the following parameters to the program:
-n [int]: The id of the client
-i [char*]: The input directory name
-m [char*]: The mirror directory name
-c [char*]: The common directory name
-b [int]: The buffer size when sendind through the pipe
-l [char*]: The logfile name to pipe the logged output to

4 types of log levels are introduced:
[INFO]: Low severity verbose log level that prints most of the system operations descriptively.
[PARENT MONITOR]: Low severity log level (similarly to info) that contains messages regarding the parent process monitoring on children.
[LOG]: Low severity log level that is used to create statistics. Do not create additional messages of this level unless you know what you're doing.
[ERROR]: High severity log level that is normally printed to stderr and contains error messages.

The implementation consists of 5 components:
- The childstruct file, which contains the structure used to monitor the children of the parent client process.
- The pollstruct file, which contains the structure used to monitor the polled clients.
- The utils file, which contains utility methods suh as deleting a directory.
- The main file, which contains the client's validation, initialization, polling and child process management.
- The clients file, which contains the code for the reading and the writing client.

The data structures used are the following:
- 2 lists (named as hash tables but since the hash value is always 0 this is a list effectively) to save the polled clients and the children processes.
The structures are implemented as lists because of the potential alteration of the pid, despite the small complexity overhead this creates.
