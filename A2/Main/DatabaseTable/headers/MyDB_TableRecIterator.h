#ifndef PAGE_REC_ITER_H
#define PAGE_REC_ITER_H

#include "../../Catalog/headers/MyDB_Table.h"
#include "../../Record/headers/MyDB_Record.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_RecordIterator.h"


#include <memory>
using namespace std;

// This pure virtual class is used to iterate through the records in a page or file
// Instances of this class will be created via calls to MyDB_PageReaderWriter.getIterator ()
// or MyDB_FileReaderWriter.getIterator ().
//
class MyDB_TableRecIterator;
typedef shared_ptr <MyDB_TableRecIterator> MyDB_TableRecIteratorPtr;

// DO NOT MODIFY!

class MyDB_TableRecIterator : public MyDB_RecordIterator {

public:

	// put the contents of the next record in the file/page into the iterator record
	// this should be called BEFORE the iterator record is first examined
	void getNext ();

	// return true iff there is another record in the file/page
	bool hasNext ();

	// destructor and contructor
	MyDB_TableRecIterator (MyDB_TableReaderWriter &myParent, MyDB_TablePtr myTable, MyDB_RecordPtr myRecord);
	~MyDB_TableRecIterator () {};

private:

	MyDB_TableReaderWriter &myParent;
	
	MyDB_TablePtr myTable;
	
	MyDB_RecordPtr myRecord;

	MyDB_RecordIteratorPtr myPageIterPtr;

	/*  */
	int page_index;


};

#endif