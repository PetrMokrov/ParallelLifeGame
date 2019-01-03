#pragma once

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <mpi.h>
#include <cstring>

bool test_life(int x, int y, const std::vector<bool *> & game_field, int N, int M) {
    size_t lived_neighbour_count = 0;
    bool currently_lived = game_field[x][y];
    lived_neighbour_count += game_field[x + 1][y];
    lived_neighbour_count += game_field[x + 1][(y + 1) % M];
    lived_neighbour_count += game_field[x][(y + 1) % M];
    lived_neighbour_count += game_field[x - 1][(y + 1) % M];
    lived_neighbour_count += game_field[x - 1][y];
    lived_neighbour_count += game_field[x - 1][(M + y - 1) % M];
    lived_neighbour_count += game_field[x][(M + y - 1) % M];
    lived_neighbour_count += game_field[x + 1][(M + y - 1) % M];
    if(lived_neighbour_count == 3) return(true);
    if(currently_lived && lived_neighbour_count == 2) return(true);
    return(false);
  }


int left_nb(int a, int world_size) {
  if(a == 1) {
    return world_size - 1;
  }
  return a - 1;
}

int right_nb(int a, int world_size) {
  if(a == world_size - 1) {
    return 1;
  } 
  return a + 1;
}


void SlaverJob(int world_rank, int world_size) {
  // this is the length of the filed to process
  size_t M = 0;
  MPI_Recv(&M, 1, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  // this is the height of the field to process
  int N = 0;
  MPI_Recv(&N, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  std::vector<bool *> field(N + 2, nullptr);

  for(int i = 1 ; i < N + 1; ++i ) {
    bool * new_buff = new bool[M];
    MPI_Recv(new_buff, int(M), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    field[i] = new_buff;
  }

  // the first and the last fields of field will be used for in communication

  bool * new_buff = new bool[M];
  field[0] = new_buff;
  new_buff = new bool[M];
  field[N + 1] = new_buff;

  // this variable will be used during execution
  // it will denote the iteration appropriate to the field[0]
  // our left neighbour in charge of this field
  int left_border_status = -1;

  // this variable will be used during execution
  // it will denote the iteration appropriate to the main field
  // we are responsible for this field
  int field_status = 0;

  // -//-
  int right_border_status = -1;

  /*
  std::cout << "Thread number " << world_rank << " has got " << M << " * " << N << " bytes\n";
  std::cout << "It takes the following disposition:\n";
  for(auto s : field) {
    for(int i = 0 ; i < M ; ++i) {
      std::cout << s[i];
    }
    std::cout << "\n";
  }
  */

  // the real job starts here:

  // wait for RUN command:
  int it_count = 0;
  MPI_Bcast(&it_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // run ahead!

  //std::cout << "the process number " << world_rank << " is runned " << "with " << it_count << "its \n";

  //MPI_Barrier(MPI_COMM_WORLD);


  // request for local operations
  MPI_Request local_request = MPI_REQUEST_NULL;

  // request for master messages operations
  MPI_Request master_request = MPI_REQUEST_NULL;

  // launch listening for master (Because we obey master :) )

  int master_message = 0;
  bool stop_flag = false;
  MPI_Ibcast(&master_message, 1, MPI_INT, 0, MPI_COMM_WORLD, &master_request);

  while(true) {
    // check for master commands:
    int receive_flag = 0;
    MPI_Test(&master_request, &receive_flag, MPI_STATUS_IGNORE);
    if(receive_flag) {
      // we have got the message from our master

      MPI_Request send_request = MPI_REQUEST_NULL;

      if(master_message == 1) {
        // Status request

        int stat = field_status;
        // current status of the runner
        MPI_Isend(&stat, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &send_request);

        for(int i = 1 ; i < N + 1 ; ++ i) {
          // send rows of the field to the master
          MPI_Isend(field[i], M, MPI_BYTE, 0, i - 1, MPI_COMM_WORLD, &send_request);
        }
      }

      if(master_message == 2) {
        // Stop request
        int responce = field_status;

        // send current iteration
        MPI_Isend(&responce, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &send_request);
        stop_flag = true;
      }

      if(master_message == 3) {
        // this command returns the current status of execution
        int responce = field_status;
        MPI_Isend(&responce, 1, MPI_INT, 0, 3, MPI_COMM_WORLD, &send_request);
      }

      // wait for new command
      if(!stop_flag) {
        MPI_Ibcast(&master_message, 1, MPI_INT, 0, MPI_COMM_WORLD, &master_request);
      }
    }
    // nonblocking send the first our row to top worker
    MPI_Isend(
      field[1], 
      M, 
      MPI_BYTE, 
      left_nb(world_rank, world_size),
      7, 
      MPI_COMM_WORLD, &local_request);
    
    // nonblocking send the last our row to the bottom worker
    MPI_Isend(
      field[N], 
      M, 
      MPI_BYTE, 
      right_nb(world_rank, world_size),
      8, 
      MPI_COMM_WORLD, &local_request);
/*
    if(stop_flag) {
      stop_flag = false;
      MPI_Bcast(&it_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

      std::cout << world_rank << " " << it_count << " " << field_status <<  "\n";

      MPI_Ibcast(&master_message, 1, MPI_INT, 0, MPI_COMM_WORLD, &master_request);
    }
    */
    // blocking receiving the field[0] *we can not execute our job without it*

    if(stop_flag) {
      stop_flag = false;
      MPI_Bcast(&it_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

      // std::cout << world_rank << " " << it_count << " " << field_status <<  "\n";

      MPI_Ibcast(&master_message, 1, MPI_INT, 0, MPI_COMM_WORLD, &master_request);

      if(world_rank <= field_status) {
        break;
      }
    }

    MPI_Recv(
      field[0],
      M,
      MPI_BYTE,
      left_nb(world_rank, world_size),
      8,
      MPI_COMM_WORLD,
      MPI_STATUS_IGNORE);

    // blocking receiving the field[N + 1]
    MPI_Recv(
      field[N + 1],
      M,
      MPI_BYTE,
      right_nb(world_rank, world_size),
      7,
      MPI_COMM_WORLD,
      MPI_STATUS_IGNORE);

    // now we can work. Let's start!


    bool buff[M];
    bool buff_2[M];
    for(int i = 1 ; i < N + 1; ++i) {
      std::memcpy(buff, buff_2, M * sizeof(bool));
      for(int j = 0 ; j < M ; ++j) {
        buff_2[j] = test_life(i, j, field, N, M);
      }
      if(i > 1) {
        std::memcpy(field[i - 1], buff, M*sizeof(bool));
      }
    }
    std::memcpy(field[N], buff_2, M*sizeof(bool));
   
    ++field_status;
    if(field_status >= it_count) {
      break;
    }
  }
  /*
  std::cout << "Final disposition:\n";
  for(auto s : field) {
    for(int i = 0 ; i < M ; ++i) {
      std::cout << s[i];
    }
    std::cout << "\n";
  }
  */

  // wait for any request from master
  while(true) {

    // check for master commands:
    int receive_flag = 0;
    MPI_Test(&master_request, &receive_flag, MPI_STATUS_IGNORE);
    if(receive_flag) {
      // we have got the message from our master

      MPI_Request send_request = MPI_REQUEST_NULL;

      if(master_message == 1) {
        // Status request
        int stat = -1;
        // current status of the runner
        MPI_Isend(&stat, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &send_request);

        for(int i = 1 ; i < N + 1 ; ++ i) {
          // send rows of the field to master
          MPI_Isend(field[i], M, MPI_BYTE, 0, i - 1, MPI_COMM_WORLD, &send_request);
        }
      }

      if(master_message == 2) {
        // Stop request
        int responce = field_status;

        // send current iteration
        MPI_Isend(&responce, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &send_request);

        MPI_Bcast(&it_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
      }

      if(master_message == 3) {
        int responce = -1; // this flag means finish of execution 
        MPI_Isend(&responce, 1, MPI_INT, 0, 3, MPI_COMM_WORLD, &send_request);
      }

      if(master_message == 4) {
        break;
      }

      MPI_Ibcast(&master_message, 1, MPI_INT, 0, MPI_COMM_WORLD, &master_request);
    }
  }

  // std::cout << "process " << world_rank << " is finishing!\n";
  // clear buffers:
  for(auto s : field) {
    delete [] s;
  }
}


void SlaverOverJob(int world_rank, int world_size) {
  while(true) {
    int start_cmd = 0;
    MPI_Bcast(&start_cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(start_cmd == 1) {
      SlaverJob(world_rank, world_size);
    } else {
      return;
    }
  }
}