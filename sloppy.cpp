#include<iostream>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>
#include<vector>

using std::cout;
using std::endl;
using std::string;
using std::vector;

typedef struct __shared {
    int sloppiness;
    int work_time;
    int work_iterations;
    bool cpubound;

    int * local_counts;
    
    sem_t mutex;
    int global_count;

} shared;

typedef struct __thread_info {
  shared * sh;
  int thread_index;
} thread_info;

void * counter_thread(void * arg) {

}

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

  // parse cl arguments (very lazily; ignores incorrect bool inputs)
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

  // initialize shared data
  shared sh;
  sem_init(&sh.mutex, 0, 1);

  sh.sloppiness = sloppiness;
  sh.work_time = ms_work_time;
  sh.work_iterations = work_iterations;
  sh.cpubound = cpubound;
  sh.local_counts = new int[num_threads];
  for (int i = 0; i < num_threads; i++) {
    sh.local_counts[i] = 0;
  }
  sh.global_count = 0;

  // print settings for this run
  if (do_logging) {
    cout << "#threads:\t" << num_threads
         << "\nsloppiness:\t" << sloppiness
         << "\nwork time:\t" << work_iterations
         << "\nCPU bound:\t" << cpubound << endl;
  }

  // create threads
  pthread_t threads[num_threads];
  for (int i = 0; i < num_threads; i++) {
    thread_info ti;
    ti.sh = &sh;
    ti.thread_index = i;
    pthread_create(&threads[i], NULL, counter_thread, &ti);
  }

  // log the global and local counts periodically
  if (do_logging) {
    float ms_delay = ms_work_time * work_iterations / 10;
    cout << "Logging every " << ms_delay << " ms" << endl;

    int total_count;
    do {
      usleep(ms_delay * 1000);

      total_count = sh.global_count;
      cout << "Global Ct: " << total_count << "\t Local Cts: [";

      int local_count = sh.local_counts[0];
      total_count += local_count;
      cout << local_count;
      for (int i = 1; i < num_threads; i++) {
        local_count = sh.local_counts[i];
        total_count += local_count;
        cout << ", " << local_count;
      }

      cout << "]" << endl;
    } while (total_count < work_iterations * num_threads);
    cout << "Final Global Count: " << sh.global_count << endl;
  }
  
  return 0;
}