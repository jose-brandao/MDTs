#include <iostream>
#include <mutex>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>

#define LOOP 1000000
#define BENCH_RUNS 5

using namespace std;

template <typename T=int> 
struct node{
    T payload;
    node * next;
};

template <typename V=int>
class gBag {
    private:
    //global state
    node<V> * globalHead=NULL;

    //class auxiliars
    mutex m;

    public:
    bool strongLookup(const V value) {
        m.lock();
        node<V>* tempNode=globalHead;
        while(tempNode != NULL){
            if(tempNode->payload == value ) {
             m.unlock();
             return true;   
            }
            tempNode=tempNode->next;
        }
        m.unlock();

        return false; 
    }

    void strongAdd(const V value){
        node<V>* tempNode = new node<V>;
        tempNode->payload = value;

        m.lock();
        tempNode->next = globalHead;
        globalHead = tempNode;
        m.unlock();

    }

   ////////////////////////////BENCHMARKING PURPOSES
    int globalCount() {
        int i=0;
        node<V> * tempNode=globalHead;
        while(tempNode != NULL){
            i++;
            tempNode=tempNode->next;
        }
        return i;
    }

    void cleanLinkedList(){
        node<V>* tempNode = globalHead;
        while( tempNode != NULL ) {
            node<V>* next = tempNode->next;
            delete tempNode;
            tempNode = next;
        }
        globalHead = NULL;
    }

    void printLinkedList(){
        m.lock();
        node<V>* tempNode = globalHead;
        cout << "LIST: ";
        while( tempNode != NULL ) {
            cout << tempNode->payload << "--> ";
            node<V>* next = tempNode->next;
            tempNode = next;
        }
        cout << endl;
        m.unlock();
    }

    void reset(){
        cleanLinkedList();
    }
};

gBag<int> mdt;
vector<int> NTHREADS;
void work(){
  for (int i=0; i < LOOP; i++){
    mdt.strongAdd(i);
    
    if(i%10000 == 0){
      mdt.strongLookup(LOOP/2);
    }
  }
}

void benchmark(){
    using namespace std::chrono;
    for(int k = 0; k < NTHREADS.size(); k++){
      std::list<double> times;
      std::list<double> elementCount;
      std::list<double> throughs;

      for(int i= 0; i< BENCH_RUNS; i++){
        steady_clock::time_point t1 = steady_clock::now();

        boost::thread_group threads;
        for (int a=0; a < NTHREADS[k]; a++){
          threads.create_thread(boost::bind(work));
        }

        threads.join_all();

        steady_clock::time_point t2 = steady_clock::now();

        duration<double> ti = duration_cast<duration<double>>(t2 - t1);

        times.push_back(ti.count());
        elementCount.push_back(mdt.globalCount());
        throughs.push_back(mdt.globalCount()/ti.count());

        mdt.reset();
      }

      double sumTimes = 0;
      for(double t: times){
        sumTimes += t;
      }

      double finalTime = sumTimes/times.size();
      cout << fixed;

      double sumCounters = 0;
      for(double c: elementCount){
        sumCounters += c;
      }
      double finalElemCount = (int)(sumCounters/elementCount.size());


      double sumThroughs = 0;
      for(double th: throughs){
        sumThroughs += th;
      }
      double finalThroughs = sumThroughs/throughs.size();

      cout << (int)finalElemCount << "," << (int)finalThroughs << "," << NTHREADS[k] << endl;
    }
}

int main(int argc, char** argv){
    for(int i=1; i<argc; i++){
      NTHREADS.push_back(atoi(argv[i]));
    }

    benchmark();
    return 0;
}
