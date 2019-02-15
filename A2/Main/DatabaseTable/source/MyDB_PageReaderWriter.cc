
#ifndef PAGE_RW_C
#define PAGE_RW_C

#include "../headers/MyDB_PageReaderWriter.h"
#include "../headers/MyDB_PageRecIterator.h"

#include <memory>

void MyDB_PageReaderWriter :: clear () {
	// empties out the contents of this page
	// Set used bytes to the initial offsets
	size_t offset = sizeof(size_t) + sizeof(MyDB_PageType);
	(*((size_t *) (((char *)(this -> currentPage -> getBytes())) + sizeof(MyDB_PageType)))) = offset; 
	this -> currentPage -> wroteBytes();
}

MyDB_PageType MyDB_PageReaderWriter :: getType () {
	// gets the type of this page... this is just a value from an ennumeration
	// that is stored within the page
	return (*(MyDB_PageType *) (this -> currentPage -> getBytes()));
}

MyDB_RecordIteratorPtr MyDB_PageReaderWriter :: getIterator (MyDB_RecordPtr record) {
	// return an itrator over this page... each time returnVal->next () is
	// called, the resulting record will be placed into the record pointed to
	// by iterateIntoMe
	MyDB_RecordIteratorPtr return_iter = make_shared<MyDB_PageRecIterator>(this->currentPage, record);
	return return_iter;
}

void MyDB_PageReaderWriter :: setType (MyDB_PageType pageType) {
	// sets the type of the page to the enumerator and then update current page
	(*(MyDB_PageType *) (this -> currentPage -> getBytes())) = pageType; 
	currentPage -> wroteBytes();
}

bool MyDB_PageReaderWriter :: append (MyDB_RecordPtr record) {
	// cout << "here is page append" << endl;
	// First get the size of the record
	size_t recordSize = record -> getBinarySize();
	size_t usedSize = (*((size_t *)( ((char *)(this -> currentPage -> getBytes())) + sizeof(MyDB_PageType))));
	// Calculate remainning page size
	size_t remainSize = this -> pageSize - usedSize;

	// cout << "record size is: " << recordSize << endl;
	// cout << "remainSize is: " << remainSize << endl;

	char *myBytes = ((char *) this -> currentPage -> getBytes());

	// If there is not enough space on the page, return false
	if (remainSize < recordSize) {
		return false;
	}
	else {
		// Append the record at the end of the page space
		record -> toBinary (usedSize + myBytes);
		// Augment the usedSize
		(*((size_t *)( ((char *)(this -> currentPage -> getBytes())) + sizeof(MyDB_PageType)))) += recordSize;
		// cout << "after + record size, the used size is: " << usedSize << endl;
		this -> currentPage -> wroteBytes();
	}
	return true;
}

MyDB_PageReaderWriter :: MyDB_PageReaderWriter (MyDB_PageHandle currentPage, size_t pageSize) {
	// Initialize attributes of pageReaderWriter
	this -> currentPage = currentPage;
	this -> pageSize = pageSize;
}

MyDB_PageReaderWriter :: ~MyDB_PageReaderWriter () { }

#endif
