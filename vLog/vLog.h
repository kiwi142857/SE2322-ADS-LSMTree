#include "../utils.h"
#include <iostream>
#include <list>
#include <stdint.h>
#include <string>
#include <tuple>
#include <fstream>
#include <vector>

class vLogEntry
{
  public:
    static char magic;
    uint16_t checksum;
    uint64_t key;
    uint32_t size;
    std::string value;

    vLogEntry(uint64_t key, const std::string &value) : key(key), value(value)
    {
        size = value.size();
        setChecksum();
    };

    vLogEntry(uint64_t key, const std::string &value, uint16_t checksum) : key(key), value(value), checksum(checksum)
    {
        size = value.size();
    };

    vLogEntry(uint64_t key, const std::string &value, uint16_t checksum, uint32_t size) : key(key), value(value), checksum(checksum), size(size){};

    vLogEntry(){};

    void setChecksum()
    {
        std::vector<unsigned char> data;

        // Add key to data
        for (size_t i = 0; i < sizeof(key); ++i) {
            data.push_back((key >> (i * 8)) & 0xFF);
        }

        // Add size to data
        for (size_t i = 0; i < sizeof(size); ++i) {
            data.push_back((size >> (i * 8)) & 0xFF);
        }

        // Add value to data
        for (char c : value) {
            data.push_back(c);
        }

        // Calculate checksum
        checksum = utils::crc16(data);
    }
};

class vLog
{
  public:
    uint64_t headOffset;
    vLog(std::fstream &file);
    ~vLog();
    static void append(std::list<std::pair<vLogEntry, uint64_t>> &entries, std::fstream &file);
    std::string get(uint64_t offset, std::fstream &file);
    /* void gc(uint64_t chunk_size);
    void reset();
    void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list);
    void read();
    void write();
    void recover();
    void load();
    void merge();
    void compact();
    void split();
    void writeLog();
    void readLog();
    void writeMemTable();
    void readMemTable();
    void writeSSTable();
    void readSSTable();
    void convertMemTableToSSTable(); */
};
