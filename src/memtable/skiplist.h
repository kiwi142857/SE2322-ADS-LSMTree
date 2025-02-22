#pragma once

#include <string>
#include <cstdint>
#include <random>
#include <list>
#include <utility>

class SkipList {
private:
    struct Node {
        uint64_t key;
        std::string value;
        Node **forward;
        int level;
        
        Node(uint64_t k, const std::string &v, int lvl);
        ~Node();
    };

    Node *head;
    int level;
    int maxLevel;
    float probability;
    uint64_t size;

    int randomLevel();

public:
    SkipList(int maxLevel = 12);
    ~SkipList();

    void insert(uint64_t key, const std::string &value);
    std::string search(uint64_t key) const;
    void remove(uint64_t key);
    void clear();
    void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list) const;
    uint64_t getSize() const { return size; }
    void setSize(uint64_t s) { size = s; }
};
