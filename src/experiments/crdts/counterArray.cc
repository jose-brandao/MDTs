#include <iostream>
#include <algorithm>
#include <mutex>
#include <vector>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>

using namespace std;
#define MAXTHREADS 100
#define TARGET 500000
#define LOOP 3000000
#define BENCH_RUNS 5


class ThreadId{
    private:
        int count;
        map<boost::thread::id, int> id;
        mutex m;

    public:
        ThreadId(){
            count = 0;
        }

        int getId(boost::thread::id tid){
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

        int getCount(){
            return count;
        }

        void reset(){
            count=0;
            id.clear();
        }
};

class InnerCounter{
    private:
        int counters[MAXTHREADS];
        int threadNumber;

    public:
        InnerCounter(int threadNo){
            threadNumber = threadNo;
            for(int i=0;i<threadNumber; i++){
                counters[i]=0;
            }
        }

        int getCounterEntry(int entryIndex){
            return counters[entryIndex];
        }

        int getValue(){
            int counterValue = 0;
            for(int i=0; i < threadNumber; i++){
                counterValue += counters[i]; 
            }
            return counterValue;
        }

        void increment(int i){
            counters[i]++;
        }

        void merge(InnerCounter from){
            for(int i=0; i < threadNumber; i++){
                counters[i] = max(counters[i], from.getCounterEntry(i));
            }
        }

        void reset(){
            for(int i=0;i<threadNumber; i++){
                counters[i]=0;
            }
        }
};


class CRDTCounter{
    private:
        InnerCounter globalCounter;
        InnerCounter *localCounters [MAXTHREADS];
        ThreadId threadId;
        mutex m;

    public:
        CRDTCounter(int threadNumber) : globalCounter(threadNumber){
            for(int i=0; i<threadNumber; i++){
                localCounters[i] = new InnerCounter(threadNumber);
            }
        }

        int getValue(){
            int threadLocalId = threadId.getId(boost::this_thread::get_id());
            return localCounters[threadLocalId]->getValue();
        }

        int getGlobalValue(){
            return globalCounter.getValue();
        }

        void increment(){
            int threadLocalId = threadId.getId(boost::this_thread::get_id());
            localCounters[threadLocalId]->increment(threadLocalId);       
        }

        void merge(){
            int threadLocalId = threadId.getId(boost::this_thread::get_id());
            m.lock();
                globalCounter.merge(*localCounters[threadLocalId]);
                localCounters[threadLocalId]->merge(globalCounter);
            m.unlock();
        }

        void reset(){
            globalCounter.reset();
            for(int i=0; i<threadId.getCount(); i++){
                localCounters[i]->reset();
            }
            threadId.reset();
        }
};

CRDTCounter crdt(100);
vector<int> NTHREADS;
int SYNCFREQ [6] = {1,8,64,512,4096,32768};

void work(int syncFreqIndex){

  crdt.merge();

  for (int i=0; i < LOOP; i++){

    if(i%SYNCFREQ[syncFreqIndex] == 1){
      crdt.merge();
    }

    if(crdt.getValue() >= TARGET) {
      crdt.merge();
      break;
    }

    crdt.increment();

    if(i%SYNCFREQ[syncFreqIndex] == 0){
      crdt.merge();
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
        counters.push_back(crdt.getGlobalValue());
        throughs.push_back(crdt.getGlobalValue()/ti.count());

        crdt.reset();
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

      double overshoot = ((finalCounter-TARGET)*100)/TARGET;

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

