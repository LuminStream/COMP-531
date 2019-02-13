
#ifndef PAGE_REC_ITER_CC
#define PAGE_REC_ITER_CC

#include "MyDB_PageRecIterator.h" 
#include "../headers/MyDB_PageRecIterator.h"

void MyDB_PageRecIterator :: hasNext () {
    // return true iff there is another record in the file/page
    if (USED_BYTE(this -> currentPage -> getBytes() > offest)) {
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
        char *currentPos = offest + (char *) this -> currentPage -> getBytes();
        char *nextPos = (char *) this -> record -> fromBinary (currentPos);
        offest += nextPos - currentPos;
    }
}

MyDB_PageRecIterator :: MyDB_PageRecIterator (MyDB_PageHandle currentPage, MyDB_RecordPtr record) {
    // Constructor of the page record iterator
    this -> currentPage = currentPage;
    this -> record = record;
    this -> offest = HEADER_SIZE;
}

MyDB_PageRecIterator ::	~MyDB_PageRecIterator () {

}