CC = clang++
DEBUG = -g -v
FLAGS = -std=c++11 -ferror-limit=2
BOOST = -std=c++14 -O3
BOOSTP = -lboost_system -lboost_filesystem -lboost_thread-mt

all: poc threadLocal pCounters crdts gBags orSets queues eVotes

benchAll: benchPoc benchThreadLocal benchPCounters benchCrdts benchGBags benchOrSets benchQueues benchEVotes

finalIHope: syncGBag gBag gBagSpinLock eVotes orSet orSetSpinLock orSetSync orSetv2 orSetv2SpinLock

benchFinalIHope: benchSyncGBag benchGBag benchGBagSpinLock benchEVotes benchOrSet benchOrSetSpinLock benchOrSetSync benchOrSetV2 benchOrSetV2SpinLock

cleanAll: cleanPoc cleanThreadLocal cleanCrdts cleanPCounters cleanGBags cleanOrSets cleanQueues cleanEVotes

#POC####################################################
poc1MIncs:
	$(CC) $(BOOST) src/experiments/poc/poc1MIncs.cc -o poc1MIncs $(BOOSTP)

poc24Threads:
	$(CC) $(BOOST) src/experiments/poc/poc24Threads.cc -o poc24Threads $(BOOSTP)

benchPoc1MIncs:
	./poc1MIncs ${THREADS} > output/experiments/poc1MIncs.txt

benchPoc24Threads:
	./poc24Threads > output/experiments/poc24Threads.txt

poc: poc1MIncs poc24Threads

benchPoc: benchPoc1MIncs benchPoc24Threads

cleanPoc:
	rm poc1MIncs poc24Threads

#THREAD-LOCAL####################################################
manualThreadLocal:
	$(CC) $(FLAGS) src/experiments/threadLocal/manualThreadLocal.cc -o manualThreadLocal

standardThreadLocal: 
	$(CC) $(FLAGS) src/experiments/threadLocal/standardThreadLocal.cc -o standardThreadLocal

boostThreadLocal: src/experiments/threadLocal/boostThreadLocal.cc
	$(CC)  $(BOOST) src/experiments/threadLocal/boostThreadLocal.cc -o boostThreadLocal $(BOOSTP)

benchManualThreadLocal:
	./manualThreadLocal ${THREADS} > output/experiments/manualThreadLocal.txt

benchStandardThreadLocal:
	./standardThreadLocal ${THREADS} > output/experiments/standardThreadLocal.txt

benchBoostThreadLocal:
	./boostThreadLocal ${THREADS} > output/experiments/boostThreadLocal.txt

threadLocal: manualThreadLocal standardThreadLocal boostThreadLocal

benchThreadLocal: benchManualThreadLocal benchStandardThreadLocal benchBoostThreadLocal

cleanThreadLocal:
	rm manualThreadLocal standardThreadLocal boostThreadLocal

#CRDTS################################################################

counterArray: src/experiments/crdts/counterArray.cc
	$(CC)  $(BOOST) src/experiments/crdts/counterArray.cc -o counterArray $(BOOSTP)

benchCounterArray:
	./counterArray ${THREADS} > output/experiments/counterArray.txt

counterMap: src/experiments/crdts/counterMap.cc
	$(CC)  $(BOOST) src/experiments/crdts/counterMap.cc -o counterMap $(BOOSTP)

benchCounterMap:
	./counterMap ${THREADS} > output/experiments/counterMap.txt

crdts: counterArray counterMap

benchCrdts: benchCounterArray benchCounterMap

cleanCrdts:
	rm counterArray counterMap

#P-Counter#########################################################
pCounterAtomic: src/P-Counter/pCounterAtomic.cc 
	$(CC)  $(BOOST) src/P-Counter/pCounterAtomic.cc -o pCounterAtomic $(BOOSTP)

pCounterHybrid: src/P-Counter/pCounterHybrid.cc 
	$(CC)  $(BOOST) src/P-Counter/pCounterHybrid.cc -o pCounterHybrid $(BOOSTP)

pCounterHybridv2: src/P-Counter/pCounterHybridv2.cc 
	$(CC)  $(BOOST) src/P-Counter/pCounterHybridv2.cc -o pCounterHybridv2 $(BOOSTP)

syncCounter: src/P-Counter/syncCounter.cc 
	$(CC)  $(BOOST) src/P-Counter/syncCounter.cc -o syncCounter $(BOOSTP)

benchPCounterAtomic:
	./pCounterAtomic ${THREADS} > output/P-Counter/pCounterAtomic.txt

benchPCounterHybrid:
	./pCounterHybrid ${THREADS} > output/P-Counter/pCounterHybrid.txt

benchPCounterHybridv2:
	./pCounterHybridv2 ${THREADS} > output/P-Counter/pCounterHybridv2.txt

benchSyncCounter:
	./syncCounter ${THREADS} > output/P-Counter/syncCounter.txt

pCounters: pCounterAtomic pCounterHybrid pCounterHybridv2 syncCounter

benchPCounters: benchSyncCounter benchPCounterAtomic benchPCounterHybrid benchPCounterHybridv2

cleanPCounters: 
	rm pCounterAtomic pCounterHybrid pCounterHybridv2 syncCounter

#PN-Counter#########################################################
pnCounter: src/PN-Counter/pnCounter.cc
	$(CC)  $(BOOST) src/PN-Counter/pnCounter.cc -o pnCounter $(BOOSTP)

#G-Bag#########################################################
gBag: src/G-Bag/gBag.cc 
	$(CC)  $(BOOST) src/G-Bag/gBag.cc -o gBag $(BOOSTP)

gBagSpinLock: src/G-Bag/gBagSpinLock.cc 
	$(CC)  $(BOOST) src/G-Bag/gBagSpinLock.cc -o gBagSpinLock $(BOOSTP)

gBagRWLocks: src/G-Bag/gBagRWLocks.cc 
	$(CC)  $(BOOST) src/G-Bag/gBagRWLocks.cc -o gBagRWLocks $(BOOSTP)

syncGBag: src/G-Bag/syncGBag.cc
	$(CC)  $(BOOST) src/G-Bag/syncGBag.cc -o syncGBag $(BOOSTP)

benchGBag:
	./gBag ${THREADS} > output/G-Bag/gBag.txt

benchGBagSpinLock:
	./gBagSpinLock ${THREADS} > output/G-Bag/gBagSpinLock.txt

benchGBagRWLocks:
	./gBagRWLocks ${THREADS} > output/G-Bag/gBagRWLocks.txt

benchSyncGBag:
	./syncGBag ${THREADS} > output/G-Bag/syncGBag.txt

gBags: gBag gBagRWLocks syncGBag gBagSpinLock

benchGBags: benchGBag benchGBagRWLocks benchSyncGBag benchGBagSpinLock

cleanGBags:
	rm gBag gBagRWLocks syncGBag gBagSpinLock

#OR-SET#########################################################
orSet: src/OR-Set/orSet.cc 
	$(CC)  $(BOOST) src/OR-Set/orSet.cc -o orSet $(BOOSTP)

orSetSpinLock: src/OR-Set/orSetSpinLock.cc 
	$(CC)  $(BOOST) src/OR-Set/orSetSpinLock.cc -o orSetSpinLock $(BOOSTP)

orSetv2: src/OR-Set/orSetv2.cc 
	$(CC)  $(BOOST) src/OR-Set/orSetv2.cc -o orSetv2 $(BOOSTP)

orSetv2SpinLock: src/OR-Set/orSetv2SpinLock.cc 
	$(CC)  $(BOOST) src/OR-Set/orSetv2SpinLock.cc -o orSetv2SpinLock $(BOOSTP)

orSetv2RWLocks: src/OR-Set/orSetv2RWLocks.cc 
	$(CC)  $(BOOST) src/OR-Set/orSetv2RWLocks.cc -o orSetv2RWLocks $(BOOSTP)

orSetv2RWLocksDividedWork: src/OR-Set/orSetv2RWLocksDividedWork.cc 
	$(CC)  $(BOOST) src/OR-Set/orSetv2RWLocksDividedWork.cc -o orSetv2RWLocksDividedWork $(BOOSTP)

orSetSync: src/OR-Set/orSetSync.cc 
	$(CC)  $(BOOST) src/OR-Set/orSetSync.cc -o orSetSync $(BOOSTP)

orSetSyncDividedWork: src/OR-Set/orSetSyncDividedWork.cc 
	$(CC)  $(BOOST) src/OR-Set/orSetSyncDividedWork.cc -o orSetSyncDividedWork $(BOOSTP)

benchOrSet:
	./orSet ${THREADS} > output/OR-Set/orSet.txt

benchOrSetSpinLock:
	./orSetSpinLock ${THREADS} > output/OR-Set/orSetSpinLock.txt

benchOrSetV2:
	./orSetv2 ${THREADS} > output/OR-Set/orSetv2.txt

benchOrSetV2SpinLock:
	./orSetv2SpinLock ${THREADS} > output/OR-Set/orSetv2SpinLock.txt

benchOrSetv2RWLocks:
	./orSetv2RWLocks ${THREADS} > output/OR-Set/orSetv2RWLocks.txt

benchOrSetv2RWLocksDividedWork:
	./orSetv2RWLocksDividedWork ${THREADS} > output/OR-Set/orSetv2RWLocksDividedWork.txt

benchOrSetSync:
	./orSetSync ${THREADS} > output/OR-Set/orSetSync.txt

benchOrSetSyncDividedWork:
	./orSetSyncDividedWork ${THREADS} > output/OR-Set/orSetSyncDividedWork.txt	

orSets: orSet orSetSpinLock orSetv2 orSetv2RWLocks orSetv2RWLocksDividedWork orSetSync orSetSyncDividedWork orSetv2SpinLock

benchOrSets: benchOrSet benchOrSetSpinLock benchOrSetV2 benchOrSetv2RWLocks benchOrSetv2RWLocksDividedWork benchOrSetSync benchOrSetSyncDividedWork benchOrSetV2SpinLock 

cleanOrSets:
	rm orSet orSetSpinLock orSetv2 orSetv2RWLocks orSetv2RWLocksDividedWork orSetSync orSetSyncDividedWork orSetv2SpinLock

#QUEUES#########################################################

hybridQueue: src/Queue/hybridQueue.cc 
	$(CC)  $(BOOST) src/Queue/hybridQueue.cc -o hybridQueue $(BOOSTP)

syncQueue: src/Queue/syncQueue.cc 
	$(CC)  $(BOOST) src/Queue/syncQueue.cc -o syncQueue $(BOOSTP)

lockFreeQueue: src/Queue/lockFreeQueue.cc 
	$(CC)  $(BOOST) src/Queue/lockFreeQueue.cc -o lockFreeQueue $(BOOSTP)

benchHybridQueue:
	./hybridQueue ${THREADS} > output/Queue/hybridQueue.txt

benchSyncQueue:
	./syncQueue ${THREADS} > output/Queue/syncQueue.txt

benchLockFreeQueue:
	./lockFreeQueue ${THREADS} > output/Queue/lockFreeQueue.txt

queues: hybridQueue syncQueue lockFreeQueue

benchQueues: benchHybridQueue benchSyncQueue benchLockFreeQueue

cleanQueues:
	rm hybridQueue syncQueue lockFreeQueue

#E-VOTE#########################################################

eVoteHybrid: src/E-Vote/eVoteHybrid.cc 
	$(CC)  $(BOOST) src/E-Vote/eVoteHybrid.cc -o eVoteHybrid $(BOOSTP)

eVoteSync: src/E-Vote/eVoteSync.cc 
	$(CC)  $(BOOST) src/E-Vote/eVoteSync.cc -o eVoteSync $(BOOSTP)

benchEVoteHybrid:
	./eVoteHybrid ${THREADS} > output/E-Vote/eVoteHybrid.txt

benchEVoteSync:
	./eVoteSync ${THREADS} > output/E-Vote/eVoteSync.txt

eVotes: eVoteHybrid eVoteSync

benchEVotes: benchEVoteHybrid benchEVoteSync

cleanEVotes:
	rm eVoteHybrid eVoteSync