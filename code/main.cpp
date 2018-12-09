#include <iostream>
#include <pthread.h>
#include "life_game.hpp"


int main(int argc, char const *argv[])
{
  LifeGame lg;
  lg.Launch();
  return 0;
}
