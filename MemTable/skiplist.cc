#include "skiplist.h"
#include <iostream>
namespace skiplist
{

skiplist_type::skiplist_type(double p)
{
    this->p = p;
    head = new Node(0, "", max_level);
    size = 0;
}

void skiplist_type::put(key_type key, const value_type &val)
{
    Node *update[max_level];
    Node *node = head;
    for (int i = max_level - 1; i >= 0; i--) {
        while (node->forward[i] != nullptr && node->forward[i]->key < key) {
            node = node->forward[i];
        }
        update[i] = node;
    }
    node = node->forward[0];
    if (node != nullptr && node->key == key) {
        node->value = val;
    } else {
        int level = random_level();
        
        node = new Node(key, val, level);
        for (int i = 0; i < level; i++) {
            node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = node;
        }
        size++;
    }
}

value_type skiplist_type::get(key_type key) const
{
    Node *node = head;
    for (int i = max_level - 1; i >= 0; i--) {
        while (node->forward[i] != nullptr && node->forward[i]->key < key) {
            node = node->forward[i];
        }
    }
    node = node->forward[0];
    return node != nullptr && node->key == key ? node->value : "";
}

void skiplist_type::print()
{
    Node *node = head;
    std::cout << std::endl;
    for (int i = max_level - 1; i >= 0; i--) {
        node = head;
        if (node->forward[i] != nullptr) {
            std::cout << std::endl;
            std::cout << "Level " << i << ": ";
        } else {
            continue;
        }
        while (node->forward[i] != nullptr) {
            std::cout << node->forward[i]->key << " ";
            node = node->forward[i];
        }
    }
    std::cout << std::endl;
}

/* return nullptr if no element satisfied */
void skiplist_type::scan(key_type start, key_type end, std::list<std::pair<uint64_t, std::string>> &list)
{
    // get the start node
    Node *start_node = head;
    for (int i = max_level - 1; i >= 0; i--) {
        while (start_node->forward[i] != nullptr && start_node->forward[i]->key < start) {
            start_node = start_node->forward[i];
        }
    }
    start_node = start_node->forward[0];
    if (start_node == nullptr || start_node->key > end) {
        return;
    }

    Node *node = start_node;
    // add the key and value of node whose key is in [start, end] to the list
    while (node != nullptr && node->key <= end) {
        list.emplace_back(std::make_pair(node->key, node->value));
        node = node->forward[0];
    }
}

bool skiplist_type::del(key_type key)
{
    Node *update[max_level];
    Node *node = head;
    for (int i = max_level - 1; i >= 0; i--) {
        while (node->forward[i] != nullptr && node->forward[i]->key < key) {
            node = node->forward[i];
        }
        update[i] = node;
    }
    node = node->forward[0];
    if (node != nullptr && node->key == key) {
        for (int i = 0; i < max_level; i++) {
            if (update[i]->forward[i] != node) {
                break;
            }
            update[i]->forward[i] = node->forward[i];
        }
        delete node;
        size--;
        return true;
    }
    return false;
}

skiplist_type::~skiplist_type()
{
    Node *node = head;
    while (node != nullptr) {
        Node *tmp = node;
        node = node->forward[0];
        delete tmp;
    }
}

void skiplist_type::getList(std::list<std::pair<key_type, value_type>> &list) 
{
    Node *node = head->forward[0];
    while (node != nullptr) {
        list.emplace_back(std::make_pair(node->key, node->value));
        node = node->forward[0];
    }
}
} // namespace skiplist
