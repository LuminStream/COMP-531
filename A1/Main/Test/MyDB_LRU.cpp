
#ifndef LRU_C
#define LRU_C
#include <iostream>
#include <map>
#include "MyDB_LRU.h"
#include "MyDB_BufferManager.h"

using namespace std;

// Add a new Node after the head, set as the first node of the double linked list
void LRUCache :: addNewNode (Node *node) {
    node -> prev = head;
    node -> next = head -> next;

    head -> next -> prev = node;
    head -> next = node;
}

// Remove a node from LinkedList
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

// Pop the last not pinned node
// If the current tail node is pinned page
// We move forward one step
Node* LRUCache :: popTail () {
    // 
    Node *lastNode = tail -> prev;
    while (lastNode -> getPage().isPinned() != true) {
        lastNode = lastNode -> prev;
    }

    if (lastNode == head) {
        return nullptr;
    }
    else {
        killNode(lastNode);
        return lastNode;
    }
    /*
    Node *lastNode = tail -> prev;
    this -> removeNode(lastNode);
    return lastNode;
    */
}

// Return the head node
Node* LRUCache :: getHead () {
    return head -> next;
}

// Return the tail node
Node* LRUCache :: getTail () {
    return tail -> prev;
}

// Find the target node with page identifier


// Add new page to the LRU cache
// We use map to keep track each page, the key is the page identifier pair<int, long>, and the value is a Node which contains page
// We create a new Node and put it at the front of the LRU when map does not have the key - page identifier
// If map has the key, then we update the page in that node and then set it to the head of the LRU
/*
Node* LRUCache :: addToList(pair<MyDB_TablePtr, size_t> identifier, Page *page) {
    // If we have the node in map, then update page in that node, then move to head
    if (map.find(identifier) != map.end()) {
        cout << "has this node" << endl;
        Node *currentNode = map.find(identifier) -> second;
        currentNode -> setPage(page);
        moveToHead(currentNode);
        return currentNode;
    }
    else{
        // If we dont have that node, then create new one and insert to map
        // Then add to head of the list, check the total number of nodes, pop the tail if exceeds the capacity
        // First check if we have available slot for that new page
        if (count == capacity) {
            // Kick out the least used not pinned node
            Node *tailNode = popTail()
            // No extra position for current adding node
            if (tailNode != nullptr) {
                return nullptr;
            }
            else {
                count--;
            }
        }

        if () {        }
        // Pop out last node, then add new node
            else {
                // Set ram to this page
                page -> setByte(availableRam[availableRam.size () - 1]);
                Node *newNode = new Node(page);
                map[make_pair(identifier.first, identifier.second)] = newNode;
                addNewNode(newNode);

            }
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
*/

// Add a new node to the list
Node* LRUCache :: addToList (pair<MyDB_TablePtr, size_t> identifier, Page *page) {
    // Give page byte address
    page -> setByte(availableRam[availableRam.size () - 1]);
    // Create a new node
    Node *newNode = new Node(page);
    // Push node to the hashMap
    map[make_pair(identifier.first, identifier.second)] = newNode;
    // Add this node to the head of the list
    addNewNode(newNode);
    count++;
    return newNode;
}

// Delete a node
void LRUCache :: killNode (Node* node) {
    // Remove node from list
    removeNode(node);
    Page page = node -> getPage();
    if (page.getByte() != nullptr) {
        // push byte back to Ram
        availableRam.push_back (page.getByte());
    }
    // erase key from hashmap
    map.erase(page.getPageIndex());
}

// Check if list is full
bool isFull () {
    return count == capacity;
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