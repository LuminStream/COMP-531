#ifndef PAGE_C
#define PAGE_C

#include "../headers/MyDB_Page.h"
#include "../headers/MyDB_BufferManager.h"
#include <utility>

#include <iostream>
using namespace std;

/* constructor */
MyDB_Page :: MyDB_Page(MyDB_BufferManager &parentManager, MyDB_TablePtr parentTable, size_t offset) : parentManager(parentManager), parentTable(parentTable), offset(offset) {
    this->bytes = nullptr;
    this->reference_counter = 0;
    this->dirty = false;
    this->pinned = false;
}

/* destructor */
MyDB_Page :: ~MyDB_Page() {

}

/* add a reference to this page */
void MyDB_Page :: addReference() {
    this->reference_counter += 1;
}

/* reduce a reference to this page */
void MyDB_Page :: reduceReference() {
    this->reference_counter -= 1;
    if (this->reference_counter == 0) {
        this->parentManager.killPage(this->parentTable, this->offset);
    }
}

/* access the raw bytes in this page... if the page is not currently 
in the buffer, then the contents of the page are loaded from 
secondary storage.  */
void *MyDB_Page :: getBytes() {
    /* TODO: process the page */
    this->parentManager.managePage(this->parentTable, this->offset);
    
    return this->bytes;
}

/* let the page know we have written to the bytes */
void MyDB_Page :: wroteBytes() {
    this->dirty = true;
}

bool MyDB_Page :: isDirty() {
    return this->dirty;
}

bool MyDB_Page :: isPinned() {
    return this->pinned;
}

void MyDB_Page :: setBytes(void *ram) {
    this->bytes = ram;
}

void MyDB_Page :: setPin(bool pinned) {
    this->pinned = pinned;
}

void MyDB_Page :: setDirty(bool dirty) {
    this->dirty = dirty;
}

pair<MyDB_TablePtr, size_t> MyDB_Page :: getPageIndex() {
    return make_pair(this->parentTable, this->offset);
}

size_t MyDB_Page :: getOffset() {
    return this->offset;
}

MyDB_TablePtr MyDB_Page :: getParentTable() {
    return this->parentTable;
}

long MyDB_Page :: getReferenceCounter() {
    return this->reference_counter;
}

void* MyDB_Page :: returnBytes() {
    return this->bytes;
}

#endif