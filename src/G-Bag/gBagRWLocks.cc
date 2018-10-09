#include <iostream>
#include <mutex>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <tuple>

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

    //thread-local state
    boost::thread_specific_ptr<node<V> *> localHead;
    boost::thread_specific_ptr<node<V> **> localGraft;
    boost::thread_specific_ptr<bool> last;

    //class auxiliars
    mutable boost::shared_mutex m;

    public:
    void init(){
        localHead.reset(new node<V>*); 
        localGraft.reset(new node<V>**);
        last.reset(new bool(false));

        *localHead = NULL;
        *localGraft = NULL;

    }

    bool searchLocally(const V value) {
        node<V>* tempNode=*localHead;

        while(tempNode != NULL){
            if(tempNode->payload == value ) {
                return true;
            }
            tempNode=tempNode->next;
        }
        return false; 
    }

    bool strongLookup(const V value) {
        if(*last && searchLocally(value)) return true;


        boost::shared_lock<boost::shared_mutex> lock(m);
        node<V>* tempNode=globalHead;
        while(tempNode != NULL){
            if(tempNode->payload == value ) {
             return true;   
            }
            tempNode=tempNode->next;
        }

        return false; 
    }

    void weakAdd(const V value){
        node<V>* tempNode = new node<V>;
        tempNode->payload = value;
        tempNode->next = *localHead;
        
        if (*last==false){
            *localGraft = &(tempNode->next);
            *last = true;
        }
        *localHead = tempNode;
    }

    void strongAdd(const V value){
        node<V>* tempNode = new node<V>;
        tempNode->payload = value;

        boost::unique_lock<boost::shared_mutex> lock(m);
        tempNode->next = globalHead;
        globalHead = tempNode;
    }

    void merge() {
        if(*last) { //because we may not have a weak add

          boost::unique_lock<boost::shared_mutex> lock(m); // Protect global head accesses 
          **localGraft=globalHead; //*localGraft is &tail->next(node*), we are saying tail will point to the global head (assign by reference)
          globalHead=*localHead;

          *last=false;
          *localHead=NULL;          
        }
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
        boost::unique_lock<boost::shared_mutex> lock(m);
        node<V>* tempNode = globalHead;
        cout << "LIST: ";
        while( tempNode != NULL ) {
            cout << tempNode->payload << "--> ";
            node<V>* next = tempNode->next;
            tempNode = next;
        }
        cout << endl;
    }

    void printLocalLinkedList(){
        node<V>* tempNode = *(localHead.get());
        cout << "LOCAL LIST: ";
        while( tempNode != NULL ) {
            cout << tempNode->payload << "--> ";
            node<V>* next = tempNode->next;
            tempNode = next;
        }
        cout << endl;
    }

    void reset(){
        cleanLinkedList();
    }
};

gBag<int> mdt;
vector<int> NTHREADS;
int SYNCFREQ [6] = {1,8,64,512,4096,32768};
void work(int syncFreqIndex){
  mdt.init();
  mdt.merge();


  for (int i=0; i < LOOP; i++){
    if(i%SYNCFREQ[syncFreqIndex] == 1){
      mdt.merge();
    }
    mdt.weakAdd(i);
    
    if(i%10000 == 0){
      mdt.strongLookup(LOOP/2);
    }

    if(i%SYNCFREQ[syncFreqIndex] == 0){
      mdt.merge();
    }
  }
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

    for(int i=0; i<6;i++){
        cout << "***************SYNCFREQ: " << SYNCFREQ[i] << " ***************" << endl;
        benchmarkPerFreq(i);
        cout << endl;
    }

    return 0;
}
