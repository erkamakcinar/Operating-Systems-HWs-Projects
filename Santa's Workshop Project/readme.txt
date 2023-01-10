This assignment was prepared in macOS environment. gcc is used as compiler.
The code creates 3 worker threads for Elf A, Elf B and Santa. 2 other threads as Printer and Generate Tasks.
Each worker thread runs in a loop, checking its task queue for tasks to complete.
If a task is available, the worker removes it from the queue and performs the necessary work (e.g., sleep for the appropriate amount of time to simulate the task).
After completing the task, the worker adds any necessary follow-up tasks to the appropriate queues.
The main function initializes the worker threads and runs the simulation for a given amount of time (in this case, 120 seconds).
To prevent deadlocks, the code uses mutexes to ensure that each worker thread only accesses the task queues in a mutually exclusive manner.
This ensures that only one worker thread can access a task queue at a time, which helps to prevent situations where two worker threads try to access the same queue simultaneously, leading to a deadlock.

NOTE:
Part1 of this assignment works without error, but part 2 and part 3 are not implemented.

NOTE:
The "$" symbol is used to indicate that it is a terminal command. The command is the code snippet after the "$" symbol.
Terminal commands required to compile:
1. $gcc -o c.o santasWorkshop.c
Required terminal commands to run:
1. Run the terminal command given below from a terminal.
$./c.o 6
The first parameter is the compiled output file. The second parameter is the n we give as the command line argument.
If the 2nd parameter is not given, the code still works but does not print.

