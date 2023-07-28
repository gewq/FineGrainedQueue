#include <iostream>

#include "FineGrainedQueue/FineGrainedQueue.h"

int main()
{
  try{
    fine_grained_queue::test();
  }
  catch (std::exception& error) {
    std::cerr << error.what() << std::endl;
  }
  catch (...) {
    std::cerr << "Undefined exception" << std::endl;
  }
  return EXIT_SUCCESS;
}