#include <iostream>
#include <mutex>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <tuple>

#define LOOP 1000000
#define THREADS 10 

using namespace std;

template <typename T=int> 
struct node{
    T payload;
    node * next;
};

template <typename V=int>

class gset {
private:
  node<V> * ghead=NULL;
  mutex m;

  boost::thread_specific_ptr<node<V> *> lhead;
  boost::thread_specific_ptr<node<V> **> lgraft;
  boost::thread_specific_ptr<bool> last;


public:

  tuple<node<V> **, node<V> ***, bool*> init(){
    lhead.reset(new node<V>*); 
    lgraft.reset(new node<V>**);
    last.reset(new bool(false));
    return make_tuple(lhead.get(),lgraft.get(),last.get());
  }

  void add(const V value, node<V> ** plhead=NULL, node<V> *** plgraft=NULL, bool *plast=NULL ){
    if(plhead == NULL || plgraft == NULL || plast == NULL){ //maybe a cleaner way ?

      if(last.get()==NULL) last.reset(new bool(false));
      if(lhead.get()==NULL) lhead.reset(new node<V>*);
      if(lgraft.get()==NULL) lgraft.reset(new node<V>**);
    
      node<V> * ptr = new node<V>;
      ptr->payload=value;
      ptr->next=*lhead;
      
      if (*last==false){
        *lgraft=& ptr->next;
        *last=true;
      }
      *lhead=ptr;
    }

    else{
      node<V> * ptr = new node<V>;
      ptr->payload=value;
      ptr->next=*plhead;
      
      if (*plast==false){
        *plgraft=& ptr->next;
        *plast=true;
      }
      *plhead=ptr;
    }
  }

  void merge() {   
      m.lock(); // Protect global head accesses 
      **lgraft=ghead; 
      ghead=*lhead;
      m.unlock();
      *last=false;
  }

  int read() const { // for now, report size. Later report a Set
    int i=0;
    node<V> * ptr=ghead;
    while (ptr != NULL)
    {
      i++;
      ptr=ptr->next;
    }
    return i;
  }

};


gset<int> mdt;
void work(){
  //tuple<node<int> **, node<int> ***, bool*> t = mdt.init();
  auto t = mdt.init();
  for (int i=0; i < LOOP; i++){
    mdt.add(i,get<0>(t),get<1>(t),get<2>(t));
  }
  mdt.merge();
}

void slow_work(){
  for (int i=0; i < LOOP; i++){
    mdt.add(i);
  }
  mdt.merge();
}

int main(){
    boost::thread_group threads;
    for (int a=0; a < THREADS; a++) threads.create_thread(&slow_work);
    // Work baby threads, work ...
    threads.join_all();
    cout << "mdt set " << mdt.read() << endl;

}