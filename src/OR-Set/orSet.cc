#include <iostream>
#include <mutex>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <list>
#include <tuple>

#define LOOP 1000000
#define BENCH_RUNS 5

using namespace std;
using namespace boost;

template <typename V=int> struct node{
  V val;
  list< std::tuple<int,int,int> > vinfo; //first,last,remove
  node * left;
  node * right; 
};

template <typename T=int> class orSet{
private:

  //global state
  int gvid;
  node<T> *root;

  //thread-local state
  boost::thread_specific_ptr<int> lvid;
  boost::thread_specific_ptr<list<T>> localAdd;
  boost::thread_specific_ptr<list<T>> localRemove;

  //class auxiliars
  boost::mutex m;

void insert(T val, node<T> *leaf, T vid){
  if(val < leaf->val){
    if(leaf->left != NULL) insert(val, leaf->left,vid);

    else {  
      leaf->left = new node<T>;
      leaf->left->val = val;
      leaf->left->left = NULL;
      leaf->left->right = NULL;
      
      (leaf->left->vinfo).push_front(make_tuple(vid,vid,0));

    }

  }

  else{
    if(leaf->right != NULL) insert(val, leaf->right,vid);

    else{
      leaf->right=new node<T>;
      leaf->right->val=val;
      leaf->right->left=NULL;
      leaf->right->right=NULL;
      
      (leaf->right->vinfo).push_front(make_tuple(vid,vid,0));
    }
  }
}

node<T>* searchNode(node<T> * leaf, T val){
  if(leaf!=NULL){
    if(val == leaf->val) return leaf;
    if(val < leaf->val) return searchNode(leaf->left, val);
    else return searchNode(leaf->right, val);
  }
  else return NULL;
}

node<T>* findParentNode(node<T> * leaf, T val, node<T> * parent){
  if(leaf!=NULL){
    if(val == leaf->val) return parent;
    if(val < leaf->val) return findParentNode(leaf->left, val, leaf);
    else return findParentNode(leaf->right, val, leaf);
  }
  else return parent;
}

void removeFromLocalAdded(T val){
  typename std::list<T>::iterator itr = (*localAdd).begin();
  while (itr != (*localAdd).end()){ 
    if((*itr) == val) {
      itr = (*localAdd).erase(itr);
    }
    else ++itr;
  }
}

void addHelper(T val, int vid){
    //search for a node, if doesn't exist insert it. Otherwise just update
    node<T>* n = searchNode(root, val);
    if(n==NULL){
        //node to insert doesn't exist so we just need to insert it
        if (root != NULL) insert(val, root, vid);
        else insertAtRoot(val, vid);
    }
    else{     
        //node to insert already exists, just update vinfo
        if(get<2>((n->vinfo).front()) > 0){
            (n->vinfo).push_front(make_tuple(vid,vid,0)); //if removed insert new vinfo at head
        }
        else{
            get<1>((n->vinfo).front())= vid; //if not update last of the head
        }
    }
}

void removeHelper(T val, int vid){
    //just need to check if node is != NULL and if removed is == 0
    node<T>* n = searchNode(root, val); 
    if(n != NULL){
      if(get<2>((n->vinfo).front()) == 0){
        get<2>((n->vinfo).front()) = vid;
      }
    }
}

void readSet(node<T> * leaf){
    if(leaf == NULL) return;
    if(leaf!=NULL) {
      m.lock();
      cout << leaf->val << "\n";
      m.unlock();
    }
  
    readSet(leaf->left);
    readSet(leaf->right);
  }

  int globalCount(node<T>* node){
    if(node == NULL) return 0;
    else return globalCount(node->left) + globalCount(node->right) + 1;
  }

  void deleteTree(node<T>* leaf) {
    if(leaf==NULL) return;

    deleteTree(leaf->left);
    deleteTree(leaf->right);

    delete leaf;
  }

public:
  orSet(){
    root=NULL;
    gvid = 0;
  }

  void init(){
    localAdd.reset(new list<T>);
    localRemove.reset(new list<T>);
    lvid.reset(new int(gvid));
  }

  bool weakLookup(T val){
    for(T lval: *localAdd){  //checking on local added
      if(lval == val) return true;
    }

    m.lock();
    node<T>* n = searchNode(root, val);
    if(n == NULL) {
        m.unlock();
        return false;
    }
    list<std::tuple<int,int,int>> vinfo (n->vinfo);
    m.unlock();

    for(auto v : vinfo){
      if(get<2>(v) == 0 && get<0>(v) <= *lvid) return true; 
      else if(get<0>(v) <= *lvid && *lvid < get<2>(v)) return true; //node could be valid in local version
    }

    return false;
  }

  bool strongLookup(T val){
    for(T lval: *localAdd){     //checking on local added
      if(lval == val) return true;
    }

    m.lock();
    node<T>* n = searchNode(root, val);
    if(n == NULL) {
        m.unlock();
        return false;
    }
    list<std::tuple<int,int,int>> vinfo (n->vinfo);
    m.unlock();
    
    for(auto v : vinfo){
      if(get<2>(v) == 0) return true; 
    }

    return false;
  }

  void weakAdd(T val){
    (*localAdd).push_front(val);
  }

  void strongAdd(T val){
      m.lock();
      addHelper(val, gvid);
      m.unlock();
  }

  void weakRemove(T val){
    (*localRemove).push_front(val);
    removeFromLocalAdded(val);
  }

    void strongRemove(T val){
      m.lock();
      removeHelper(val, gvid);
      m.unlock();
  }
  
  void merge(){
    m.lock();
    int newVid = gvid+1;

    //merge weak adds
    for(T val: *localAdd){
        addHelper(val, newVid);
    }
    (*localAdd).clear();  

    //merge weak removes
    for(T val: *localRemove){  
        removeHelper(val, newVid);
    }
    (*localRemove).clear();

    gvid = newVid;
    *lvid = gvid;
    m.unlock();
  }


////////////////////////////FOR DEBUGGING/BENCHMARK PURPOSES
  void readSetMdt(){
    readSet(root);
  }

  int globalCountMdt(){
    return globalCount(root);
  }

  void deleteTree(){
    deleteTree(root);
  }

  void insertAtRoot(T val, T vid){
    if(root!=NULL) insert(val, root, vid);
    else{
      root=new node<T>;
      root->val=val;
      root->left=NULL;
      root->right=NULL;

      (root->vinfo).push_front(make_tuple(vid,vid,0));
    }
  }

  void reset(){
    root=NULL;
    gvid = 0;
    deleteTree();
  }
};

orSet<int> mdt;
vector<int> NTHREADS;
int SYNCFREQ [6] = {1,8,64,512,4096,32768};
void work(int syncFreqIndex){
  mdt.init();
  mdt.merge();

  for (int i=0; i < LOOP; i++){
    if(i%SYNCFREQ[syncFreqIndex] == 1){
      mdt.merge();
    }

    if(i%10==2){ 
      mdt.weakRemove(i-1);
    }

    mdt.weakAdd(i);
  
    if(i%10000 == 0){
      mdt.strongLookup(LOOP/2);
    }

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
        
        //ensure balancing on the tree
        for(int i=0; i < LOOP; i+=500){
            int right = LOOP/2 + i;
            int left = LOOP/2 - i;
            
            if(right < LOOP) {
               mdt.insertAtRoot(right, 0);
            }
            if(left > 0 && left!=LOOP/2) {
              mdt.insertAtRoot(left, 0);
            }
        }
        
        steady_clock::time_point t1 = steady_clock::now();

        boost::thread_group threads;
        for (int a=0; a < NTHREADS[k]; a++){
          threads.create_thread(boost::bind(work, boost::cref(syncFreqIndex)));
        }

        threads.join_all();

        steady_clock::time_point t2 = steady_clock::now();

        duration<double> ti = duration_cast<duration<double>>(t2 - t1);

        times.push_back(ti.count());

        int globalCount = mdt.globalCountMdt() * NTHREADS[k];

        elementCount.push_back(globalCount);
        throughs.push_back(globalCount/ti.count());

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