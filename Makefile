CC = clang++
DEBUG = -g -v
FLAGS = -std=c++11 -ferror-limit=2
BOOST = -std=c++14 -O3
BOOSTP = -lboost_system -lboost_filesystem -lboost_thread-mt

all: poc threadLocal pCounters

benchAll: benchPoc benchThreadLocal benchPCounters

clean: cleanExperiments cleanThreadLocal cleanPCounters

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
