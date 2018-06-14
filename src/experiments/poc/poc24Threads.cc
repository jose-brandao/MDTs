#include <atomic>
#include <iostream>
#include <mutex>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>

#define NTHREADS 24
#define BENCH_RUNS 10

using namespace std;

unsigned int c=0;
atomic<unsigned int> v;
mutex m;
boost::thread_specific_ptr<unsigned int> t;


void noSyncInc(int incNumber){
  for (int i=0; i < incNumber; i++){
    c++;
  }
}

void lockInc(int incNumber){
  for (int i=0; i < incNumber; i++){
    m.lock();
    c++;
    m.unlock();
  }
}

void atomicInc(int incNumber){
  for (int i=0; i < incNumber; i++){
    v++;
  }
}

void threadLocalInc(int incNumber){
  t.reset(new unsigned int(0)); //init thread local var
  for (int i=0; i < incNumber; i++){
    *t+=1;
  }
  m.lock();
  c=c + *t;
  m.unlock();
}

int incrementNumber[7] = {2000000,4000000,6000000,8000000,10000000,15000000,20000000};

void benchmark(void (*function)(int)){
    using namespace std::chrono;
    for(int k = 0; k < 7; k++){
      std::list<double> times;

      for(int i= 0; i< BENCH_RUNS; i++){
        steady_clock::time_point t1 = steady_clock::now();

        boost::thread_group threads;
        for (int a=0; a < NTHREADS; a++){
          threads.create_thread(boost::bind(function, boost::cref(incrementNumber[k])));
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
      cout << "INCREMENTS PER THREAD: " << incrementNumber[k] << endl;
      cout << "Time: " << finalTime << endl << endl;
      c=0;
      v=0;
    }
}

int main(){
  cout << "***************** NO SYNC *********************" << endl;
  benchmark(noSyncInc);

  cout << "***************** LOCK *********************" << endl;
  benchmark(lockInc);

  cout << "***************** ATOMIC *********************" << endl;
  benchmark(atomicInc);

  cout << "***************** THREAD LOCAL *********************" << endl;
  benchmark(threadLocalInc);
}