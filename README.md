# ParallelLifeGame

This project includes realization of multithreading life game
To see the rules, visit this wikipedia site [life game rules](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life)

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

