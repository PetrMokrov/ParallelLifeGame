#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <atomic>

#include "barrier.hpp"
#include "executor.hpp"

std::string spaces = "\t \n";

std::string GetWord(std::string & str) {
  while(!str.empty() && !(spaces.find(str[0]) == std::string::npos)) {
    str.erase(0, 1);
  }
  int i = 0;
  while(i + 1 <= str.size() && spaces.find(str[i]) == std::string::npos) {
    ++i;
  }
  std::string word = "";
  if(i > 0) {
    word = str.substr(0, i);
  }
  str.erase(0, i);
  return(word);
}


class LifeGameMaster {
  public:

    explicit LifeGameMaster() {
    }

    ~LifeGameMaster() {
      if(game_new_field != nullptr) {
        for(int i = 0 ; i < N  ; ++i) {
          delete [] game_new_field[i];
          delete [] game_old_field[i];
        }
        delete [] game_new_field;
        delete [] game_old_field;
      }
      if(sync_barrier != nullptr) {
        delete sync_barrier;
      }
      if(swap_barrier != nullptr) {
        delete swap_barrier;
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
          /*
          if(session_executed_flag) {
            std::cout << "Session has been already run\n";
            return(-1);
          }
          */
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
      _stop_command_routine();
      return(0);
    }

  
  private:

    /////////////////////////////////////////////////////////////////////
    // this private block includes realization of STOP command routine //
    /////////////////////////////////////////////////////////////////////

    int _stop_command_routine() {
      if(!work_status.load()) {
        std::cout << "-----Stop------\n";
        std::cout << "No launched processes\n";
        std::cout << "current field disposition:\n";
        for(int i = 0 ; i < N ; ++i) {
          for(int j = 0 ; j < M ; ++j) {
            std::cout << game_old_field[i][j];
          }
          std::cout << '\n';
        }
        std::cout << "-----------------\n";

        work_status.store(false);
        stop_signal.store(false);
        return(0);
      }

      int prev_it_num = it_num.load();

      stop_signal.store(true);

      while(it_num.load() == prev_it_num) {
        // wait for better times
        if(!work_status.load()) {
          std::cout << "-----Stop------\n";
          std::cout << "No launched processes\n";
          std::cout << "current field disposition:\n";
          for(int i = 0 ; i < N ; ++i) {
            for(int j = 0 ; j < M ; ++j) {
              std::cout << game_old_field[i][j];
            }
            std::cout << '\n';
          }
          std::cout << "-----------------\n";

          work_status.store(false);
          stop_signal.store(false);
          return(0);
        }
      }

      std::cout << "-----Stop------\n";
      std::cout << "stopped interation is : " << it_num.load() << "\n";
      std::cout << "current field disposition:\n";
      for(int i = 0 ; i < N ; ++i) {
        for(int j = 0 ; j < M ; ++j) {
          std::cout << game_old_field[i][j];
        }
        std::cout << '\n';
      }
      std::cout << "-----------------\n";

      executor.Join();
      sync_barrier->RecoverGates();
      swap_barrier->RecoverGates();
      work_status.store(false);
      stop_signal.store(false);
      
      return(0);
    }
  
  private:

    ///////////////////////////////////////////////////////////////////////
    // this private block includes realization of STATUS command routine //
    ///////////////////////////////////////////////////////////////////////

    int _status_command_routine() {
      if(!work_status.load()) {
        std::cout << "-----Status------\n";
        std::cout << "No launched processes\n";
        std::cout << "current field disposition:\n";
        for(int i = 0 ; i < N ; ++i) {
          for(int j = 0 ; j < M ; ++j) {
            std::cout << game_old_field[i][j];
          }
          std::cout << '\n';
        }
        std::cout << "-----------------\n";
        return(0);
      }

      int prev_it_num = it_num.load();

      status_signal.store(-1);

      while(it_num.load() == prev_it_num) {
        // wait for better times
        if(!work_status.load()) {
          std::cout << "-----Status------\n";
          std::cout << "No launched processes\n";
          std::cout << "current field disposition:\n";
          for(int i = 0 ; i < N ; ++i) {
            for(int j = 0 ; j < M ; ++j) {
              std::cout << game_old_field[i][j];
            }
            std::cout << '\n';
          }
          std::cout << "-----------------\n";
          return(0);
        }
      }

      std::cout << "-----Status------\n";
      std::cout << "current interation is : " << it_num.load() << "\n";
      std::cout << "current field disposition:\n";
      for(int i = 0 ; i < N ; ++i) {
        for(int j = 0 ; j < M ; ++j) {
          std::cout << game_old_field[i][j];
        }
        std::cout << '\n';
      }
      std::cout << "-----------------\n";

      status_signal.store(0);
      status_cv.notify_all();

      return(0);
    }

    std::atomic<int> it_num{0};
  private:

    ///////////////////////////////////////////////////////////////////////////////////
    // this private block includes realization of slavers initialisation and running //
    ///////////////////////////////////////////////////////////////////////////////////

    std::pair<size_t, size_t> _calculate_slaver_bound(int slv_num) {
      size_t base = N/threads_count;
      size_t bias = N % threads_count;
      if(slv_num - 1 < bias) {
        return(
          std::pair<size_t, size_t>(
            (base + 1) * (slv_num - 1), 
            (base + 1) * slv_num));
      } else {
        return(
          std::pair<size_t, size_t>(
            bias * (base + 1) + (slv_num - 1 - bias) * base, 
            bias * (base + 1) + (slv_num - bias) * base));
      }
    }

    void _initialize_slavers() {
      // we change the count of threads, if N < threads_count
      if(N < threads_count) {
        threads_count = N;
      }
      if(threads_count > 1) {
        Barrier * new_barrier = new Barrier(threads_count);
        sync_barrier = new_barrier;
        new_barrier = new Barrier(threads_count);
        swap_barrier = new_barrier;
      }
    }

    void _run_slavers(size_t it_count) {
      executor.Join();
      // we change the count of threads, if N < threads_count
      if(threads_count == 1) {
        // launch one slaver
        std::cout << "Single thread job not supported yet\n";
        return;
        executor.Submit(&LifeGameMaster::_single_slaver_job, this, it_count);
      } else if (threads_count > 1) {
        for (int t = 0; t < threads_count; ++t) {
          std::pair<size_t, size_t> bounds = _calculate_slaver_bound(t + 1);
          executor.Submit(&LifeGameMaster::_mul_slaver_job, this, it_count, bounds.first, bounds.second);
        }
      }
      work_status.store(true);
    }

    bool _test_life(int x, int y, bool * (*game_field)) {
      size_t lived_neighbour_count = 0;
      bool currently_lived = game_field[x][y];
      lived_neighbour_count += game_field[(x + 1) % N][y];
      lived_neighbour_count += game_field[(x + 1) % N][(y + 1) % M];
      lived_neighbour_count += game_field[x][(y + 1) % M];
      lived_neighbour_count += game_field[(N + x - 1) % N][(y + 1) % M];
      lived_neighbour_count += game_field[(N + x - 1) % N][y];
      lived_neighbour_count += game_field[(N + x - 1) % N][(M + y - 1) % M];
      lived_neighbour_count += game_field[x][(M + y - 1) % M];
      lived_neighbour_count += game_field[(x + 1) % N][(M + y - 1) % M];
      if(lived_neighbour_count == 3) return(true);
      if(currently_lived && lived_neighbour_count == 2) return(true);
      return(false);
    }

    void _single_slaver_job(size_t it_count) {

      for(int i = 0 ; i < it_count ; ++i) {
        bool * (*buff) = game_new_field;
        game_new_field = game_old_field;
        game_old_field = buff;
        for(int x = 0; x < N ; ++x) {
          for(int y = 0 ; y < M ; ++y) {
            game_new_field[x][y] = _test_life(x, y, game_old_field);
          }
        }
        // this is the place for management tools
      }
      _print_block();
    }

    void _mul_slaver_job(size_t it_count, size_t bottom_bound, size_t top_bound) {
      for(int i = 0 ; i < it_count ; ++i) {

        // synchronisation part

        sync_barrier->PassThrough();

        if(stop_signal.load()) {
          sync_barrier->BreakGates();
          swap_barrier->BreakGates();
          it_num.store(i);
          return;
        }

        if(change_fields_flag.fetch_add(1) == 0) {
          // replace fields
          bool * (*buff) = game_new_field;
          game_new_field = game_old_field;
          game_old_field = buff;
        }


        swap_barrier->PassThrough();

        if(stop_signal.load()) {
          sync_barrier->BreakGates();
          swap_barrier->BreakGates();
          it_num.store(i);
          return;
        }

        change_fields_flag.store(0);

        // status management tools;
        // not optimal!
        if(status_signal.load() != 0) {
          it_num.store(i);
          std::unique_lock<std::mutex> lock{mutex_};
          status_cv.wait(lock, [this](){ return status_signal.load() == 0; });
        }

        // game life part

        for(size_t x = bottom_bound; x < top_bound ; ++x) {
          for(size_t y = 0 ; y < M ; ++y) {
            game_new_field[x][y] = _test_life(x, y, game_old_field);
          }
        }
      }
      sync_barrier->PassThrough();
      work_status.store(false);
    }

    // stop command interface
    std::atomic<bool> stop_signal{false};

    // status command interface
    std::mutex mutex_;
    std::atomic<int> status_signal{0};
    std::condition_variable status_cv;
    std::atomic<bool> work_status{false};

    std::atomic<int> change_fields_flag{0};
    Barrier * sync_barrier{nullptr};
    Barrier * swap_barrier{nullptr};
    Executor executor;


  private:

    //////////////////////////////////////////////////////////////////////
    // this private block includes realization of START command routine //
    //////////////////////////////////////////////////////////////////////

    void _print_block() {
      for(int i = 0 ; i < N ; ++i) {
        for(int j = 0 ; j < M ; ++j) {
          std::cout << game_new_field[i][j];
        }
        std::cout << '\n';
      }
    }

    int _start_command_routine(std::string & command) {
      // launching slavers
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
          game_new_field[i][j] = std::rand() % 2;
        }
      }
    }

    int _size_game_field_init() {
      bool* (*fields) = new bool * [N];
      game_new_field = fields;
      fields = new bool * [N];
      game_old_field = fields;
      for(int i = 0 ; i < N ; ++i) {
        bool * temp =  new bool[M];
        game_new_field[i] = temp;
        temp = new bool[M];
        game_old_field[i] = temp;
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
      game_new_field = fields;
      fields = new bool * [N];
      game_old_field = fields;

      for(int i = 0 ; i < N ; ++i) {
        game_new_field[i] = v[i];
        bool * temp =  new bool[M];
        game_old_field[i] = temp;
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

      // count of threads command processing
      // if threads_count is not specificated, 
      // 1 used by default
      if(!cmds.empty()) {
        std::string cmd = cmds[0];
        if(cmd == "-t" || cmd == "--threads") {
          try{
            threads_count = std::stoi(cmds[1]);
          } catch (...) {
            return(-1);
          }
          cmds.erase(cmds.begin(), cmds.begin() + 2);
        }
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
        if(work_status.load()) {
          std::cout << "The process cun not be launched now\n";
          return(-1);
        }
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

    size_t threads_count{1};
    size_t N{10};
    size_t M{10};
    bool * (*game_new_field){nullptr};
    bool * (*game_old_field){nullptr};
};

class LifeGame {
  public:

    explicit LifeGame(void) = default;

    // this function starts LifeGame session
    void Launch(void) {
      master.StartCommandListen();
    }


  private:
    LifeGameMaster master;
};