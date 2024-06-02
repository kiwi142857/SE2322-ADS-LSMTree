#include "./SSTableHandler.h"
#include "../vLog/vLog.h"
#include <algorithm>
#include <map>

void SSTableHandler::convertMemTableToSSTable(MemTable &memTable)
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
    bloomFilter.resize(64 * kb);
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

    // Increment the timeId
    timeId++;

    /* // Add the SSTable to the first level
    sstables.push_back({sstable}); */

    // add the sstable to level 0
    if (sstables.size() == 0) {
        sstables.push_back({sstable});
    } else {
        sstables[0].push_back(sstable);
        // since the max size of level 0 is 2, so if the size of level 0 is 2, then we need to merge the sstable
        if (sstables[0].size() > 2) {
            compactLevel0();
        } else {
            // output the sstable
            if (utils::dirExists("./data/sstable/sstable0") == 0) {
                if (utils::mkdir("./data/sstable/sstable0") != 0) {
                    std::cerr << "Failed to create directory: " << "./data/sstable/sstable0" << std::endl;
                    return;
                }
            }
            std::stringstream ss;
            ss << "./data/sstable/sstable0/sstable" << sstables[0].size();
            std::string filename = ss.str();
            std::fstream sstableFile(filename, std::ios::out | std::ios::binary);
            sstable.output(sstableFile);
            sstableFile.close();
        }
    }
}

// TODO:check bloomFilter & max | min key to find the sstable
std::string SSTableHandler::get(uint64_t key)
{
    int sstablesSize = sstables.size();

    // Iterate through the SSTables
    for (int i = 0; i < sstablesSize; i++) {
        // Iterate through the SSTable objects
        int sstableSize = sstables[i].size()-1;
        for (int j = sstableSize; j >= 0; j--) {
            // Get the offset of the key in the SSTable
            auto offset = sstables[i][j].getOffset(key);
            if (std::get<0>(offset) == key) {

                if(std::get<2>(offset) == 0) {
                    return "";
                }

                // Seek to the offset in the vlog file
                vlogFile.seekg(std::get<1>(offset) + 15);

                // Read the value from the vlog file
                std::string value(std::get<2>(offset), ' ');
                vlogFile.read(&value[0], std::get<2>(offset));

                return value;
            }
        }
    }

    // Return an empty string if the key is not found
    return "";
}

void SSTableHandler::reset()
{
    // Close the vlog file
    vlogFile.close();

    // Remove the vlog file
    utils::rmfile("./data/vlog");

    // Remove the sstable directory
    utils::rmrf("./data/sstable");

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
    for (int i = 0; i < sstables.size(); i++) {
        // Iterate through the SSTable objects
        for (int j = sstables[i].size() -1; j >=0 ; j--) {
            // Get the list of key-value pairs from the SSTable
            sstables[i][j].scanOffset(key1, key2, offsetList);
        }
    }

    // 将offsetList中按照key递增排序
    offsetList.sort([](const std::tuple<uint64_t, uint64_t, uint32_t> &a, const std::tuple<uint64_t, uint64_t, uint32_t> &b) {
        return std::get<0>(a) < std::get<0>(b);
    });

    // Iterate through the offsetList
    for (auto &offset : offsetList) {
        // Seek to the offset in the vlog file
        vlogFile.seekg(std::get<1>(offset) + 15);

        // Read the value from the vlog file
        std::string value(std::get<2>(offset), ' ');
        vlogFile.read(&value[0], std::get<2>(offset));

        // Add the key-value pair to the list
        list.push_back(std::make_pair(std::get<0>(offset), value));
    }
}

void SSTableHandler::compactLevel0()
{
    // 统计Level0层所有SSTable覆盖的键的区间
    uint64_t minKey = std::numeric_limits<uint64_t>::max();
    uint64_t maxKey = 0;
    int j, m = 0;
    for (int i = 0; i < sstables[0].size(); i++) {
        j = sstables[0][i].getSmallestKey();
        if (j < minKey) {
            minKey = j;
        }
        m = sstables[0][i].getLargestKey();
        if (j > maxKey) {
            maxKey = m;
        }
    }

    // 在Level1层中找到与此区间相交的SSTable，并将它们从原来的位置删除
    std::vector<SSTable> level1SSTables;
    // 当Level1 存在时
    if(sstables.size() == 1) {
        sstables.push_back({});
    }
    for (auto it = sstables[1].begin(); it != sstables[1].end();) {
        if (it->getSmallestKey() <= maxKey && it->getLargestKey() >= minKey) {
            level1SSTables.push_back(*it);
            it = sstables[1].erase(it);
        } else {
            ++it;
        }
    }

    // 将Level0层和Level1层的SSTable的引用合并到一个vector中
    std::vector<std::reference_wrapper<SSTable>> allSSTables;
    for (auto &sstable : level1SSTables) {
        allSSTables.push_back(sstable);
    }
    for (auto &sstable : sstables[0]) {
        allSSTables.push_back(sstable);
    }

    // 对所有的SSTable进行排序，时间戳相同时，键值较小的SSTable排在后面
    std::sort(allSSTables.begin(), allSSTables.end(), [](const SSTable &a, const SSTable &b) {
        if (a.getTimeId() == b.getTimeId()) {
            return a.getSmallestKey() > b.getSmallestKey();
        }
        return a.getTimeId() < b.getTimeId();
    });

    // 合并SSTable
    std::map<uint64_t, std::pair<uint64_t, uint32_t>> keyOffsetTable;
    for (auto &sstableRef : allSSTables) {
        SSTable &sstable = sstableRef.get(); // Access the underlying SSTable object
        std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> &offsetList = sstable.getItem();
        for (auto &offset : offsetList) {
            keyOffsetTable[std::get<0>(offset)] = std::make_pair(std::get<1>(offset), std::get<2>(offset));
        }
    }
    //获取最大时间戳
    uint64_t maxTimeId = allSSTables.back().get().getTimeId();

    // 生成新的SSTable,每个SSTable的大小不超过16KB,即item数不超过MEMTABLE_THRESHOLD，且每个SSTable的键值区间不重叠
    std::vector<SSTable> newSSTables;
    std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> newOffsetList;
    uint64_t pairNum = 0;
    for (auto &keyOffset : keyOffsetTable) {
        newOffsetList.push_back(std::make_tuple(keyOffset.first, keyOffset.second.first, keyOffset.second.second));
        pairNum++;
        if (pairNum == MEMTABLE_THRESHOLD) {
            uint64_t maxKey = std::get<0>(newOffsetList.back());
            uint64_t minKey = std::get<0>(newOffsetList.front());
            std::vector<bool> bloomFilter;
            bloomFilter.resize(64 * kb);
            gen_bloom_filter(newOffsetList, bloomFilter);
            SSTable sstable(maxTimeId, pairNum, maxKey, minKey, bloomFilter, newOffsetList);
            newSSTables.push_back(sstable);
            newOffsetList.clear();
            pairNum = 0;
        }
    }

    // handle the last sstable
    if (pairNum > 0) {
        uint64_t maxKey = std::get<0>(newOffsetList.back());
        uint64_t minKey = std::get<0>(newOffsetList.front());
        std::vector<bool> bloomFilter;
        bloomFilter.resize(64 * kb);
        gen_bloom_filter(newOffsetList, bloomFilter);
        SSTable sstable(maxTimeId, pairNum, maxKey, minKey, bloomFilter, newOffsetList);
        newSSTables.push_back(sstable);
    }

    // output the sstable to level 1
    if (utils::dirExists("./data/sstable/sstable1") == 0) {
        if (utils::mkdir("./data/sstable/sstable1") != 0) {
            std::cerr << "Failed to create directory: " << "./data/sstable/sstable1" << std::endl;
            return;
        }
    }
    
    // use utils::scanDir to get the largest sstable file number
    std::vector<std::string> files;
    utils::scanDir("./data/sstable/sstable1", files);
    int fileNum = 0;
    for (auto &file : files) {
        std::string::size_type pos = file.find("sstable");
        if (pos != std::string::npos) {
            int num = std::stoi(file.substr(pos + 7));
            if (num > fileNum) {
                fileNum = num;
            }
        }
    }
    for (auto &sstable : newSSTables) {
        fileNum++;
        std::stringstream ss;
        ss << "./data/sstable/sstable1/sstable" << fileNum;
        std::string filename = ss.str();
        std::fstream sstableFile(filename, std::ios::out | std::ios::binary);
        sstable.output(sstableFile);
        sstableFile.close();
    }

    // 将新生成的SSTable加入Level1层
    for (auto &sstable : newSSTables) {
        sstables[1].push_back(sstable);
    }

    // 将Level1层的SSTable排序，时间戳较大的排在后面，时间戳相同时，键值较小的排在后面
    std::sort(sstables[1].begin(), sstables[1].end(), [](const SSTable &a, const SSTable &b) {
        if (a.getTimeId() == b.getTimeId()) {
            return a.getSmallestKey() > b.getSmallestKey();
        }
        return a.getTimeId() < b.getTimeId();
    });

    sstables[0].clear();
}   