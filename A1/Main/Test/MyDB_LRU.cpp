
#ifndef LRU_C
#define LRU_C
#include <iostream>
#include <map>
#include "MyDB_LRU.h"

using namespace std;

// Add a new Node after the head, set as the first node of the double linked list
void LRUCache :: addNewNode (Node *node) {
    node -> prev = head;
    node -> next = head -> next;

    head -> next -> prev = node;
    head -> next = node;
}

// Remove a node
void LRUCache :: removeNode (Node *node) {
    Node *preNode = node -> prev;
    Node *nextNode = node -> next;

    preNode -> next = nextNode;
    nextNode -> prev = preNode;
}

// Set a node to the front
void LRUCache :: moveToHead (Node *node) {
    this -> removeNode(node);
    this -> addNewNode(node);
}

// Pop the tail node
Node* LRUCache :: popTail () {
    Node *lastNode = tail -> prev;
    this -> removeNode(lastNode);
    return lastNode;
}

// Return the head node
Node* LRUCache :: getHead () {
    return head -> next;
}

// Return the tail node
Node* LRUCache :: getTail () {
    return tail -> prev;
}

// Add new page to the LRU cache
// We use map to keep track each page, the key is the page identifier pair<int, long>, and the value is a Node which contains page
// We create a new Node and put it at the front of the LRU when map does not have the key - page identifier
// If map has the key, then we update the page in that node and then set it to the head of the LRU
void LRUCache :: addToList(pair<int, long> identifier, Page *page) {
    // If we have the node in map, then update page in that node, then move to head
    if (map.find(identifier) != map.end()) {
        cout << "has this node" << endl;
        Node *currentNode = map.find(identifier) -> second;
        currentNode -> setPage(page);
        moveToHead(currentNode);
    }
    else{
        // If we dont have that node, then create new one and insert to map
        // Then add to head of the list, check the total number of nodes, pop the tail if exceeds the capacity
        cout << "no this node" << endl;
        Node *newNode = new Node(page);
        map[make_pair(identifier.first, identifier.second)] = newNode;
        addNewNode(newNode);
        count++;
        if (count > capacity) {
            Node *tailNode = popTail();
            Page tailPage = tailNode -> getPage();
            map.erase(tailPage.getPair());
            count--;
        }
    }
}

// return current cache size
int LRUCache :: cacheSize () {
    return count;
}

// Constructor for LRUCache
LRUCache :: LRUCache (int capacity) {
    // Set up capacity and count
    this -> capacity = capacity;
    this -> count = 0;

    // Initialize head and tail
    // Head and tail are two nullptr node
    // Prevent null pointer
    // All page nodes are between head and tail
    head = new Node(nullptr);
    tail = new Node(nullptr);

    head -> prev = nullptr;
    tail -> next = nullptr;

    head -> next = tail;
    tail -> prev = head;

}

// Constructor for Node 
Node :: Node (Page *page) {
    this -> page = page;
    this -> next = nullptr;
    this -> prev = nullptr;
}

// Get page
Page Node :: getPage () {
    return *page;
}

// Set page
void Node :: setPage (Page *page) {
    this -> page = page;
}

#endif