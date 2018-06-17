CC = clang++
DEBUG = -g -v
FLAGS = -std=c++11 -ferror-limit=2
BOOST = -std=c++14 -O3
BOOSTP = -lboost_system -lboost_filesystem -lboost_thread-mt

all: experiments pCounters

benchAll: benchExperiments benchPCounters

clean: cleanExperiments cleanPCounters

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

#P-Counter#########################################################
agreggationCounterAtomic: src/P-Counter/agreggationCounterAtomic.cc 
	$(CC)  $(BOOST) src/P-Counter/agreggationCounterAtomic.cc -o agreggationCounterAtomic $(BOOSTP)

agreggationCounterAtomicHybrid: src/P-Counter/agreggationCounterAtomicHybrid.cc 
	$(CC)  $(BOOST) src/P-Counter/agreggationCounterAtomicHybrid.cc -o agreggationCounterAtomicHybrid $(BOOSTP)

syncCounter: src/P-Counter/syncCounter.cc 
	$(CC)  $(BOOST) src/P-Counter/syncCounter.cc -o syncCounter $(BOOSTP)

benchSyncCounter:
	./syncCounter ${THREADS} > output/P-Counter/syncCounter.txt

benchAgreggationCounterAtomic:
	./agreggationCounterAtomic ${THREADS} > output/P-Counter/agreggationCounterAtomic.txt

benchAgreggationCounterAtomicHybrid:
	./agreggationCounterAtomicHybrid ${THREADS} > output/P-Counter/agreggationCounterAtomicHybrid.txt

pCounters: agreggationCounterAtomic agreggationCounterAtomicHybrid syncCounter

benchPCounters: benchSyncCounter benchAgreggationCounterAtomic benchAgreggationCounterAtomicHybrid

cleanPCounters: 
	rm agreggationCounterAtomic agreggationCounterAtomicHybrid syncCounter
