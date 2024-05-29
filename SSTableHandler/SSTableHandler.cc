#include "./SSTableHandler.h"
#include "../vLog/vLog.h"


void SSTableHandler::convertMemTableToSSTable(MemTable& memTable)
{
    // Get the list of key-value pairs from the MemTable
    std::list<std::pair<uint64_t, std::string>> list;
    memTable.getList(list);

    // generate list of vlog
    std::list<std::pair<uint64_t, std::string>> vlogList;
    uint64_t pairNum = list.size();
    if (pairNum == 0)
        return;
    uint64_t maxKey = list.back().first;
    uint64_t minKey = list.front().first;
    std::vector<bool> bloomFilter;
    bloomFilter.resize(8 * kb);
    gen_bloom_filter(list, bloomFilter);
    std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> keyOffsetTable;
    // store the key value into vlog
    std::list<std::pair<vLogEntry, uint64_t>> entries;
    for (auto &pair : list) {
        vLogEntry entry(pair.first, pair.second);
        entries.push_back(std::make_pair(entry, 0));
    }


    // append the vlog
    vLog::append(entries, vlogFile);
    vlogFile.flush();
    // store the key and offset into sstable and the offset will be the offset in vlog, which is the second element in
    // the pair
    for (auto &entry : entries) {
        keyOffsetTable.push_back(std::make_tuple(entry.first.key, entry.second, entry.first.vlen));
    }
    // store the vlog into sstable
    SSTable sstable(timeId, pairNum, maxKey, minKey, bloomFilter, keyOffsetTable);

    /* // Add the SSTable to the appropriate level
    if (sstables.size() <= maxLevel) {
        sstables.resize(maxLevel + 1);
    }
    sstables[maxLevel].push_back(sstable); */

    // Add the SSTable to the first level
    sstables.push_back({sstable});

    // Increment the timeId
    timeId++;

    // output the sstable
    std::fstream sstableFile("./data/sstable0/sstable", std::ios::in | std::ios::out | std::ios::binary);
    sstable.output(sstableFile);
    
    sstableFile.close();
}

// TODO:check bloomFilter & max | min key to find the sstable
std::string SSTableHandler::get(uint64_t key)
{
    // Iterate through the SSTables
    for (int i = sstables.size() -1 ; i >= 0; i--)
    {
        // Iterate through the SSTable objects
        for (int j = sstables[i].size() -1 ; j >=0; j--)
        {
            // Get the offset of the key in the SSTable
            auto offset = sstables[i][j].getOffset(key);
            if(std::get<0>(offset) == key)
            {   
                
                // Seek to the offset in the vlog file
                vlogFile.seekg(std::get<1>(offset) + 15 );

                // Read the value from the vlog file
                std::string value(std::get<2>(offset), ' ');
                vlogFile.read(&value[0], std::get<2>(offset));

                // Return the value
                if(value == "~DELETED~")
                    return "";
                return value;
            }
        }
    }

    // Return an empty string if the key is not found
    return "";
}

void SSTableHandler::reset(){
    // Close the vlog file
    vlogFile.close();

    // Remove the vlog file
    utils::rmfile("./data/vlog");

    // Remove the sstable directory
    utils::rmdir("./data/sstable");

    // Open the vlog file
    vlogFile.open("./data/vlog", std::ios::in | std::ios::out | std::ios::app | std::ios::binary);

    // delete all the sstable
    sstables.clear();
}

// TODO: avoid unnecessary read | multiThread | binary search
void SSTableHandler::scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list)
{   
    std::list<std::tuple<uint64_t, uint64_t, uint32_t>> offsetList;
    // Iterate through the SSTables
    for (int i = 0; i < sstables.size(); i++)
    {
        // Iterate through the SSTable objects
        for (int j = 0; j < sstables[i].size(); j++)
        {
            // Get the list of key-value pairs from the SSTable
            sstables[i][j].scanOffset(key1, key2, offsetList);
        }
    }

    // Iterate through the offsetList
    for (auto &offset : offsetList)
    {
        // Seek to the offset in the vlog file
        vlogFile.seekg(std::get<1>(offset) + 15);

        // Read the value from the vlog file
        std::string value(std::get<2>(offset), ' ');
        vlogFile.read(&value[0], std::get<2>(offset));

        // Add the key-value pair to the list
        list.push_back(std::make_pair(std::get<0>(offset), value));
    }
}