#include <atomic>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#define MAXTHREADS 100
#define LOOP 200000000
#define TARGET 50000000
#define BENCH_RUNS 10


using namespace std;

class threadId{
  private:
    int count;
    map<thread::id,int> id;
    mutex m;

  public:
    threadId(){
      count = 0;
    }

    int getId(thread::id tid){
      int threadLocalId;
      m.lock();
      if(id.find(tid) != id.end()) threadLocalId = id[tid];
      else {
        threadLocalId=count;
        id[tid]=count;
        count++;
      } 
      m.unlock();
      return threadLocalId;
    }

    void reset(){
      count=0;
      id.clear();
    }
};

template <typename V=int> class counter {
  private:
    atomic<V> gcount;
    V local_lcounts [MAXTHREADS];
    V local_lgcounts [MAXTHREADS];
    threadId tid;

  public:
    counter(){
      init();
    }

    void init(){
      for(int i=0; i<MAXTHREADS;i++){
        local_lcounts[i]=0;
        local_lgcounts[i]=0;
      }
    }

    int getId(){
      return tid.getId(this_thread::get_id());
    }
    
    void weakInc(int id, V tosum={1}){ //weakUpdate 
      local_lcounts[id] += tosum;
    }

    V weakValue(int id){ //weakRead
      int readed = local_lgcounts[id] + local_lcounts[id];
      return readed;
    }

    void merge(int id){
      gcount += local_lcounts[id];
      local_lgcounts[id]=gcount;
      local_lcounts[id]=0;

    }

    V strongValue(){
      V globalValue = gcount;
      return globalValue;
    }

    void reset(){
      gcount=0;
      init();
      tid.reset();
    }
};

counter<int> mdt;
vector<int> NTHREADS;
int SYNCFREQ [6] = {1,8,64,512,4096,32768};

void work(int syncFreqIndex){
    int id= mdt.getId();
    mdt.merge(id);

    for (int i=0; i < LOOP; i++){
      if(i%SYNCFREQ[syncFreqIndex] == 1){ //due to almost deterministic intermediate results, we sync more often
        mdt.merge(id);
      }

      if(mdt.weakValue(id) >= TARGET) {
        mdt.merge(id);
        break;
      }

      mdt.weakInc(id);

      if(i%SYNCFREQ[syncFreqIndex] == 0){
        mdt.merge(id);
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