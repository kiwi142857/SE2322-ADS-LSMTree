#include "memTable.h"

MemTable::MemTable()
{
    skiplist = new skiplist::skiplist_type();
}

MemTable::~MemTable()
{
    delete skiplist;
}

void MemTable::put(uint64_t key, const std::string &val)
{
    skiplist->put(key, val);
}

std::string MemTable::get(uint64_t key) const
{
    return skiplist->get(key);
}

bool MemTable::del(uint64_t key)
{
    return skiplist->del(key);
}

void MemTable::print()
{
    skiplist->print();
}

void MemTable::scan(uint64_t start, uint64_t end, std::list<std::pair<uint64_t, std::string>> &list)
{   
    skiplist->scan(start, end, list);
    return;
}
// Path: MemTable/memTable.h