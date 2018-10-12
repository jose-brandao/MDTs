#include <boost/lockfree/queue.hpp>
#include <boost/thread/thread.hpp>
#include <thread>
#include <iostream>

#define LOOP 500000
#define BENCH_RUNS 5

using namespace std;

boost::lockfree::queue<int> q{LOOP};

vector<int> NTHREADS;
int poped;
void lockFree(){
  for (int i=0; i < LOOP; i++){
    q.push(i);
    
    if(i%1000==0){ 
      q.pop(poped);
    }
  }
}

void benchmark(void (*function)()){
    using namespace std::chrono;
    for(int k = 0; k < NTHREADS.size(); k++){
      std::list<double> times;

      for(int i = 0; i< BENCH_RUNS; i++){
        steady_clock::time_point t1 = steady_clock::now();

        boost::thread_group threads;
        for (int a=0; a < NTHREADS[k]; a++){
          threads.create_thread(function);
        }
        threads.join_all();

        steady_clock::time_point t2 = steady_clock::now();

        duration<double> ti = duration_cast<duration<double>>(t2 - t1);

        times.push_back(ti.count());
      }

      double sumTimes = 0;
      for(double t: times){
        sumTimes += t;
      }

      double finalTime = sumTimes/times.size();
      cout << fixed;
      cout << "Time: " << finalTime << endl << endl;
      cout << "THREADS NO: " << NTHREADS[k] << endl;
      cout << "Throughput: " << (int)(LOOP/finalTime) << endl;

    }
}

int main(int argc, char** argv){
  for(int i=1; i<argc; i++){
      NTHREADS.push_back(atoi(argv[i]));
  }
  cout << "***************** LOCK FREE *********************" << endl;
  benchmark(lockFree);
}