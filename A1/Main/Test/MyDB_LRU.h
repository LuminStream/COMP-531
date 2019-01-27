#ifndef LRU_H
#define LRU_H

#include "MyDB_Page.h"
#include <map>
#include <string>
using namespace std;

/* 
This is the node for LRU algorithm
Node contains page
Node has next and prev pointers
*/
class Node{

public:
    // Constructor
    Node (Page *page);

    // Get page
    Page getPage();

    // Set Page
    void setPage(Page *page);

    // Ptr to next node
    Node *next;

    // Ptr to previous node
    Node *prev;

private:
    // Stored pages
    Page *page;
};

/*
This is the LRUCache
Performe the LRU algorithm
The most recent used node will be moved to the front of the list
Tail has the least recent used node
*/
class LRUCache{
    
public:
    // Add a new Node after the head, set as the first node of the double linked list
    void addNewNode(Node *node);

    // Remove a node
    void removeNode(Node *node);

    // Set a node to the front
    void moveToHead(Node *node);

    // Pop the tail node
    Node* popTail();

    // Return the head node
    Node* getHead();

    // Return the tail node
    Node* getTail();
    
    // Add a newNode to the list
    void addToList(pair<int, long> identifier, Page *page);

    // return current cache size
    int cacheSize();

    // Constructor
    LRUCache(int capacity);

    // This is a hashMap
    // Where key is pair<int, long> copntains the identifier of page
    // Value is the Node
    map<pair<int, long>, Node*> map;

private:
    // The size of LRU cache
    int capacity;

    // Count current number of nodes in the LRU Cache
    int count;

    // Preset head node
    Node *head;

    // Preset tail node
    Node *tail;

};

#endif