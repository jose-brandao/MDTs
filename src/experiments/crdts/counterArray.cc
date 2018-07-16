#include <iostream>
#include <algorithm>
#include <mutex>
#include <vector>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>

using namespace std;
#define MAXTHREADS 100


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
};

CRDTCounter crdt(10);
mutex m;
void work(int a){    
    crdt.increment();
    crdt.increment();

    crdt.merge();

    m.lock();
    int value = crdt.getValue();
    cout << "FINAL VALUE THREAD: " << value << endl;
    m.unlock();
}

int main(int argc, char** argv){
    boost::thread_group threads;
    for(int a=0; a < 10; a++){
        threads.create_thread(boost::bind(work, boost::cref(a)));
    } 
    threads.join_all();

    cout << "MAIN: " << crdt.getGlobalValue() << endl;
    //fazer benchmark
    return 0;
}

