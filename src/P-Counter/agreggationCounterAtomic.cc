#include <atomic>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>

#define LOOP 200000000
#define TARGET 50000000
#define BENCH_RUNS 10

using namespace std;

template <typename V=int> class counter {
private:
  atomic<V> gcount;
  mutex m;
  boost::thread_specific_ptr<V> lcount;
  boost::thread_specific_ptr<V> lgcount;

public:

 void init(){ //instatiate thread local object parts
    lcount.reset(new V(0));
    lgcount.reset(new V(0));
  }

  void weakInc(V tosum={1}){ //weakUpdate
    *lcount += tosum;
  }

  void strongInc(V tosum={1}){ //strongUpdate
   gcount.fetch_add(tosum);
  }

  V weakValue(){ //weakRead
    V readed = *lgcount + *lcount;
    return readed;
  }

  V strongValue(){ //strongRead
    V readed = gcount + *lcount;
    return readed;
  }

  void merge(){ //mergeToGlobal
    gcount.fetch_add(*lcount); //merging thread local state with global state
    *lgcount = gcount;  // updating the snapshot
    *lcount = 0; //resetting thread local state
  }


  ////////////////ONLY FOR DEBUGGING/BENCHMARKING PURPOSES//////
  V getLgcount(){
    V i = *lgcount;
    return i;
  }

  V getLcount(){
    V i = *lcount;
    return i;
  }

  V readP(){ //To read the global state on main
      V ret;
      ret=gcount;
    return ret;
  }

  void reset(){
    gcount = 0;
  }

};

counter<int> mdt;
vector<int> NTHREADS;
int SYNCFREQ [6] = {1,8,64,512,4096,32768};

void work(int syncFreqIndex){

  mdt.init();

  mdt.merge();

  for (int i=0; i < LOOP; i++){

    if(i%SYNCFREQ[syncFreqIndex] == 1){
      mdt.merge();
    }

    if(mdt.weakValue() >= TARGET) {
      mdt.merge();
      break;
    }

    mdt.weakInc();

    if(i%SYNCFREQ[syncFreqIndex] == 0){
      mdt.merge();
    }

  }

}

void benchmarkPerFreq(int syncFreqIndex){
    using namespace std::chrono;
    for(int k = 0; k < NTHREADS.size(); k++){
      std::list<double> times;
      std::list<double> counters;
      std::list<double> throughs;

      for(int i= 0; i< BENCH_RUNS; i++){
        steady_clock::time_point t1 = steady_clock::now();

        boost::thread_group threads;
        for (int a=0; a < NTHREADS[k]; a++){
          threads.create_thread(boost::bind(work, boost::cref(syncFreqIndex)));
        }

        threads.join_all();

        steady_clock::time_point t2 = steady_clock::now();

        duration<double> ti = duration_cast<duration<double>>(t2 - t1);

        times.push_back(ti.count());
        counters.push_back(mdt.readP());
        throughs.push_back(mdt.readP()/ti.count());

        mdt.reset();
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


      double sumThroughs = 0;
      for(double th: throughs){
        sumThroughs += th;
      }
      double finalThroughs = sumThroughs/throughs.size();

      //double overshoot = ((finalCounter-TARGET)*100)/TARGET;

      cout << (int)finalThroughs << "," << NTHREADS[k] << endl;
    }
}

int main(int argc, char** argv){
    for(int i=1; i<argc; i++){
      NTHREADS.push_back(atoi(argv[i]));
    }

    for(int i=0; i<6;i++){
        cout << "***************SYNCFREQ: " << SYNCFREQ[i] << " ***************" << endl;
        benchmarkPerFreq(i);
        cout << endl;
    }

    return 0;
}
