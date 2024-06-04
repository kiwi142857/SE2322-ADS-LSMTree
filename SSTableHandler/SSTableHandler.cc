#include "./SSTableHandler.h"
#include "../vLog/vLog.h"
#include <algorithm>
#include <map>
#include <bitset>

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
            // check if the size of level 1 is more than 4
            if (sstables[1].size() > 4) {
                compact(1);
            }
        } else {
            // output the sstable
            if (utils::dirExists("./data/sstable/level-0") == 0) {
                if (utils::mkdir("./data/sstable/level-0") != 0) {
                    std::cerr << "Failed to create directory: " << "./data/sstable/level-0" << std::endl;
                    return;
                }
            }
            std::stringstream ss;
            ss << "./data/sstable/level-0/sstable" << sstables[0].size() << ".sst";
            std::string filename = ss.str();
            std::fstream sstableFile(filename, std::ios::out | std::ios::binary);
            sstable.setFilename(filename);
            sstable.output(sstableFile);
            sstableFile.close();
        }
    }
}

std::string SSTableHandler::get(uint64_t key)
{
    int sstablesSize = sstables.size();

    // Iterate through the SSTables
    for (int i = 0; i < sstablesSize; i++) {
        // Iterate through the SSTable objects
        int sstableSize = sstables[i].size() - 1;
        for (int j = sstableSize; j >= 0; j--) {
            // check if the key is between the max and min key of the sstable
            if (key >= sstables[i][j].getSmallestKey() && key <= sstables[i][j].getLargestKey()) {
                // check the bloom filter
                if (!sstables[i][j].checkBloomFilter(key)) {
                    continue;
                }
            } else {
                continue;
            }

            // Get the offset of the key in the SSTable
            auto offset = sstables[i][j].getOffset(key);
            if (std::get<0>(offset) == key) {

                if (std::get<2>(offset) == 0) {
                    return "";
                }

                // Seek to the offset in the vlog file
                vlogFile.seekg(std::get<1>(offset) + 15);

                // Read the value from the vlog file
                std::string value(std::get<2>(offset), ' ');
                vlogFile.read(&value[0], std::get<2>(offset));

                // for debug
                if (key == 1 && value == "  ") {
                    // print the 0 ~ 2*get<2>(offset) bytes of vlog
                    vlogFile.seekg(0);
                    int size = 16 * std::get<2>(offset);
                    char *buf = new char[size + 1]; // Add 1 for the null terminator
                    vlogFile.read(buf, size);
                    buf[size] = '\0'; // Add the null terminator
                    std::cout << "key: " << key << " value: " << value << " buf: " << buf << std::endl;
                    delete[] buf; // Don't forget to delete buf when you're done with it
                }

                return value;
            }
        }
    }

    // Return an empty string if the key is not found
    return "";
}

uint64_t SSTableHandler::getOffset(uint64_t key)
{
    int sstablesSize = sstables.size();

    // Iterate through the SSTables
    for (int i = 0; i < sstablesSize; i++) {
        // Iterate through the SSTable objects
        int sstableSize = sstables[i].size() - 1;
        for (int j = sstableSize; j >= 0; j--) {
            // check if the key is between the max and min key of the sstable
            if (key >= sstables[i][j].getSmallestKey() && key <= sstables[i][j].getLargestKey()) {
                // check the bloom filter
                if (!sstables[i][j].checkBloomFilter(key)) {
                    continue;
                }
            } else {
                continue;
            }

            // Get the offset of the key in the SSTable
            auto offset = sstables[i][j].getOffset(key);
            if (std::get<0>(offset) == key) {
                return std::get<1>(offset);
            }
        }
    }

    // Return an empty string if the key is not found
    return std::numeric_limits<uint64_t>::max();
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

    // 相应删除sstable0文件
    utils::rmfile("./data/sstable/level-0/sstable1");
    utils::rmfile("./data/sstable/level-0/sstable2");
}

// TODO: avoid unnecessary read | multiThread | binary search
void SSTableHandler::scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list)
{
    std::list<std::tuple<uint64_t, uint64_t, uint32_t>> offsetList;
    // Iterate through the SSTables
    for (int i = 0; i < sstables.size(); i++) {
        // Iterate through the SSTable objects
        for (int j = sstables[i].size() - 1; j >= 0; j--) {
            // Get the list of key-value pairs from the SSTable

            // 利用最大键值和最小键值进行判断，如果key2小于最小键值或者key1大于最大键值，则不可能存在交集
            if (key2 < sstables[i][j].getSmallestKey() || key1 > sstables[i][j].getLargestKey()) {
                continue;
            }
            sstables[i][j].scanOffset(key1, key2, offsetList);
        }
    }

    // 将offsetList中按照key递增排序
    offsetList.sort([](const std::tuple<uint64_t, uint64_t, uint32_t> &a,
                       const std::tuple<uint64_t, uint64_t, uint32_t> &b) { return std::get<0>(a) < std::get<0>(b); });

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
    bool isLastLevel = 0;
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
        if (m > maxKey) {
            maxKey = m;
        }
    }

    // 在Level1层中找到与此区间相交的SSTable，并将它们从原来的位置删除
    std::vector<SSTable> level1SSTables;
    // 用于存储需要删除的文件名
    std::vector<std::string> filesToDelete;

    // 当Level1不存在时
    if (sstables.size() == 1) {
        isLastLevel = 1;
        sstables.push_back({});
    }
    for (auto it = sstables[1].begin(); it != sstables[1].end();) {
        if (it->getSmallestKey() <= maxKey && it->getLargestKey() >= minKey) {
            filesToDelete.push_back(it->getFilename());
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
            return a.getSmallestKey() < b.getSmallestKey();
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
    // 获取最大时间戳
    uint64_t maxTimeId = allSSTables.back().get().getTimeId();

    // 生成新的SSTable,每个SSTable的大小不超过16KB,即item数不超过MEMTABLE_THRESHOLD，且每个SSTable的键值区间不重叠
    std::vector<SSTable> newSSTables;
    std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> newOffsetList;
    uint64_t pairNum = 0;
    for (auto &keyOffset : keyOffsetTable) {
        // 如果是最后一层，需要将所有vlen为0的值丢弃
        if (isLastLevel && keyOffset.second.second == 0) {
            continue;
        }
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
    if (utils::dirExists("./data/sstable/level-1") == 0) {
        if (utils::mkdir("./data/sstable/level-1") != 0) {
            std::cerr << "Failed to create directory: " << "./data/sstable/level-1" << std::endl;
            return;
        }
    }

    // use utils::scanDir to get the largest sstable file number
    std::vector<std::string> files;
    utils::scanDir("./data/sstable/level-1", files);
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
        ss << "./data/sstable/level-1/sstable" << fileNum << ".sst";
        std::string filename = ss.str();
        std::fstream sstableFile(filename, std::ios::out | std::ios::binary);
        sstable.setFilename(filename);
        sstable.output(sstableFile);
        sstableFile.close();
    }

    // 将新生成的SSTable加入Level1层
    for (auto &sstable : newSSTables) {
        sstables[1].push_back(sstable);
    }

    // 将Level1层的SSTable排序，时间戳较大的排在后面，时间戳相同时，键值较大的排在后面
    std::sort(sstables[1].begin(), sstables[1].end(), [](const SSTable &a, const SSTable &b) {
        if (a.getTimeId() == b.getTimeId()) {
            return a.getSmallestKey() < b.getSmallestKey();
        }
        return a.getTimeId() < b.getTimeId();
    });

    sstables[0].clear();

    // 删除Level1层的文件
    for (auto &file : filesToDelete) {
        if (utils::rmfile(file) != 0) {
            // std::cerr << "Failed to delete file: " << file << std::endl;
        }
    }

    // 删除Level0层的文件
    utils::rmfile("./data/sstable/level-0/sstable1");
    utils::rmfile("./data/sstable/level-0/sstable2");
}

// compact
void SSTableHandler::compact(int level)
{   
    bool isLastLevel = 0;
    // get the sstable from the level
    // just to make sure the size of this level no more than 2^(level+1)
    // 选择时间戳最小的若干个文件，如果时间戳相等选择键值最小的文件
    // 由于上一层进行合并时，已经将本层排序，所以只需要选择前面的文件即可
    std::vector<SSTable> sstablesToCompact;
    int levelSize = sstables[level].size();
    int legalSize = 1 << (level + 1);
    int compactSize = levelSize - legalSize;

    std::vector<std::string> filesToDelete;

    

    for (int i = 0; i < compactSize; i++) {
        filesToDelete.push_back(sstables[level][i].getFilename());
        sstablesToCompact.push_back(sstables[level][i]);
    }

    // 统计本层所有SSTable覆盖的键的区间
    uint64_t minKey = std::numeric_limits<uint64_t>::max();
    uint64_t maxKey = 0;
    int j, m = 0;
    for (int i = 0; i < sstablesToCompact.size(); i++) {
        j = sstablesToCompact[i].getSmallestKey();
        if (j < minKey) {
            minKey = j;
        }
        m = sstablesToCompact[i].getLargestKey();
        if (m > maxKey) {
            maxKey = m;
        }
    }

    // 在下一层中找到与此区间相交的SSTable，并将它们从原来的位置删除
    std::vector<SSTable> nextLevelSSTables;
    // 当下一层不存在时
    if (sstables.size() == level + 1) {
        isLastLevel = 1;
        sstables.push_back({});
    }

    for (auto it = sstables[level + 1].begin(); it != sstables[level + 1].end();) {
        if (it->getSmallestKey() <= maxKey && it->getLargestKey() >= minKey) {
            filesToDelete.push_back(it->getFilename());
            nextLevelSSTables.push_back(*it);
            it = sstables[level + 1].erase(it);
        } else {
            ++it;
        }
    }

    // 对nextLevelSSTables进行排序
    std::sort(nextLevelSSTables.begin(), nextLevelSSTables.end(), [](const SSTable &a, const SSTable &b) {
        if (a.getTimeId() == b.getTimeId()) {
            return a.getSmallestKey() < b.getSmallestKey();
        }
        return a.getTimeId() < b.getTimeId();
    });

    // 对sstablesToCompact进行排序
    std::sort(sstablesToCompact.begin(), sstablesToCompact.end(), [](const SSTable &a, const SSTable &b) {
        if (a.getTimeId() == b.getTimeId()) {
            return a.getSmallestKey() < b.getSmallestKey();
        } 
        return a.getTimeId() < b.getTimeId();
    });

    // 将本层和下一层的SSTable的引用合并到一个vector中
    std::vector<std::reference_wrapper<SSTable>> allSSTables;
    for (auto &sstable : nextLevelSSTables) {
        allSSTables.push_back(sstable);
    }
    for (auto &sstable : sstablesToCompact) {
        allSSTables.push_back(sstable);
    }
    
    // 获取最大时间戳
    uint64_t maxTimeId = allSSTables.back().get().getTimeId();

    // 合并SSTable
    std::map<uint64_t, std::pair<uint64_t, uint32_t>> keyOffsetTable;
    for (auto &sstableRef : allSSTables) {
        SSTable &sstable = sstableRef.get(); // Access the underlying SSTable object
        std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> &offsetList = sstable.getItem();
        for (auto &offset : offsetList) {
            
            keyOffsetTable[std::get<0>(offset)] = std::make_pair(std::get<1>(offset), std::get<2>(offset));
        }
    }

    // 生成新的SSTable,每个SSTable的大小不超过16KB,即item数不超过MEMTABLE_THRESHOLD，且每个SSTable的键值区间不重叠
    std::vector<SSTable> newSSTables;
    std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> newOffsetList;
    uint64_t pairNum = 0;

    for (auto &keyOffset : keyOffsetTable) {
        // 如果是最后一层，需要将所有vlen为0的值丢弃
        if (isLastLevel && keyOffset.second.second == 0) {
            continue;
        }
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

    // output the sstable to next level
    if (utils::dirExists("./data/sstable/level-" + std::to_string(level + 1)) == 0) {
        if (utils::mkdir("./data/sstable/level-" + std::to_string(level + 1)) != 0) {
            std::cerr << "Failed to create directory: " << "./data/sstable/level-" + std::to_string(level + 1)
                      << std::endl;
            return;
        }
    }

    // use utils::scanDir to get the largest sstable file number
    std::vector<std::string> files;
    utils::scanDir("./data/sstable/level-" + std::to_string(level + 1), files);
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
        ss << "./data/sstable/level-" << level + 1 << "/sstable" << fileNum << ".sst";
        std::string filename = ss.str();
        std::fstream sstableFile(filename, std::ios::out | std::ios::binary);
        sstable.setFilename(filename);
        sstable.output(sstableFile);
        sstableFile.close();
    }

    // 将新生成的SSTable加入下一层
    for (auto &sstable : newSSTables) {
        sstables[level + 1].push_back(sstable);
    }

    // 将下一层的SSTable排序，时间戳较大的排在后面，时间戳相同时，键值较大的排在后面
    std::sort(sstables[level + 1].begin(), sstables[level + 1].end(), [](const SSTable &a, const SSTable &b) {
        if (a.getTimeId() == b.getTimeId()) {
            return a.getSmallestKey() < b.getSmallestKey();
        }
        return a.getTimeId() < b.getTimeId();
    });

    for (int i = 0; i < compactSize; i++) {
        sstables[level].erase(sstables[level].begin());
    }
    
    // 删除需要删除的文件
    for (auto &file : filesToDelete) {
        if (utils::rmfile(file) != 0) {
            // std::cerr << "Failed to delete file: " << file << std::endl;
        }
    }


    // 判断是否需要递归合并下一层
    if (sstables[level + 1].size() > (1 << (level + 2))) {
        compact(level + 1);
    }

    
}

void SSTableHandler::init(){

    // 文件名称示例 ./data/sstable/level-0/sstable1; ./data/sstable/level-0/sstable10
    // get the sstable from the disk
    // 先查看有没有./data/sstable文件夹
    if (utils::dirExists("./data/sstable") == 0) {
        return;
    }
    // 查看sstable下有没有文件夹,如果有，记录下文件夹的名字
    std::vector<std::string> files;
    utils::scanDir("./data/sstable", files);
    int i = 0;
    for (auto &file : files) {
        // 读取这个文件夹中所有的文件
        // 找到最后一个'sstable'的位置
        size_t pos = file.find_last_of("level-");
        if (pos != std::string::npos) {
            // 获取'sstable'后面的数字
            std::string numStr = file.substr(pos +1);
            // 将数字字符串转换为整数
            i = std::stoi(numStr);
        } else {
            printf("Error: sstable file name error\n");
        }
        if (utils::dirExists("./data/sstable/" + file) == 1) {
            // 对于目录下所有文件，调用sstable.input，生成新的sstable对象，并存放在sstables对应的位置
            std::vector<std::string> sstableFiles;
            utils::scanDir("./data/sstable/" + file, sstableFiles);
            for (auto &sstableFile : sstableFiles) {
                std::string filePath = std::string("./data/sstable/") + std::string(file) + "/" + std::string(sstableFile);
                std::fstream file(filePath, std::ios::in | std::ios::binary);
                input(filePath,i, sstableFile);
            }
            // 对于本层的sstable进行排序，时间戳较大的排在后面，时间戳相同时，键值较大的排在后面
            std::sort(sstables[i].begin(), sstables[i].end(), [](const SSTable &a, const SSTable &b) {
                if (a.getTimeId() == b.getTimeId()) {
                    return a.getSmallestKey() < b.getSmallestKey();
                }
                return a.getTimeId() < b.getTimeId();
            });

            i++;
        }
    }

    // remove the file in ./data/sstable
    // TODO: do not do like this, you should choose an elegant way
    // Remove the sstable directory
    utils::rmrf("./data/sstable");
}

void SSTableHandler::input(std::string filename,int level, std::string fileSubName){
    std:: ifstream file(filename);
    // 读取文件中的内容
    
    // 开头是时间戳，8 bytes
    uint64_t timeId;
    file.read((char *)&timeId, sizeof(timeId));
    // update 时间戳
    if (timeId > this->timeId) {
        this->timeId = timeId;
    }
    // 读取pairNum
    uint64_t pairNum;
    file.read((char *)&pairNum, sizeof(pairNum));
    // 读取maxKey
    uint64_t maxKey;
    file.read((char *)&maxKey, sizeof(maxKey));
    // 读取minKey
    uint64_t minKey;
    file.read((char *)&minKey, sizeof(minKey));
    // 读取bloomFilter
    std::vector<bool> bloomFilter(64 * kb); // 64kb
    for (int i = 0; i < bloomFilter.size(); i += 8) {
        char byte;
        file.read(&byte, sizeof(byte));
        for (int j = 0; j < 8; j++) {
            if (i + j < bloomFilter.size()) {
                bloomFilter[i + j] = (byte >> j) & 1;
            }
        }
    }

    // 读取keyOffsetTable
    std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> keyOffsetTable;
    for (int i = 0; i < pairNum; i++) {
        uint64_t key;
        uint64_t offset;
        uint32_t vlen;
        file.read((char *)&key, sizeof(key));
        file.read((char *)&offset, sizeof(offset));
        file.read((char *)&vlen, sizeof(vlen));
        keyOffsetTable.push_back(std::make_tuple(key, offset, vlen));
    }
    SSTable sstable(timeId, pairNum, maxKey, minKey, bloomFilter, keyOffsetTable);
    sstable.setFilename(fileSubName);
    while(sstables.size() <= level){
        sstables.push_back({});
    }
    sstables[level].push_back(sstable);
    file.close();

    
}