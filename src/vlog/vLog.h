#pragma once

#include <string>
#include <fstream>
#include <cstdint>

struct vLogEntry {
    static const char magic = 'v';
    uint64_t key;
    uint32_t vlen;
    std::string value;
    
    vLogEntry(uint64_t k, const std::string &v)
        : key(k), vlen(v.length()), value(v) {}
};

class vLog {
public:
    static bool isValidEntry(off_t offset, std::fstream &file);
    static vLogEntry getEntry(off_t offset, std::fstream &file);
    static void writeEntry(const vLogEntry &entry, std::fstream &file);
};
