#ifndef TABLE_REC_ITER_C
#define TABLE_REC_ITER_C

#include "../headers/MyDB_PageReaderWriter.h"
#include "../headers/MyDB_TableRecIterator.h"

// put the contents of the next record in the file/page into the iterator record
// this should be called BEFORE the iterator record is first examined
void MyDB_TableRecIterator :: getNext() {
    this->myPageIterPtr->getNext();
}

// return true iff there is another record in the file/page
bool MyDB_TableRecIterator :: hasNext() {
    if (this->myPageIterPtr->hasNext()) {
        return true;
    }

    while (this->page_index < this->myTable->lastPage()) {
        this->page_index += 1;
        this->myPageIterPtr = this->myParent[this->page_index].getIterator(myRecord);
        // if there exists one page that has next record, return true 
        if (this->myPageIterPtr->hasNext()) {
            return true;
        }
    }

    return false;
}

// destructor and contructor
MyDB_TableRecIterator :: MyDB_TableRecIterator(MyDB_TableReaderWriter &myParent, MyDB_TablePtr myTable, MyDB_RecordPtr myRecord) : myParent(myParent) {
    this->myTable = myTable;
    this->myRecord = myRecord;
    this->page_index = 0;
    this->myPageIterPtr = this->myParent[this->page_index].getIterator(myRecord);
}

#endif