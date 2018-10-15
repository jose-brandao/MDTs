#include <iostream>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <atomic>
#include <iomanip>
#include <array>
#include <mutex>

#define BENCH_RUNS 5
#define DURATION_MILLIS 1*60*1000
#define THRESHOLD_MILLIS 30*1000

using namespace std;

class eVote {
private:
  std::atomic<bool> globalCanVote;
  std::atomic<bool> globalSwitchOn;
  std::atomic<int> * globalCandidates;

  boost::thread_specific_ptr<bool> localCanVote;
  boost::thread_specific_ptr<bool> localSwitchOn; 
  boost::thread_specific_ptr<std::vector<int>> localCandidates;
  boost::thread_specific_ptr<std::vector<int>> localSnapshotCandidates;

  //class auxiliars
  std::atomic<int> mergeTimeMillis; 
  int candidateNumber = 0;


void voteClose(){
    boost::this_thread::sleep(boost::posix_time::milliseconds(DURATION_MILLIS));
    globalCanVote=false;
 }

void activateSwitch(){
    boost::this_thread::sleep(boost::posix_time::milliseconds(THRESHOLD_MILLIS));
    globalSwitchOn=true;
 }

 void timeWriter(){
    while(mergeTimeMillis <= DURATION_MILLIS){
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
      mergeTimeMillis = mergeTimeMillis + 100;
    }
 }

public:

 eVote(int nc){ //In a real case, hours
    candidateNumber = nc;
    globalCandidates = new std::atomic<int>[ candidateNumber ];

    globalCanVote=true;
    globalSwitchOn=false;

    boost::thread tclose(&eVote::voteClose, this);
    boost::thread tswitch(&eVote::activateSwitch, this);
    boost::thread twrite(&eVote::timeWriter, this);

 }

 int getMergeTime(){
  return mergeTimeMillis;
 }

 bool getCanVote(){
  return globalCanVote;
 }

 bool getSwitchOn(){
  return globalSwitchOn;
 }

 bool getLocalCanVote(){
    return *localCanVote;
 }

  void setLocalCanVote(bool val){
    *localCanVote = val;
 }

 bool getLocalSwitchOn(){
    return *localSwitchOn;
 }

 bool getGlobalSwitchOn(){
    return globalSwitchOn;
 }

  void setLocalSwitchOn(bool val){
    *localSwitchOn=val;
 }

 void init(){ //instatiate thread local object parts
    localCandidates.reset(new std::vector<int>(candidateNumber));
    localSnapshotCandidates.reset(new std::vector<int>(candidateNumber));
    
    localCanVote.reset(new bool{true});
    localSwitchOn.reset(new bool{false});
 }

  void weakVote(int id){ //weakUpdate
    if(id < (candidateNumber) ) (*localCandidates)[id]++;
  }

  void strongVote(int id){ //strongUpdate
    if(id < (candidateNumber)) globalCandidates[id]++;
  }

  void vote(int id){
    if(!(*localSwitchOn)){ 
      weakVote(id);
    }
    else strongVote(id);
  }

  void merge(){
    //merging thread local state with global state
    for(int i=0;i<candidateNumber;i++){
      globalCandidates[i]+=(*localCandidates)[i];
    }

    //updating the snapshot
    for(int i=0;i<candidateNumber;i++){
      (*localSnapshotCandidates)[i] = globalCandidates[i];
    }

    //reseting thread local state
    localCandidates.reset(new std::vector<int>(candidateNumber));

    if(globalSwitchOn) *localSwitchOn=true;
    if(!globalCanVote) *localCanVote=false;
  }

  std::vector<int> weakResults(){ //weakRead
    std::vector<int> results(candidateNumber);

    for(int i=0;i<candidateNumber;i++){
      results[i]= (*localSnapshotCandidates)[i]+(*localCandidates)[i];
    }

    return results;

  }

  std::vector<int> strongResults(){ //strongRead
    std::vector<int> results(candidateNumber);

    for(int i=0;i<candidateNumber;i++){
      results[i]= globalCandidates[i]+(*localCandidates)[i];
    }

    return results; 
  }

  ////FOR BENCHMARK/DEBUGGING PURPOSES ONLY

  std::vector<int> globalResults(){
    std::vector<int> results(candidateNumber);

    for(int i=0;i<candidateNumber;i++){
      results[i]= globalCandidates[i];
    }

    return results;   
  }

  void electionPolls(std::vector<int> results){
    long int total=0;

    for(int i=0;i<candidateNumber;i++){
      total+=results[i];
    }
    
    cout << fixed << setprecision(4);
    for(int j=0;j<candidateNumber;j++){
      
      if(results[j] != 0){
       long double up =results[j];
       up*=100;
       long double percentage=up/total;
       cout << "candidate " << j << " : " <<  percentage << "\%"<< endl;
     }

      else cout << "candidate " << j << " : " <<  0 << "\%"<< endl;
    }
  }

  void reset(){
    for(int i=0;i<candidateNumber;i++){
      globalCandidates[i]=0;
    }

    globalCanVote = true;
    globalSwitchOn = false;

    mergeTimeMillis = 0;

    boost::thread tclose(&eVote::voteClose, this);
    boost::thread tswitch(&eVote::activateSwitch, this);
    boost::thread twrite(&eVote::timeWriter, this);
  }

};

eVote mdt(5);
//voteTime = 12s 
//switchTime = 9sec
//mergeFreq = 1 sec
//weakPolls = 3 sec

vector<int> NTHREADS;
int SYNCFREQ [6] = {1,8,64,512,4096,32768};

void workHybrid(int syncFreqIndex){

  mdt.init();

  int auxTime = 0;
  int lastTime = 0;
  bool merge = false;

  srand(time(NULL)); 

  while(mdt.getLocalCanVote()){
    int id = rand()%5;
    mdt.vote(id); //process votes here

    //to ensure only one merge per syncfreq
    auxTime = mdt.getMergeTime();
    if((auxTime%SYNCFREQ[syncFreqIndex]==0) && auxTime!=lastTime ) {
      lastTime = auxTime;
      merge=true;
    }
    
    if(merge){
      mdt.merge();
      merge=false;  
    }
  }

  mdt.merge();
}

void benchmarkPerFreq(int syncFreqIndex){
    //run time is always the same, durationMillis

    using namespace std::chrono;
    for(int k = 0; k < NTHREADS.size(); k++){
      std::list<double> votes;
      std::list<double> throughs;

      for(int i = 0; i < BENCH_RUNS; i++){
        boost::thread_group threads;
        for (int a=0; a < NTHREADS[k]; a++){
          threads.create_thread(boost::bind(workHybrid, boost::cref(syncFreqIndex)));
        }
        // Work baby threads, work ...
        threads.join_all();

        double acVotes=0;
        for(int i : mdt.globalResults()){
          acVotes+=i;
        }
        votes.push_back(acVotes);

        throughs.push_back(acVotes/(DURATION_MILLIS/1000));

        sleep(1); //if we don't wait here, threads lauched on reset will delay some how
        mdt.reset();
      }

      cout << fixed;

      double sumVotes = 0;
      for(double c: votes){
        sumVotes += c;
      }
      double finalVotes = (int)(sumVotes/votes.size());
      //cout << "mdtc counter: " << finalCounter << endl;


      double sumThroughs = 0;
      for(double th: throughs){
        sumThroughs += th;
      }
      double finalThroughs = sumThroughs/throughs.size();
      //cout << "troughput: " << finalThroughs << endl << endl;

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