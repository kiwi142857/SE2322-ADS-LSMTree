#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <cstdint>
// #include <optional>
#include <string>
#include <vector>
#include <list>

namespace skiplist
{
using key_type = uint64_t;
// using value_type = std::vector<char>;
using value_type = std::string;

class skiplist_type
{
    // add something here
    class Node
    {
      public:
        key_type key;
        value_type value;
        std::vector<Node *> forward;
        int level;
        Node(key_type key, const value_type &val, int level) : key(key), value(val), level(level){
			forward.resize(level);
		};
    };

	  const int max_level = 16;  //define the max level of the skiplist
    double p;  //define the probability of the skiplist
    Node *head;  //define the head of the skiplist
    int size;  //define the size of the skiplist

    int random_level()
    {
        int level = 1;
        while (rand() < p * RAND_MAX && level < max_level)
            level++;
        return level;
    };
    
	
  public:
    explicit skiplist_type(double p = 0.5);
    void put(key_type key, const value_type &val);
    // std::optional<value_type> get(key_type key) const;
    value_type get(key_type key) const;
    bool del(key_type key);
    // for hw1 only
    int query_distance(key_type key) const;
    void scan(key_type start, key_type end, std::list<std::pair<key_type, value_type>> &list);
    ~skiplist_type();
	void print();
};

} // namespace skiplist

#endif // SKIPLIST_H
