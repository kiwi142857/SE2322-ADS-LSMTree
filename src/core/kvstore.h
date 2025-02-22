#pragma once

#include "kvstore_api.h"
#include "../memtable/memTable.h"
#include "../sstable/sstable.h"
#include "../sstable/SSTableHandler.h"

class KVStore : public KVStoreAPI
{
	// You can add your implementation here
private:
	MemTable memtable;
	// std::string dir;
	// std::string vlog;
	SSTableHandler sstables;
	std::string dir;
	std::string vlog;

public:
	KVStore(const std::string &dir, const std::string &vlog);

	~KVStore();

	void put(uint64_t key, const std::string &s) override;

	std::string get(uint64_t key) override;

	bool del(uint64_t key) override;

	void reset() override;

	void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list) override;

	void gc(uint64_t chunk_size) override;

	// void convertMemTableToSSTable();

	void printVLog(int offset){
		sstables.printVLog(offset);
	}
};
