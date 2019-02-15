#ifndef PAGE_REC_ITER_H
#define PAGE_REC_ITER_H

#include "../../BufferMgr/headers/MyDB_PageHandle.h"
#include "../../Record/headers/MyDB_Record.h"
#include "MyDB_RecordIterator.h"

#include <memory>

using namespace std;

class MyDB_PageRecIterator;
typedef shared_ptr <MyDB_PageRecIterator> MyDB_PageRecIteratorPtr;

// IT IS PAGE_REC_ITERATOR

class MyDB_PageRecIterator : public MyDB_RecordIterator {

public:

	// put the contents of the next record in the file/page into the iterator record
	// this should be called BEFORE the iterator record is first examined
	void getNext ();

	// return true iff there is another record in the file/page
	bool hasNext ();

	// destructor and contructor
	MyDB_PageRecIterator (MyDB_PageHandle currentPage, MyDB_RecordPtr record);
	~MyDB_PageRecIterator ();

private:

    // Page handler and record 
    MyDB_PageHandle currentPage;
    MyDB_RecordPtr record;
    size_t offset;
};

#endif