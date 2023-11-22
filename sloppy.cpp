#include<iostream>
#include<pthread.h>
#include<semaphore.h>
#include<vector>

using std::cout;
using std::endl;
using std::string;
using std::vector;

typedef struct __shared {
    int sloppiness;
    float work_time;
    int work_iterations;
    bool cpubound;

    int * local_counts;
    
    sem_t mutex;
    int global_count;

} shared;

typedef struct __thread_info {
  shared sh;
  int thread_index;
} thread_info;

int main(int argc, char * argv[]) {
  string usage = "\nUsage: sloppySim <N_Threads = 2> <Sloppiness = 10> <work_time = 10> <work iterations = 100> <CPU_BOUND = false> <Do_Logging = false>";

  // convert argv to string[]
  vector<string> argList(argv, argv + argc);

  // set argument defaults
  int num_threads = 2;
  int sloppiness = 10;
  int ms_work_time = 10;
  int work_iterations = 100;
  bool cpubound = false;
  bool do_logging = false;

  // parse cl arguments (very lazily; may not detect if bool inputs are incorrect)
  switch(argc) {
    case 7:
      if (argList[6] == "true") {
        do_logging = true;
      }
    
    case 6:
      if (argList[5] == "true") {
        cpubound = true;
      }

    case 5:
      try {
        work_iterations = stoi(argList[4]);
      }
      catch (...) {
        cout << "Could not parse work iterations." << usage << endl;
        return 1;
      }

    case 4:
      try {
        ms_work_time = stoi(argList[3]);
      }
      catch (...) {
        cout << "Could not parse work time." << usage << endl;
        return 1;
      }

    case 3:
      try {
        sloppiness = stoi(argList[2]);
      }
      catch (...) {
        cout << "Could not parse sloppiness." << usage << endl;
        return 1;
      }

    case 2:
      try {
        num_threads = stoi(argList[1]);
      }
      catch (...) {
        cout << "Could not parse number of threads." << usage << endl;
        return 1;
      }

    case 1:
      break;

    default:
      cout << "Incorrect number of arguments." << usage << endl;
      return 1;
  }


}