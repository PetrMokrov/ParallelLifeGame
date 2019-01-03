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
#include <assert.h>
#include <algorithm>

std::string spaces = "\t \n";

// this function extracts the next word from the string
// a word is a set of symbols surrounded by spaces 
// the string is changing
std::string GetWord(std::string & str) {

  // deletion spaces from the start of the initial string
  while(!str.empty() && !(spaces.find(str[0]) == std::string::npos)) {
    str.erase(0, 1);
  }

  // iteration for the word
  int i = 0;
  while(i + 1 <= str.size() && spaces.find(str[i]) == std::string::npos) {
    ++i;
  }

  // extraction the word
  std::string word = "";
  if(i > 0) {
    word = str.substr(0, i);
  }
  str.erase(0, i);
  return(word);
}


class LifeGameMPIMaster {
  public:

    explicit LifeGameMPIMaster(int world_rank, int world_size) {
      _world_size = world_size;
      _world_rank = world_rank;
    }

    ~LifeGameMPIMaster() {
      if(game_field != nullptr) {
        for(int i = 0 ; i < N  ; ++i) {
          delete [] game_field[i];
        }
        delete [] game_field;
      }
    }

    void StartCommandListen(void) {
      while(true) {
        std::string command;
        std::getline(std::cin, command);
        int cmd = _command_interpretation(command);
        if(cmd == -1) {
          continue;
        }
        if(cmd == 0) {
          _start_command_routine(command);
          continue;
        }
        if(cmd == 2) {
          _run_command_routine(command);
          continue;
        }
        if(cmd == 1) {
          _status_command_routine();
          continue;
        }
        if(cmd == 3) {
          _stop_command_routine();
          continue;
        }
        if(cmd == 4) {
          _quit_command_routine();
          break;
        }
      }
    }

  private:

    ////////////////////////////////////////////////////////////////////
    // this private block includes realization of command interpreter //
    ////////////////////////////////////////////////////////////////////

    int _command_interpretation(const std::string& command) {
      std::string::size_type pos;
      pos = command.find("START");
      if(pos != std::string::npos) {
        if(pos == 0) {
          // perform command Start
          if(session_started_flag) {
            std::cout << "Session has started already\n";
            return(-1);
          }
          return(0);
        }
      }
      pos = command.find("STATUS");
      if(pos != std::string::npos) {
        if(pos == 0) {
          // perform command Status
          if(!session_started_flag) {
            std::cout << "Session hasn't been started yet\n";
            return(-1);
          }
          /*
          if(!session_executed_flag) {
            std::cout << "No active session\n";
            return(-1);
          }
          */
          return(1);
        }
      }
      pos = command.find("RUN");
      if(pos != std::string::npos) {
        if(pos == 0) {
          // perform command run
          if(!session_started_flag) {
            std::cout << "Session hasn't been started yet\n";
            return(-1);
          }
          if(session_executed_flag) {
            if(_runners_finished()) {
              session_executed_flag = false;
              return(2);
            }
            std::cout << "Session has been already run\n";
            return(-1);
          }
          return(2);
        }
      }
      pos = command.find("STOP");
      if(pos != std::string::npos) {
        if(pos == 0) {
          //perform command stop
          if(!session_started_flag) {
            std::cout << "Session hasn't been started yet\n";
            return(-1);
          }

          if(!session_executed_flag) {
            std::cout << "Session has been already stopped\n";
            return(-1);
          }
          return(3);
        } 
      }
      pos = command.find("QUIT");
      if(pos != std::string::npos) {
        // perform comman quit
        if(!session_started_flag) {
          std::cout << "Session hasn't been started yet\n";
          return(-1);
        }
        return(4);
      }
      std::cout << "Unknown command\n";
      return(-1);
    }
  
  private:

    /////////////////////////////////////////////////////////////////////
    // this private block includes realization of QUIT command routine //
    /////////////////////////////////////////////////////////////////////

    int _quit_command_routine() {
      _finish_all_slavers();
      return(0);
    }

  
  private:

    /////////////////////////////////////////////////////////////////////
    // this private block includes realization of STOP command routine //
    /////////////////////////////////////////////////////////////////////

    int _stop_command_routine() {

      // have to be implemented
      int status_command = 2;

      MPI_Request req = MPI_REQUEST_NULL;
      
      // broadcast the command among the slavers
      MPI_Ibcast(&status_command, 1, MPI_INT, 0, MPI_COMM_WORLD, &req);

      MPI_Wait(&req, MPI_STATUS_IGNORE);


      std::vector<int> its(_world_size - 1, 0);
      for(int i = 1 ; i < _world_size; ++i) {
        int curr_it = 0;
        MPI_Recv(&curr_it, 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        its[i - 1] = curr_it;
      }

      auto result = std::max_element(its.begin(), its.end());
      int end_iteration = *result;

      MPI_Bcast(&end_iteration, 1, MPI_INT, 0, MPI_COMM_WORLD);

      while(true) {
        int sum = 0;

        std::vector<int> stat = _runners_status();
        for(auto s : stat) {
          if(s > 0) {
            sum += 1;
          }
        }

        if(sum > 0) {
          continue;
        }

        break;
      }

      std::cout << "--------Stopped iteration---------\n";
      std::cout << "  " << end_iteration << "\n";
      std::cout << "----------------------------------\n";
      _status_command_routine();
      
      session_executed_flag = false;
      
      return(0);
    }
  
  private:

    ///////////////////////////////////////////////////////////////////////
    // this private block includes realization of STATUS command routine //
    ///////////////////////////////////////////////////////////////////////

    int _status_command_routine() {

      if(!session_executed_flag) {
        // the runners have finished already
        std::cout << "-----Current status of runners----\n";
        std::cout << "  all slavers have been finished\n";
        std::cout << "----------------------------------\n";
        std::cout << "-----Current block image----------\n";
        _print_block();
        std::cout << "----------------------------------\n";
        return(0);
      }

      int status_command = 1;

      MPI_Request req = MPI_REQUEST_NULL;
      
      // broadcast the command among the slavers
      MPI_Ibcast(&status_command, 1, MPI_INT, 0, MPI_COMM_WORLD, &req);

      MPI_Wait(&req, MPI_STATUS_IGNORE);

      // collecting results
      std::vector<int> its(_world_size - 1, 0);
      for(int i = 1 ; i < _world_size; ++i) {
        int curr_it = 0;
        MPI_Recv(&curr_it, 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        its[i - 1] = curr_it;
        std::pair<int, int> bd = _calculate_slaver_bound(i);
        for(int raw = bd.first; raw < bd.second ; ++raw) {
          MPI_Recv(game_field[raw], M, MPI_BYTE, i, raw - bd.first, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
      }

      // printing results
      std::cout << "-----Current status of runners----\n";
      for(int i = 1 ; i < _world_size ; ++i) {
        std::cout << "  slaver " << i << ":";
        if(its[i - 1] < 0) {
          std::cout << " finished\n";
        } else {
          std::cout << " on " << its[i - 1] << " iteration\n";
        }
      }
      std::cout << "----------------------------------\n";
      std::cout << "-----Current block image----------\n";
      _print_block();
      std::cout << "----------------------------------\n";

      if(_runners_finished()) {
        session_executed_flag = false;
      }

      return(0);
    }
  private:

    ///////////////////////////////////////////////////////////////////////////////////
    // this private block includes realization of slavers initialisation and running //
    ///////////////////////////////////////////////////////////////////////////////////

    std::pair<int, int> _calculate_slaver_bound(int slv_num) {
      int base = N/(_world_size - 1);
      int bias = N % (_world_size - 1);
      if(slv_num - 1 < bias) {
        return(
          std::pair<int, int>(
            (base + 1) * (slv_num - 1), 
            (base + 1) * slv_num));
      } else {
        return(
          std::pair<int, int>(
            bias * (base + 1) + (slv_num - 1 - bias) * base, 
            bias * (base + 1) + (slv_num - bias) * base));
      }
    }

    void _initialize_slavers() {
      if(N < _world_size - 1) {
        std::cout << "N is to small\n";
        assert(1 == 0);
      }

      int start_signal = 1;
      MPI_Bcast(&start_signal, 1, MPI_INT, 0, MPI_COMM_WORLD);

      // here I send the message for each process, which includes the initial field
      for(int i = 1 ; i < _world_size ; ++i) {
        std::pair<int, int> slv_bound = _calculate_slaver_bound(i);

        // send message, which specifies M parameter to the slaver
        MPI_Send(&M, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD);

        // send message, which specifies N parameter to the slaver
        int curr_N = slv_bound.second - slv_bound.first;
        MPI_Send(&curr_N, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

        for(int num = slv_bound.first ; num < slv_bound.second ; ++num) {
          MPI_Send(game_field[num], M, MPI_BYTE, i, 0, MPI_COMM_WORLD);
        }

      }

    }

    void _run_slavers(size_t it_count) {
      MPI_Bcast(&it_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }

    // this function collects information about 
    // current status of slavers
    std::vector<int> _runners_status() {
      int check_command = 3;
      MPI_Request req = MPI_REQUEST_NULL;
      
      MPI_Ibcast(&check_command, 1, MPI_INT, 0, MPI_COMM_WORLD, &req);

      MPI_Wait(&req, MPI_STATUS_IGNORE);

      // if req > 0, so this means, that the appropriate slaver 
      // in work status
      std::vector<int> res(_world_size - 1, 0);
      for(int i = 1 ; i < _world_size ; ++i) {
        int req = 0;
        MPI_Recv(&req, 1, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        res[i - 1] = req;
      } 
      return(res);
    }

    bool _runners_finished() {
      int sum = 0;

      std::vector<int> stat = _runners_status();
      for(auto s : stat) {
        if(s > 0) {
          sum += 1;
        }
      }

      if(sum > 0) {
        return false;
      }
      
      int finish_command = 4;
       MPI_Request req = MPI_REQUEST_NULL;
      
      // send finish command
      MPI_Ibcast(&finish_command, 1, MPI_INT, 0, MPI_COMM_WORLD, &req);

      MPI_Wait(&req, MPI_STATUS_IGNORE);
      return true;
    }

  void _finish_all_slavers() {
    if(session_executed_flag) {
      _stop_command_routine();
    }

    int finish_signal = 0;
    MPI_Bcast(&finish_signal, 1, MPI_INT, 0, MPI_COMM_WORLD);
  }


  private:

    //////////////////////////////////////////////////////////////////////
    // this private block includes realization of START command routine //
    //////////////////////////////////////////////////////////////////////

    void _print_block() {
      for(int i = 0 ; i < N ; ++i) {
        for(int j = 0 ; j < M ; ++j) {
          std::cout << game_field[i][j];
        }
        std::cout << '\n';
      }
    }

    int _start_command_routine(std::string & command) {

      int res = _start_initialisation(command);
      if(res == -1) {
        std::cout << "An error occurred during initialisation\n";
        return(-1);
      }
      session_started_flag = true;
      std::cout << "Initial disposition is following:\n";
      _print_block();
      return(0);
    }

    void _generate_init_disposition() {
      std::srand(std::time(nullptr));
      for(int i = 0 ; i < N ; ++i) {
        for(int j = 0 ; j < M ; ++j) {
          game_field[i][j] = std::rand() % 2;
        }
      }
    }

    int _size_game_field_init() {
      bool* (*fields) = new bool * [N];
      game_field = fields;
      for(int i = 0 ; i < N ; ++i) {
        bool * temp =  new bool[M];
        game_field[i] = temp;
      }

      _generate_init_disposition();

      return(0);
    }

    int _file_game_field_init(std::string & file_name) {
      std::ifstream input;
      try{
        input.open(file_name, std::ifstream::in);
      } catch(...) {
        return(-1);
      }
      std::string line = "";
      size_t count = 0;
      std::getline(input, line);
      for(auto c: line) {
        if(c == '0' || c == '1') {
          ++count;
        }
      }
      M = count;
      std::vector<bool * > v;
      while(!line.empty()) {
        bool * curr_str = nullptr;
        try{
          int i = 0;
          curr_str = new bool[M];
          for(auto c : line) {
            if(c == '1') {
              curr_str[i] = true;
              ++i;
            } else if(c == '0') {
              curr_str[i] = false;
              ++i;
            }
          }
          v.push_back(curr_str);
          std::getline(input, line);

        } catch(...) {
          delete [] curr_str;
          for(auto val : v) {
            delete [] val;
          }
          return(-1);
        }
        
      }

      N = v.size();
      bool* (*fields) = new bool * [N];
      game_field = fields;

      for(int i = 0 ; i < N ; ++i) {
        game_field[i] = v[i];
      }
      input.close();
      return(0);
    }

    int _start_initialisation(std::string & command) {
      // default parameters of the game
      std::string file_name = "";

      // parse command into divided words
      command = command.substr(5, command.size() - 5);
      std::string word = "";
      std::vector<std::string> cmds;
      while(true) {
        word = GetWord(command);
        if(word.empty()) {
          break;
        }
        else cmds.push_back(word);
      }

      // field information specification
      if(!cmds.empty()) {
        std::string cmd = cmds[0];
        if(cmd == "-s" || cmd == "--size") {
          try{
            N = std::stoi(cmds[1]);
            M = std::stoi(cmds[2]);
            return(_size_game_field_init());
          } catch(...) {
            return(-1);
          }
        } else if(cmd == "-f" || cmd == "--file") {
          try{
            file_name = cmds[1];
            return(_file_game_field_init(file_name));
          } catch(...) {
            return(-1);
          }
        }
      }

      if(!cmds.empty()) {
        return(-1);
      }

      // by default a field of 10x10 generated
      return(_size_game_field_init());
    }
  
  private:

    ////////////////////////////////////////////////////////////
    // this private block includes realization of RUN command //
    ////////////////////////////////////////////////////////////

    int _run_command_routine(std::string & command) {
      int it_count = _run_iterations(command);
      if(it_count > 0) {

        _initialize_slavers();
        _run_slavers(size_t(it_count));

        session_executed_flag = true;

      } else {
        return(-1);
      }
      return(0);
    }

    int _run_iterations(std::string & command) {
      int it_count = 0;
      command = command.substr(3, command.size() - 3);
      std::string word = GetWord(command);
      if(word.empty()) {
        std::cout << "Count of interations not specified\n";
        return(-1);
      }
      try {
        it_count = std::stoi(word);
      } catch(...) {
        std::cout << "An error occurred\n";
        return(-1);
      }
      if(it_count < 0) {
        std::cout << "An error occurred\n";
        return(-1);
      }
      return(it_count);
    }

  private:

    bool session_started_flag{false};
    bool session_executed_flag{false};

    size_t _world_size{0};
    size_t _world_rank{0};
    size_t N{10};
    size_t M{10};

    bool * (*game_field){nullptr};
};