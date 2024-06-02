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
