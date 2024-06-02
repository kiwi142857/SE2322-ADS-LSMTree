#pragma once
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
    uint32_t vlen;
    std::string value;

    vLogEntry(uint64_t key, const std::string &value) : key(key), value(value)
    {
        vlen = value.size();
        setChecksum();
    };

    vLogEntry(uint64_t key, const std::string &value, uint16_t checksum) : key(key), value(value), checksum(checksum)
    {
        vlen = value.size();
    };

    vLogEntry(uint64_t key, const std::string &value, uint16_t checksum, uint32_t vlen) : key(key), value(value), checksum(checksum), vlen(vlen){};

    vLogEntry(){};

    void setChecksum()
    {
        std::vector<unsigned char> data;

        // Add key to data
        for (size_t i = 0; i < sizeof(key); ++i) {
            data.push_back((key >> (i * 8)) & 0xFF);
        }

        // Add vlen to data
        for (size_t i = 0; i < sizeof(vlen); ++i) {
            data.push_back((vlen >> (i * 8)) & 0xFF);
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
    
};
