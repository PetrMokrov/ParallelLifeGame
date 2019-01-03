# pragma once

# include <mpi.h>

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>

#include "life_game_master.hpp"
#include "life_game_slaver.hpp"

int MasterRoutine(int world_rank, int world_size) {

  LifeGameMPIMaster master(world_rank, world_size);
  master.StartCommandListen();

}

int SlaverRoutine(int world_rank, int world_size) {
  SlaverOverJob(world_rank, world_size);
}

void LifeGameMPI() {
  
  MPI_Init(NULL, NULL);

  // Get the number of the processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  if(world_size == 1) {
    std::cout << "One processed systems not supported yet\n";
    MPI_Finalize();
    return;
  }

  // Get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if(world_rank == 0) {
    // we makes the 0 process to be master process

    MasterRoutine(world_rank, world_size);

    // finalizing the job
    MPI_Finalize();
    return;

  }

  // the processes with world_rank > 0 become slavers

  SlaverRoutine(world_rank, world_size);

  // finalizing the job
  MPI_Finalize();
  return;
  
}