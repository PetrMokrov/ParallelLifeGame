To build the project select following in your terminal:
```bash
cmake ..
make
```

The program requires MPI on your computer
To install one visit the [offcial cite](https://www.open-mpi.org/).

To run the project select following:
```bash
mpirun -np [count of kernels to run] ./bin/MpiLifeGame
```