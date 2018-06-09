#include <atomic>
#include <iostream>
#include <mutex>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>

#define LOOP 1000000
#define THREADS 10 

using namespace std;

unsigned int c=0;
atomic<unsigned int> v;
mutex m;
boost::thread_specific_ptr<unsigned int> t;


void inc(){
  for (int i=0; i < LOOP; i++){
    c++;
  }
}


void minc(){
  for (int i=0; i < LOOP; i++){
    m.lock();
    c++;
    m.unlock();
  }
}

void ainc(){
  for (int i=0; i < LOOP; i++){
    v++;
  }
}


void tinc(){
  t.reset(new unsigned int(0)); //init thread local var
  unsigned int* p = t.get(); //get a pointer to thread local var
  for (int i=0; i < LOOP; i++){
    ++*p;
  }
  m.lock();
  c=c + *p;
  m.unlock();
}


int main(){
    cout << c << endl;

    boost::thread_group threads;

    for (int a=0; a < THREADS; a++) 
      threads.create_thread(&inc);
    // Work baby threads, work ...
    threads.join_all();
    cout << "standard counter " << c << endl;
    cout << "atomic counter " << v.load() << endl;
}