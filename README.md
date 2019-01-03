# ParallelLifeGame

This project includes realizations of life game using different techniques
To see the rules, visit this wikipedia site [life game rules](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life)

## Multithreading ParallelLifeGame

This program includes realization of multithreading life game
The code presented in `code` directory

The program provides following interface:

* `START` command. This command can be passed with following list of optional arguments:
   
  * **-t** **--threads** \[THREADS COUNT\] specifies the count of threads to launch

  * **-f** **--file** \[file.csv\] specifies the csv file with initial disposition

  * **-s** **--size** \[N\] \[M\] specifies the shapes of the game filed. The initial disposition will be created randomly in this case

    The command takes either `-f` or `-s` argument as the second one. If only `-t` argument is passed, the field of 10x10 will be created with random initial disposition
    If no arguments passed, the count of launched threads will be equal to `1`
  
* `STATUS` command shows current state of the field and displays the number of current iteration. Takes no arguments

* `RUN` command launches the threads which process life game states. It takes the argument:

  * [N], which denotes count of life game iterations to perform

* `STOP` command stops the execution and shows the current state

* `QUIT` command stops the threads and finish the program

To build the program see `readme.md` file in `code\build\` directory.

## MPI ParallelLifeGame

This program includes realisation of life game using MPI interface. To see more information about this technology visit [wikipedia](https://en.wikipedia.org/wiki/Message_Passing_Interface) or [official cite](https://www.open-mpi.org/doc/current/). Detailed documentation can be founded [here](https://www.mpi-forum.org/docs/).

The program provides similar interface as the multithreading one. One difference is that `START` command does not support **-t** and **--threads** command.

To build the program see `readme.md` file in `code_MPI\build\` directory.
