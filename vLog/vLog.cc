#include "vLog.h"
#include <iostream>
#include <list>
#include <stdint.h>
#include <string>
#include <vector>
#include <fstream>

char vLogEntry::magic = 'v';

vLog::vLog(std::fstream &file)
{
    headOffset = file.tellg();
}

vLog::~vLog()
{
}

void vLog::append(std::list<std::pair<vLogEntry, uint64_t>> &entries, std::fstream &file){
    for (auto &entry : entries) {
        file.seekp(0, std::ios::end); 
        entry.second = file.tellp(); 
        file.write(&vLogEntry::magic, 1);
        file.write((char *)&entry.first.checksum, sizeof(entry.first.checksum));
        file.write((char *)&entry.first.key, sizeof(entry.first.key));
        file.write((char *)&entry.first.size, sizeof(entry.first.size));
        file.write(&entry.first.value[0], entry.first.size);
    }
}

std::string vLog::get(uint64_t offset, std::fstream &file)
{
    file.seekg(offset);
    char magic;
    file.read(&magic, 1);
    if (magic != vLogEntry::magic) {
        return "";
    }

    vLogEntry entry;
    file.read((char *)&entry.checksum, sizeof(entry.checksum));
    file.read((char *)&entry.key, sizeof(entry.key));
    file.read((char *)&entry.size, sizeof(entry.size));
    entry.value.resize(entry.size);
    file.read(&entry.value[0], entry.size);

    std::vector<unsigned char> data;
    for (size_t i = 0; i < sizeof(entry.key); ++i) {
        data.push_back((entry.key >> (i * 8)) & 0xFF);
    }
    for (size_t i = 0; i < sizeof(entry.size); ++i) {
        data.push_back((entry.size >> (i * 8)) & 0xFF);
    }
    for (char c : entry.value) {
        data.push_back(c);
    }

    if (entry.checksum != utils::crc16(data)) {
        return "";
    }

    return entry.value;
}