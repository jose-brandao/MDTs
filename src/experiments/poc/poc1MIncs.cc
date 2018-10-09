#include <atomic>
#include <iostream>
#include <mutex>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>

#define LOOP 1000000
#define BENCH_RUNS 5

using namespace std;

unsigned int c=0;
atomic<unsigned int> v;
mutex m;
boost::thread_specific_ptr<unsigned int> t;


void noSyncInc(){
  for (int i=0; i < LOOP; i++){
    c++;
  }
}

void lockInc(){
  for (int i=0; i < LOOP; i++){
    m.lock();
    c++;
    m.unlock();
  }
}

void atomicInc(){
  for (int i=0; i < LOOP; i++){
    v++;
  }
}

void threadLocalInc(){
  t.reset(new unsigned int(0)); //init thread local var
  for (int i=0; i < LOOP; i++){
    *t+=1;
  }
  m.lock();
  c=c + *t;
  m.unlock();
}


vector<int> NTHREADS;

void benchmark(void (*function)()){
    using namespace std::chrono;
    for(int k = 0; k < NTHREADS.size(); k++){
      std::list<double> times;

      for(int i= 0; i< BENCH_RUNS; i++){
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
      cout << "THREADS NO: " << NTHREADS[k] << endl;
      cout << "Time: " << finalTime << endl << endl;
      c=0;
      v=0;
    }
}

int main(int argc, char** argv){
  for(int i=1; i<argc; i++){
      NTHREADS.push_back(atoi(argv[i]));
  }
  cout << "***************** NO SYNC *********************" << endl;
  benchmark(noSyncInc);

  cout << "***************** LOCK *********************" << endl;
  benchmark(lockInc);

  cout << "***************** ATOMIC *********************" << endl;
  benchmark(atomicInc);

  cout << "***************** THREAD LOCAL *********************" << endl;
  benchmark(threadLocalInc);
}