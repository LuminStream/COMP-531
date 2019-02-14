
#ifndef PAGE_REC_ITER_CC
#define PAGE_REC_ITER_CC

#include "MyDB_PageType.h"
#include "MyDB_PageRecIterator.h" 
#include "../headers/MyDB_PageRecIterator.h"

void MyDB_PageRecIterator :: hasNext () {
    // return true iff there is another record in the file/page
    // *((size_t *) (this -> currentPage -> getBytes()) + sizeof(size_t)) > offset

    // Check if there has another record in the page other than current offset
    if ((*((size_t *) ((this -> currentPage -> getBytes()) + sizeof(MyDB_PageType)))) > offset) {
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
    if (hasNext() == true) {
        char *currentPos = offset + (char *) this -> currentPage -> getBytes();
        char *nextPos = (char *) this -> record -> fromBinary (currentPos);
        // The new offset equals to the 
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

MyDB_PageRecIterator ::	~MyDB_PageRecIterator () {

}