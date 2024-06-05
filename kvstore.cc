#include "kvstore.h"
#include "./vLog/vLog.h"
#include "bloomFilter/bloomFilter.h"
#include "sstable/sstable.h"
#include "utils.h"
#include <string>

KVStore::KVStore(const std::string &dir, const std::string &vlog) : KVStoreAPI(dir, vlog), sstables(dir, vlog)
{
    sstables.init();
    this->dir = dir;
    this->vlog = vlog;
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
    if (value == "~DELETED~") {
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
    if (get(key) == "") {
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
    // gc

    // since there maybe some gc operation done before, so we need to get the first valid data block in vlogFile
    // use utils::seek_data_block to get the offset of the first data block
    off_t offset = utils::seek_data_block(vlog);
    off_t start_offset = offset;
    std::fstream &vlogFile = sstables.vlogFile;
    // search vLog entry sign 'v', if not found, output an error message
    if (offset == -1) {
        std::cerr << "No valid data block found in vlogFile" << std::endl;
        return;
    }
    char buffer;
    vlogFile.seekg(offset);

    while (vlogFile.read(&buffer, 1)) {
        if (buffer == vLogEntry::magic) {
            // use vLog::isValidEntry to check if the entry is valid
            if (vLog::isValidEntry(offset, vlogFile)) {
                break;
            }
        }
        offset++;
    }

    // if the entry is valid, check if if the entry still count
    // if the entry is still count, put(entry.key, entry.value) in memtable
    // else continue
    int scan_size = 0;
    while (scan_size < chunk_size) {

        vLogEntry entry = vLog::getEntry(offset, vlogFile);
        if (memtable.get(entry.key) != "") {
            offset += entry.vlen + 15;
            scan_size += entry.vlen + 15;
            continue;
        }
        if (sstables.getOffset(entry.key) != offset) {
            offset += entry.vlen + 15;
            scan_size += entry.vlen + 15;
            continue;
        }
        this->put(entry.key, entry.value);
        offset += entry.vlen + 15;
        scan_size += entry.vlen + 15;
    }

    // 将剩余的Memtable数据转换为SSTable
    sstables.convertMemTableToSSTable(memtable);
    memtable.clean();
    memtable.setSize(0);

    // 使用utils::de_alloc_file()释放vlogFile的空间
    utils::de_alloc_file(vlog, start_offset, offset - start_offset);
}
