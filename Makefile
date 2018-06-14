CC = clang++
DEBUG = -g -v
FLAGS = -std=c++11 -ferror-limit=2
BOOST = -std=c++14 -O3
BOOSTP = -lboost_system -lboost_filesystem -lboost_thread-mt

all: experiments pCounters

benchAll: benchExperiments benchPCounters

clean: cleanExperiments cleanPCounters

#Experiments####################################################
poc1MIncs:
	$(CC) $(BOOST) src/experiments/poc/poc1MIncs.cc -o poc1MIncs $(BOOSTP)

benchPoc1MIncs:
	./poc1MIncs ${THREADS} > output/experiments/poc1MIncs.txt

poc24Threads:
	$(CC) $(BOOST) src/experiments/poc/poc24Threads.cc -o poc24Threads $(BOOSTP)

benchPoc24Threads:
	./poc24Threads > output/experiments/poc24Threads.txt

manualThreadLocal:
	$(CC) $(FLAGS) src/experiments/threadLocal/manualThreadLocal.cc -o manualThreadLocal

standardThreadLocal: 
	$(CC) $(FLAGS) src/experiments/threadLocal/standardThreadLocal.cc -o standardThreadLocal

benchManualThreadLocal:
	./manualThreadLocal ${THREADS} > output/experiments/manualThreadLocal.txt

benchStandardThreadLocal:
	./standardThreadLocal ${THREADS} > output/experiments/standardThreadLocal.txt

experiments: manualThreadLocal standardThreadLocal

benchExperiments: benchManualThreadLocal benchStandardThreadLocal

cleanExperiments:
	rm manualThreadLocal standardThreadLocal

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
