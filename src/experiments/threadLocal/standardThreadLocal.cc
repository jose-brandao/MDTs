#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <list>
#include <thread>
#include <vector>


#define LOOP 200000000
#define TARGET 50000000
#define BENCH_RUNS 5

using namespace std;

template <typename V=int> class counter {
  private:
    atomic<V> gcount;

  public:
    static thread_local V lcount;
    static thread_local V lgcount;

    void weakInc(V tosum={1}){ //weakUpdate
      lcount += tosum;
    }

    V weakValue(){ //weakRead
      int readed = lgcount + lcount;
      return readed;
    }

    void merge(){
      gcount += lcount;
      lgcount = gcount;
      lcount = 0;
    }

    V strongValue(){
      V globalValue = gcount;
      return globalValue;
    }

    void incToTarget(V target, V syncfreq){
      merge();
      int loop = target*10;

      for (int i=0; i < loop; i++){

        if(i%syncfreq == 1){ //due to almost deterministic intermediate results, we sync more often
          merge();
        }

        if(weakValue() >= target) {
          merge();
          break;
        }

        weakInc();

        if(i%syncfreq == 0){
          merge();
        }

      }
    }

    void reset(){
      gcount = 0;
    }
};

counter<int> mdt;

template<>
thread_local int counter<int>::lcount = 0;
template<>
thread_local int counter<int>::lgcount = 0;

vector<int> NTHREADS;
int SYNCFREQ [6] = {1,8,64,512,4096,32768};

void work(int syncFreqIndex){

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

      thread t[NTHREADS[k]];
      for (int a=0; a < NTHREADS[k]; a++){
        t[a]=thread(work,syncFreqIndex);
      }

      // Work baby threads, work ...
      for (int a=0; a < NTHREADS[k]; a++){
        t[a].join();
      }

      steady_clock::time_point t2 = steady_clock::now();

      duration<double> ti = duration_cast<duration<double>>(t2 - t1);

      times.push_back(ti.count());
      counters.push_back(mdt.strongValue());
      throughs.push_back(mdt.strongValue()/ti.count());

      mdt.reset();
    }

    double sumTimes = 0;
    for(double t: times){
      sumTimes += t;
    }

    double finalTime = sumTimes/times.size();
    cout << fixed;
    //cout <<  "time: " << finalTime << endl;

    double sumCounters = 0;
    for(double c: counters){
      sumCounters += c;
    }
    double finalCounter = (int)(sumCounters/counters.size());
    //cout << "mdtc counter: " << finalCounter << endl;

    double sumTroughs = 0;
    for(double th: throughs){
      sumTroughs += th;
    }
    double finalThroughs = sumTroughs/throughs.size();
    //cout << "troughput: " << finalThroughs << endl << endl;

    cout << (int)finalThroughs << "," << ((finalCounter-TARGET)*100)/TARGET << "," << NTHREADS[k] << endl;
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