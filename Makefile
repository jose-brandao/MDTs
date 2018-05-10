CC = clang++
DEBUG = -g -v
FLAGS = -std=c++11 -ferror-limit=2
BOOST = -std=c++14 
BOOSTP = -lboost_system -lboost_filesystem -lboost_thread-mt

all: agreggationCounterAtomic agreggationCounterAtomicHybrid syncCounter

clean: 
	rm agreggationCounterAtomic agreggationCounterAtomicHybrid syncCounter

#P-Counter
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
###########################################################################################

benchAll: benchSyncCounter benchAgreggationCounterAtomic benchAgreggationCounterAtomicHybrid