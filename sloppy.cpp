#include<stdlib.h>
#include<iostream>
#include<pthread.h>
#include<semaphore.h>
#include<time.h>
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
  thread_info * info = (thread_info*) arg;
  int ms_delay;
  for (int i = 0; i < info -> sh -> work_iterations; i++) {
    // "work"
    float random = (rand() / (float)RAND_MAX) + 0.5; // number in range [0.5, 1.5]
    ms_delay = (info -> sh -> work_time) * random;
    if (info -> sh -> cpubound) {
      for (int j = 0; j < ms_delay * 1000000; j++) {}
    }
    else {
      usleep(ms_delay * 1000);
    }

    // increment local counter. If we reach the threshold, dump bucket into global
    ++(info -> sh -> local_counts[info -> thread_index]);
    if ((info -> sh -> local_counts[info -> thread_index]) >= info -> sh -> sloppiness) {
      sem_wait(&(info -> sh -> mutex));
      (info -> sh -> global_count) += (info -> sh -> local_counts[info -> thread_index]);
      sem_post(&(info -> sh -> mutex));
      info -> sh -> local_counts[info -> thread_index] = 0;
    }
  }

  delete info;

  bool* ok = new bool;
  *ok = true;
  return ok;
}

int main(int argc, char * argv[]) {
  srand(time(NULL));
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
  cout << "#threads:\t\t" << num_threads
       << "\nsloppiness:\t\t" << sloppiness
       << "\nwork iterations:\t" << work_iterations
       << "\nCPU bound:\t\t" << cpubound << endl;

  // create threads
  pthread_t threads[num_threads];
  for (int i = 0; i < num_threads; i++) {
    thread_info * ti = new thread_info;
    ti->sh = &sh;
    ti->thread_index = i;
    pthread_create(&threads[i], NULL, counter_thread, ti);
  }

  // log the global and local counts periodically
  if (do_logging) {
    int ms_delay = ms_work_time * work_iterations / 10;
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
  
  // take care of loose ends
  delete[] sh.local_counts;
  bool * ret;
    for (int i = 0; i < num_threads; i++) {
      pthread_join(threads[i],(void**) &ret);
      delete ret;
    }

  return 0;
}