LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++17 -Wall   -Ofast -march=native -mtune=native -fopenmp
#CXXFLAGS = -std=c++17 -Wall   -g
CXX = g++

all: correctness persistence

correctness: kvstore.o correctness.o memTable.o skiplist.o vLog.o sstable.o bloomFilter.o SSTableHandler.o
	$(LINK.o) -o correctness kvstore.o correctness.o memTable.o skiplist.o vLog.o sstable.o bloomFilter.o SSTableHandler.o

persistence: kvstore.o persistence.o memTable.o skiplist.o vLog.o sstable.o bloomFilter.o SSTableHandler.o
	$(LINK.o) -o persistence kvstore.o persistence.o memTable.o skiplist.o vLog.o sstable.o bloomFilter.o SSTableHandler.o

kvstore.o: kvstore.cc
	$(CXX) $(CXXFLAGS)  -c kvstore.cc

correctness.o: correctness.cc
	$(CXX) $(CXXFLAGS)  -c correctness.cc

persistence.o: persistence.cc
	$(CXX) $(CXXFLAGS)  -c persistence.cc

memTable.o: ./MemTable/memTable.cc
	$(CXX) $(CXXFLAGS)  -c ./MemTable/memTable.cc

skiplist.o: ./MemTable/skiplist.cc
	$(CXX) $(CXXFLAGS)  -c ./MemTable/skiplist.cc

vLog.o: ./vLog/vLog.cc
	$(CXX) $(CXXFLAGS)  -c ./vLog/vLog.cc

sstable.o: ./sstable/sstable.cc
	$(CXX) $(CXXFLAGS)  -c ./sstable/sstable.cc

bloomFilter.o: ./bloomFilter/bloomFilter.cc
	$(CXX) $(CXXFLAGS)  -c ./bloomFilter/bloomFilter.cc

SSTableHandler.o: ./SSTableHandler/SSTableHandler.cc
	$(CXX) $(CXXFLAGS)  -c ./SSTableHandler/SSTableHandler.cc

clean:
	-rm -f correctness persistence *.o