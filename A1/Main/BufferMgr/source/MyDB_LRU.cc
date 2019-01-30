
#ifndef LRU_C
#define LRU_C
#include <iostream>
#include <map>
#include "../headers/MyDB_LRU.h"
#include "../headers/MyDB_BufferManager.h"
#include "../headers/MyDB_Page.h"

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>

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
    // cout << "what? wrong at the begining?" << endl;
    int counter = 0;
    while (counter < this->capacity) {
        if (lastNode->getPage()->isPinned() == false) {
            break;
        }
        lastNode = lastNode -> prev;
        counter ++;
    }

    if (lastNode == head) {
        // cout << "what? nullptr?" << endl;
        return nullptr;
    }
    else {
        /* write the bytes back to disk if the bytes is dirty */
        MyDB_PagePtr page = lastNode->getPage();
	    if (page->isDirty() == true) {
            int file_descriptor;
            if (page->getParentTable() == nullptr) {
                file_descriptor = open (this->parentManager.tempFile.c_str (), O_CREAT | O_RDWR | O_SYNC, 0666);
            } else {
                file_descriptor = open (page->getParentTable()->getStorageLoc().c_str (), O_CREAT | O_RDWR | O_SYNC, 0666);
            }
            lseek (file_descriptor, page->getOffset() * this->parentManager.pageSize, SEEK_SET);
            write (file_descriptor, page->bytes, this->parentManager.pageSize);
            close (file_descriptor);
            page->setDirty(false);
        }

        this->deleteNode(lastNode);

        if (page->getReferenceCounter() == 0) {
            this->parentManager.killPage(page->getParentTable(), page->getOffset());
        }

        return lastNode;
    }
}

// Return the head node
Node* LRUCache :: getHead () {
    return head -> next;
}

// Return the tail node
Node* LRUCache :: getTail () {
    return tail -> prev;
}

// Add a new node to the list
Node* LRUCache :: addToList (pair<MyDB_TablePtr, size_t> identifier, MyDB_PagePtr page) {
    // Give page byte address
    page->bytes = this->parentManager.availableRam[this->parentManager.availableRam.size () - 1];

    this->parentManager.availableRam.pop_back();

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
void LRUCache :: deleteNode (Node* node) {
    // Remove node from list
    removeNode(node);
    MyDB_PagePtr page = node -> getPage();
    // if (page->bytes != nullptr) {
    // push byte back to Ram
    this->parentManager.availableRam.push_back (page->bytes);
    // erase key from hashmap
    map.erase(page -> getPageIndex());
    page->bytes = nullptr;
    count--;
}

// Find a node by checking identifier
Node* LRUCache :: findNode (pair<MyDB_TablePtr, size_t> identifier) {
    // Find identifier in hashmap
    if (map.find(identifier) != map.end()) {
        // We find it, set it to the head of the list, then return
        Node *curr = map.find(identifier) -> second;
        moveToHead(curr);
        return curr;
    }
    else {
        // No such record in the map, return nullptr
        return nullptr;
    }
}

// Check if list is full
bool LRUCache :: isFull () {
    return this->count == this->capacity;
}

// return current cache size
int LRUCache :: cacheSize () {
    return this->count;
}

// Constructor for LRUCache
LRUCache :: LRUCache (int capacity, MyDB_BufferManager &parentManager) : parentManager(parentManager) {
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

LRUCache :: ~LRUCache(){
    while(count >= 0){
        Node *node = tail -> prev;
        delete node;
        node = node -> prev;
    }
}


// Constructor for Node 
Node :: Node (MyDB_PagePtr page) {
    this -> page = page;
    this -> next = nullptr;
    this -> prev = nullptr;
}

// Get page
MyDB_PagePtr Node :: getPage () {
    return this->page;
}

// Set page
void Node :: setPage (MyDB_PagePtr page) {
    this -> page = page;
}

#endif