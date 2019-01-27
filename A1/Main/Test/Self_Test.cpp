
#ifndef TEST_CPP
#define TEST_CPP
#include <iostream>

#include "MyDB_LRU.h"
#include "MyDB_Page.h"

using namespace std;

void returnHead (LRUCache *table);
void returnTail (LRUCache *table);

int main () {
    cout << "hello world" << endl;

    // First create 3 pages
    Page *page1 = new Page("page1",1,1);
    cout << "current page identifier is:";
    cout << page1 -> getPair().first << endl;
    Page *page2 = new Page("page2",2,2);
    Page *page3 = new Page("page3",3,3);
    Page *page4 = new Page("page4",4,4);
    Page *page5 = new Page("page5",5,5);
    Page *page6 = new Page("page6",6,6);

    // Create LRU table
    LRUCache *table = new LRUCache(5);

    cout << "add page1 to table" << endl;
    table -> addToList(page1 -> getPair(), page1);  // Head should be page1
    returnHead(table);

    cout << "add page2 to table" << endl;
    table -> addToList(page2 -> getPair(), page2);  // Head should be page2
    returnHead(table);

    cout << "add page3 to table" << endl;
    table -> addToList(page3 -> getPair(), page3);  // Head should be page3
    returnHead(table);

    cout << "add page4 to table" << endl;
    table -> addToList(page4 -> getPair(), page4);  // Head should be page4
    returnHead(table);

    cout << "add page5 to table" << endl;
    table -> addToList(page5 -> getPair(), page5);  // Head should be page5
    returnHead(table);

    cout << "add page6 to table" << endl;
    table -> addToList(page6 -> getPair(), page6);  // Head should be page6 and the size should still be 5
    returnTail(table);  // Should return page2
    returnHead(table);

    // Now add a updated page with same identifier but different ID
    Page *newPage = new Page("pageUpdate",4,4); // Set id is 4,4
    table -> addToList(newPage -> getPair(), newPage); 
    // Should return "pageUpdate" with pageId 4,4
    // The size should be still 5 and last node should be still 2
    returnTail(table); 
    returnHead(table);
    return 0;
}

void returnHead (LRUCache *table) {
    // Return the head node
    Node *temp = table -> getHead();
    Page test = temp -> getPage();
    cout << "current head is: ";
    cout << test.getId() << endl;  
    cout << "the size of LRU is: ";
    cout << table -> cacheSize() << endl;
    cout << "page identifier is: ";
    cout << test.getPair().first << endl;
    cout << "--------------------" << endl;
}

void returnTail (LRUCache *table) {
    // Return the head node
    Node *temp = table -> getTail();
    Page test = temp -> getPage();
    cout << "---------" << endl;
    cout << "current tail is: ";
    cout << test.getId() << endl;   
    cout << "page identifier is: ";
    cout << test.getPair().first << endl;
}


#endif