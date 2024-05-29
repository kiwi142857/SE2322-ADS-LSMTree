LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++17 -Wall
CXX = g++

all: correctness persistence

correctness: kvstore.o correctness.o memTable.o skiplist.o vLog.o sstable.o bloomFilter.o SSTableHandler.o
	$(LINK.o) -o correctness kvstore.o correctness.o memTable.o skiplist.o vLog.o sstable.o bloomFilter.o SSTableHandler.o

persistence: kvstore.o persistence.o memTable.o skiplist.o vLog.o sstable.o bloomFilter.o SSTableHandler.o
	$(LINK.o) -o persistence kvstore.o persistence.o memTable.o skiplist.o vLog.o sstable.o bloomFilter.o SSTableHandler.o

kvstore.o: kvstore.cc
	$(CXX) $(CXXFLAGS) -g -c kvstore.cc

correctness.o: correctness.cc
	$(CXX) $(CXXFLAGS) -g -c correctness.cc

persistence.o: persistence.cc
	$(CXX) $(CXXFLAGS) -g -c persistence.cc

memTable.o: ./MemTable/memTable.cc
	$(CXX) $(CXXFLAGS) -g -c ./MemTable/memTable.cc

skiplist.o: ./MemTable/skiplist.cc
	$(CXX) $(CXXFLAGS) -g -c ./MemTable/skiplist.cc

vLog.o: ./vLog/vLog.cc
	$(CXX) $(CXXFLAGS) -g -c ./vLog/vLog.cc

sstable.o: ./sstable/sstable.cc
	$(CXX) $(CXXFLAGS) -g -c ./sstable/sstable.cc

bloomFilter.o: ./bloomFilter/bloomFilter.cc
	$(CXX) $(CXXFLAGS) -g -c ./bloomFilter/bloomFilter.cc

SSTableHandler.o: ./SSTableHandler/SSTableHandler.cc
	$(CXX) $(CXXFLAGS) -g -c ./SSTableHandler/SSTableHandler.cc

clean:
	-rm -f correctness persistence *.o