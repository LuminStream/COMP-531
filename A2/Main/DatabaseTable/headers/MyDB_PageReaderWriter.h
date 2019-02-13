
#ifndef PAGE_RW_H
#define PAGE_RW_H

#include "MyDB_PageType.h"
#include "MyDB_TableReaderWriter.h"


#define PAGE_TYPE(pageByte) (*((MyDB_PageType *) pageByte))
#define USED_BYTE(pageByte) (*((size_t *) (((char *) pageByte) + sizeof (size_t))))

class MyDB_PageReaderWriter {

public:

	// ANY OTHER METHODS YOU WANT HERE

	// empties out the contents of this page, so that it has no records in it
	// the type of the page is set to MyDB_PageType :: RegularPage
	void clear ();	

	// return an itrator over this page... each time returnVal->next () is
	// called, the resulting record will be placed into the record pointed to
	// by iterateIntoMe
	MyDB_RecordIteratorPtr getIterator (MyDB_RecordPtr iterateIntoMe);

	// appends a record to this page... return false is the append fails because
	// there is not enough space on the page; otherwise, return true
	bool append (MyDB_RecordPtr appendMe);

	// gets the type of this page... this is just a value from an ennumeration
	// that is stored within the page
	MyDB_PageType getType ();

	// sets the type of the page
	void setType (MyDB_PageType toMe);

	// Constructor
	MyDB_PageReaderWriter(MyDB_PageHandle currentPage, size_t pageSize);

	// Deconstructor
	~MyDB_PageReaderWriter();
	
private:

	// ANYTHING ELSE YOU WANT HERE
	
	// This is the page we need
	MyDB_PageHandle currentPage;

	// We need to get bufferManager here
	size_t pageSize;
};

#endif
