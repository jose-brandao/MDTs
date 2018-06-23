#include <iostream>
#include <mutex>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <chrono>
#include <atomic>
#include <iomanip>


#define NTHREADS 16

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

  void weakInc(V tosum={1}){
    *lcount += tosum;
  }

  void weakDec(V tosum={1}){
    *lcount -= tosum;
  }

  V weakValue(){
    V readed = *lgcount + *lcount;
    return readed;
  }

  void strongInc(V tosum={1}){
     gcount.fetch_add(tosum);  
  }

  void strongDec(V tosub={1}){
    gcount.fetch_sub(tosub);  
  }

  void merge(){ //mergeToGlobal
    gcount.fetch_add(*lcount); //merging thread local state with global state
    *lgcount = gcount;  // updating the snapshot
    *lcount = 0; //resetting thread local state
  }

  V globalValue(){
    V globalValue = gcount;
    return globalValue;
  }

};

counter<int> mdt;

void work(){

  mdt.init();

  mdt.weakInc();
  mdt.weakDec();
  mdt.merge();
  mdt.strongInc();
  mdt.strongDec();

}

int main(){

  boost::thread_group threads;
  for (int a=0; a < NTHREADS; a++){
    threads.create_thread(&work);
  }

  // Work baby threads, work ...
  threads.join_all();

  cout << mdt.globalValue() << endl;

  return 0;
}