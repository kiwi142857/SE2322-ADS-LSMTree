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
    std::stringstream ss;
    file.seekp(0, std::ios::end);
    std::streampos filePos = file.tellp();
    
    for (auto &entry : entries) {
        if(entry.first.value == "~DELETED~")
        {
            entry.first.vlen = 0;
            continue;
        }
        entry.second = ss.tellp() + filePos; 
        ss.write(&vLogEntry::magic, 1);
        ss.write((char *)&entry.first.checksum, sizeof(entry.first.checksum));
        ss.write((char *)&entry.first.key, sizeof(entry.first.key));
        ss.write((char *)&entry.first.vlen, sizeof(entry.first.vlen));
        ss.write(&entry.first.value[0], entry.first.vlen);
    }
    if(!ss.str().empty())
    {
        file.seekp(0, std::ios::end); 
        file << ss.rdbuf();
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
    file.read((char *)&entry.vlen, sizeof(entry.vlen));
    entry.value.resize(entry.vlen);
    file.read(&entry.value[0], entry.vlen);

    std::vector<unsigned char> data;
    for (size_t i = 0; i < sizeof(entry.key); ++i) {
        data.push_back((entry.key >> (i * 8)) & 0xFF);
    }
    for (size_t i = 0; i < sizeof(entry.vlen); ++i) {
        data.push_back((entry.vlen >> (i * 8)) & 0xFF);
    }
    for (char c : entry.value) {
        data.push_back(c);
    }

    if (entry.checksum != utils::crc16(data)) {
        return "";
    }

    return entry.value;
}

vLogEntry vLog::getEntry(uint64_t offset, std::fstream &file)
{
    file.seekg(offset);
    char magic;
    file.read(&magic, 1);
    if (magic != vLogEntry::magic) {
        return vLogEntry();
    }

    vLogEntry entry;
    file.read((char *)&entry.checksum, sizeof(entry.checksum));
    file.read((char *)&entry.key, sizeof(entry.key));
    file.read((char *)&entry.vlen, sizeof(entry.vlen));
    entry.value.resize(entry.vlen);
    file.read(&entry.value[0], entry.vlen);

    std::vector<unsigned char> data;
    for (size_t i = 0; i < sizeof(entry.key); ++i) {
        data.push_back((entry.key >> (i * 8)) & 0xFF);
    }
    for (size_t i = 0; i < sizeof(entry.vlen); ++i) {
        data.push_back((entry.vlen >> (i * 8)) & 0xFF);
    }
    for (char c : entry.value) {
        data.push_back(c);
    }

    if (entry.checksum != utils::crc16(data)) {
        return vLogEntry();
    }

    return entry;
}

bool vLog::isValidEntry(uint64_t offset, std::fstream &file)
{
    // Save the current file position
    std::streampos currentPos = file.tellg();

    file.seekg(offset);
    char magic;
    file.read(&magic, 1);
    if (magic != vLogEntry::magic) {
        // Restore the file position before returning
        file.seekg(currentPos);
        return false;
    }

    vLogEntry entry;
    file.read((char *)&entry.checksum, sizeof(entry.checksum));
    file.read((char *)&entry.key, sizeof(entry.key));
    file.read((char *)&entry.vlen, sizeof(entry.vlen));
    entry.value.resize(entry.vlen);
    file.read(&entry.value[0], entry.vlen);

    std::vector<unsigned char> data;
    for (size_t i = 0; i < sizeof(entry.key); ++i) {
        data.push_back((entry.key >> (i * 8)) & 0xFF);
    }
    for (size_t i = 0; i < sizeof(entry.vlen); ++i) {
        data.push_back((entry.vlen >> (i * 8)) & 0xFF);
    }
    for (char c : entry.value) {
        data.push_back(c);
    }

    if (entry.checksum != utils::crc16(data)) {
        // Restore the file position before returning
        file.seekg(currentPos);
        return false;
    }

    // Restore the file position before returning
    file.seekg(currentPos);
    return true;
}