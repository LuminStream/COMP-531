
#ifndef PAGE_RW_C
#define PAGE_RW_C

#include "MyDB_PageReaderWriter.h"
#include "../../Record/headers/MyDB_Record.h"

//#define HEADER_SIZE (2 * sizeof (size_t));

void MyDB_PageReaderWriter :: clear () {
	// empties out the contents of this page
	//(*((size_t *) ((this -> currentPage -> getBytes()) + sizeof(MyDB_PageType))))
	// Set used bytes to the initial offsets
	size_t offset = sizeof(size_t) + sizeof(MyDB_PageType);
	(*((size_t *) ((this -> currentPage -> getBytes()) + sizeof(MyDB_PageType)))) = offset; 
	//USED_BYTE(this -> currentPage -> getBytes()) = HEADER_SIZE;
	this -> currentPage -> wroteBytes();
}

MyDB_PageType MyDB_PageReaderWriter :: getType () {
	// gets the type of this page... this is just a value from an ennumeration
	// that is stored within the page
	///////////////////////////////////////////////////////////////
	
	return (*(MyDB_PageType *) (this -> currentPage -> getBytes()))
}

MyDB_RecordIteratorPtr MyDB_PageReaderWriter :: getIterator (MyDB_RecordPtr record) {
	// return an itrator over this page... each time returnVal->next () is
	// called, the resulting record will be placed into the record pointed to
	// by iterateIntoMe
	return make_shared<MyDB_PageRecIterator>(currentPage, record);
}

void MyDB_PageReaderWriter :: setType (MyDB_PageType pageType) {
	// sets the type of the page to the enumerator
	//////////////////////////////////////////////////////////////////
	(*(MyDB_PageType *) (this -> currentPage -> getBytes())) = pageType; 
	currentPage -> wroteBytes();
}

bool MyDB_PageReaderWriter :: append (MyDB_RecordPtr record) {
	// First get the size of the record
	size_t recordSize = record -> getBinarySize();
	// Now get the available space on the page ////////////////////// Not sure
	//size_t usedSize = USED_BYTE(this -> currentPage -> getBytes());
	size_t usedSize = (*((size_t *)((this -> currentPage -> getBytes()) + sizeof(MyDB_PageType))))
	// Calculate remainning page size
	size_t remainSize = this -> pageSize - usedSize;

	char *head = ((char *) this -> currentPage -> getBytes());

	// If there is not enough space on the page, return false
	if (remainSize < recordSize) {
		return false;
	}
	else {
		// Append the record at the end of the page space
		record -> toBinary (usedSize + head);
		// Augment the usedSize
		usedSize = usedSize + recordSize;
		this -> currentPage -> wroteBytes();
	}
	return true;
}

MyDB_PageReaderWriter :: MyDB_PageReaderWriter (MyDB_PageHandle currentPage, size_t pageSize) {
	// Initialize attributes of pageReaderWriter
	this -> currentPage = currentPage;
	this -> pageSize = pageSize;
}

~MyDB_PageReaderWriter :: MyDB_PageReaderWriter () {

}

#endif
