#include "kvstore.h"
#include "./vLog/vLog.h"
#include "bloomFilter/bloomFilter.h"
#include "sstable/sstable.h"
#include "utils.h"
#include <string>


KVStore::KVStore(const std::string &dir, const std::string &vlog) : KVStoreAPI(dir, vlog)
{
}

KVStore::~KVStore()
{
}

/**
 * Insert/Update the key-value pair.
 * No return values for simplicity.
 */
void KVStore::put(uint64_t key, const std::string &s)
{
    memtable.put(key, s);
    if (memtable.getSize() >= MEMTABLE_THRESHOLD) {
        sstables.convertMemTableToSSTable(memtable);
        memtable.clean();
        memtable.setSize(0);
    }
}
/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 */
std::string KVStore::get(uint64_t key)
{
    std::string value = memtable.get(key);
    if (value != "" && value != "~DELETED~")
        return value;
    if(value == "~DELETED~")
    {
        return "";
    }

    return sstables.get(key);
}
/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key)
{
    if(get(key) == "")
    {
        return false;
    }
    put(key, "~DELETED~");
    return true;
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset()
{
    memtable.clean();

    sstables.reset();
}

/**
 * Return a list including all the key-value pair between key1 and key2.
 * keys in the list should be in an ascending order.
 * An empty string indicates not found.
 */
void KVStore::scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list)
{
    sstables.scan(key1, key2, list);
    memtable.scan(key1, key2, list);
}

/**
 * This reclaims space from vLog by moving valid value and discarding invalid value.
 * chunk_size is the vlen in byte you should AT LEAST recycle.
 */
void KVStore::gc(uint64_t chunk_size)
{
}

/* void KVStore::convertMemTableToSSTable()
{
    // open the vlog file

    std::fstream vlogFile("./data/vlog", std::ios::in | std::ios::out | std::ios::binary);
    std::list<std::pair<uint64_t, std::string>> list;
    memtable->getList(list);
    uint64_t pairNum = list.size();
    if (pairNum == 0)
        return;
    uint64_t maxKey = list.back().first;
    uint64_t minKey = list.front().first;
    std::vector<bool> bloomFilter;
    bloomFilter.resize(8 * kb);
    std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> keyOffsetTable;
    // store the key value into vlog
    std::list<std::pair<vLogEntry, uint64_t>> entries;
    for (auto &pair : list) {
        vLogEntry entry(pair.first, pair.second);
        entries.push_back(std::make_pair(entry, 0));
    }
    vLog::append(entries, vlogFile);
    vlogFile.flush();

    // store the key and offset into sstable and the offset will be the offset in vlog, which is the second element in
    // the pair
    for (auto &entry : entries) {
        keyOffsetTable.push_back(std::make_tuple(entry.first.key, entry.second, entry.first.vlen));
    }
    // store the sstable
    SSTable sstable(timeId, pairNum, maxKey, minKey, bloomFilter, keyOffsetTable);

    // Create directory if it doesn't exist
    std::string dirPath = "./data/sstable/level0";
    if (!utils::dirExists(dirPath)) {
        if (utils::mkdir(dirPath) != 0) {
            std::cerr << "Failed to create directory: " << dirPath << std::endl;
            return; // Or handle error appropriately
        }
    }

    std::string filename = dirPath + std::to_string(timeId) + ".sst";
    std::fstream sstableFile(filename, std::ios::out | std::ios::binary);
    sstable.output(sstableFile);
    timeId++;
}
 */