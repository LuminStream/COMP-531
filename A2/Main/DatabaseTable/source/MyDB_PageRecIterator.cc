
#ifndef PAGE_REC_ITER_CC
#define PAGE_REC_ITER_CC

#include "../headers/MyDB_PageRecIterator.h"
#include "../../Catalog/headers/MyDB_PageType.h"

#include <iostream>
#include <string>
using namespace std;

bool MyDB_PageRecIterator :: hasNext () {
    // return true iff there is another record in the file/page
    // Check if there has another record in the page other than current offset
    if ((*((size_t *) ( ((char *)(this -> currentPage -> getBytes())) + sizeof(MyDB_PageType)))) > offset) {
        return true;
    }
    else {
        return false;
    }
}

void MyDB_PageRecIterator :: getNext () {
    // put the contents of the next record in the file/page into the iterator record
	// this should be called BEFORE the iterator record is first examined
    // If we have the another record
    // Reference : https://piazza.com/class/jqmhgy0qw0z5oe?cid=94
    if (hasNext() == true) {
        char *currentPos = offset + ((char *)(this -> currentPage -> getBytes()));
        char *nextPos = (char *) this -> record -> fromBinary (currentPos);
        // The new offset equals to
        offset = (nextPos - currentPos) + offset;
    }
}

MyDB_PageRecIterator :: MyDB_PageRecIterator (MyDB_PageHandle currentPage, MyDB_RecordPtr record) {
    // Constructor of the page record iterator
    this -> currentPage = currentPage;
    this -> record = record;
    // Offset is the initial used bytes in a page
    this -> offset = sizeof(MyDB_PageType) + sizeof(size_t);
}

MyDB_PageRecIterator ::	~MyDB_PageRecIterator () { }

#endif