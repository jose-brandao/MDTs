#include <iostream>
#include <mutex>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <atomic>

#define LOOP 1000000
#define BENCH_RUNS 10
#define EMPTY_QUEUE -999

using namespace std;

template <typename T=int> 
struct node{
    T payload;
    node * next;
};

template <typename V=int> class HybridQueue {
private:
  node<V>* globalHead=NULL;
  node<V>* globalTail=NULL;
  mutex m;

  boost::thread_specific_ptr< node<V> *> localHead;
  boost::thread_specific_ptr< node<V> *> localTail;

public:

  void init(){
    localHead.reset(new node<V>*); 
    localTail.reset(new node<V>*); 

    *localHead = NULL;
    *localTail = NULL;
  }

  void weakEnqueue(V value){     
    if(*localHead == NULL){
        *localHead= new node<V>;
        (*localHead)->payload = value;
        (*localHead)->next = NULL;

        *localTail=*localHead;
    }

    else{
      node<V> * tempNode = new node<V>;
      tempNode->payload = value;
      tempNode->next = NULL;

      (*localTail)->next = tempNode;
      *localTail = tempNode;
    }
  }

  void strongEnqueue(V value){ 
    m.lock();  
    if(globalHead == NULL){
        globalHead= new node<V>;
        globalHead->payload = value;
        globalHead->next = NULL;

        globalTail=globalHead;
    }

    else{
      node<V> * tempNode = new node<V>;
      tempNode->payload = value;
      tempNode->next = NULL;

      globalTail->next = tempNode;
      globalTail = tempNode;
    }
    m.unlock();
  }

  V strongDequeue(){
    m.lock();
    if(globalHead == NULL){
      m.unlock();

      return V(EMPTY_QUEUE);
    }
    else{
      V res = globalHead->payload;
      node<V> * n = globalHead->next;
      globalHead = n;
      m.unlock();

      return res;
    } 

  }

  void merge(){
    if(*localTail == NULL) return;

    m.lock();
    if(globalHead != NULL) {
      globalTail->next = *localHead;
      globalTail = *localTail;
      m.unlock();
    }
    else {
      globalHead=*localHead;
      globalTail=*localTail;
      m.unlock();
    }

    *localHead=NULL;
    *localTail=NULL;
  }


  //##################FOR DEBUGGING/BENCHMARKING PURPOSES ONLY #############################

  void globalPrint(){
    node<V> * ptr=globalHead;
    while (ptr != NULL)
    {
      cout << ptr->payload << endl;
      ptr=ptr->next;
    }
  }

  int globalLength(){
    int length=0;
    node<V> * ptr=globalHead;
    while (ptr != NULL)
    {
      length++;
      ptr=ptr->next;
    }
    return length;
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

  void reset(){
    cleanLinkedList();
  }

};

HybridQueue<int> mdt;
vector<int> NTHREADS;
int SYNCFREQ [6] = {1,8,64,512,4096,32768};
void work(int syncFreqIndex){
  mdt.init();
  mdt.merge();

  for (int i=0; i < LOOP; i++){
    if(i%SYNCFREQ[syncFreqIndex] == 1){
      mdt.merge();
    }

    if(i%1000==0){ 
      mdt.strongDequeue();
    }

    mdt.weakEnqueue(i);

    if(i%SYNCFREQ[syncFreqIndex] == 0){
      mdt.merge();
    }
  }

  mdt.merge();
}

void benchmarkPerFreq(int syncFreqIndex){
    using namespace std::chrono;
    for(int k = 0; k < NTHREADS.size(); k++){
      std::list<double> times;
      std::list<double> elementCount;
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
        elementCount.push_back(mdt.globalLength());
        throughs.push_back(mdt.globalLength()/ti.count());

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

    for(int i=0; i<6;i++){
        cout << "***************SYNCFREQ: " << SYNCFREQ[i] << " ***************" << endl;
        benchmarkPerFreq(i);
        cout << endl;
    }

    return 0;
}