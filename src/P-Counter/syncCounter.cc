#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <iostream>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <vector>

#define LOOP 100000000
#define TARGET 50000000
#define BENCH_RUNS 5

using namespace std;

atomic<unsigned int> c;
vector<int> NTHREADS;

int inc(){
	  unsigned int oldValue = c.load();
    unsigned int newValue;

    do{
        if (oldValue < TARGET) newValue = oldValue + 1;
        else break;
    }
    while (!c.compare_exchange_weak(oldValue, newValue));

    return oldValue;
}

void work(){
	for(int i=0; i<LOOP; i++){
		if (inc() >= TARGET) break;
	}
}

void work2(){
 	for(int i=0; i<LOOP; i++){
 		if(c < TARGET) c++;
 		else break;
 	}
 }

void benchmark(){
	using namespace std::chrono;
    for(int k = 0; k < NTHREADS.size(); k++){
      std::list<double> times;
      std::list<double> counters;
      std::list<double> throughs;

      for(int i= 0; i< BENCH_RUNS; i++){
        steady_clock::time_point t1 = steady_clock::now();

        boost::thread_group threads;
        for (int a=0; a < NTHREADS[k]; a++){ 
          threads.create_thread(&work);
        }

        threads.join_all();

        steady_clock::time_point t2 = steady_clock::now();

        duration<double> ti = duration_cast<duration<double>>(t2 - t1);

        times.push_back(ti.count());
        counters.push_back(c);
        throughs.push_back(c/ti.count());
        
        cout << "VALUE: " << c << endl;
        c=0;
      }

      double sumTimes = 0;
      for(double t: times){
        sumTimes += t;
      } 

      double finalTime = sumTimes/times.size();
      cout << fixed;

      double sumCounters = 0;
      for(double c: counters){
        sumCounters += c;
      } 
      double finalCounter = (int)(sumCounters/counters.size());

      double sumTroughs = 0;
      for(double th: throughs){
        sumTroughs += th;
      } 
      double finalThroughs = sumTroughs/throughs.size();

      //double overshoot = ((finalCounter-TARGET)*100)/TARGET;

      cout << (int)finalThroughs << "," << NTHREADS[k] << endl;
    }
}

int main(int argc, char** argv){
  for(int i=1; i<argc; i++){
    cout << argv[i] << endl;
    NTHREADS.push_back(atoi(argv[i]));
  }
	benchmark();
}