
#ifndef TABLE_RW_C
#define TABLE_RW_C

#include <fstream>
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_TableRecIterator.h"

using namespace std;

MyDB_TableReaderWriter :: MyDB_TableReaderWriter (MyDB_TablePtr table, MyDB_BufferManagerPtr bufferManager) {
	this -> table = table;
	this -> bufferManager = bufferManager;

	// If we dont have last page
	if (this -> table -> lastPage () == -1) {
		this -> table -> setLastPage (0);
		//lastPage = make_shared <MyDB_PageReaderWriter> (*this, table -> lastPage());
		MyDB_PageReaderWriter lastPage = new MyDB_PageReaderWriter(*this, this -> table -> lastPage());
		lastPage -> clear();
		///////////////////////////////////
	}
	/* Set the last page
	else {
		lastPage = make_shared <MyDB_PageReaderWriter> (*this, table -> lastPage());
	}
	*/
}

MyDB_PageReaderWriter MyDB_TableReaderWriter :: operator [] (size_t id) {
	MyDB_PageReaderWriter temp;
	return temp;	
}

MyDB_RecordPtr MyDB_TableReaderWriter :: getEmptyRecord () {
	this -> emptyRecord = make_shared <MyDB_record> (this -> table -> getSchema());
	return this -> emptyRecord;
}

MyDB_PageReaderWriter MyDB_TableReaderWriter :: last () {
	MyDB_PageReaderWriter temp = new MyDB_PageReaderWriter(*this, this -> table -> lastPage());
	return temp;	
}

void MyDB_TableReaderWriter :: append (MyDB_RecordPtr) {
}

void MyDB_TableReaderWriter :: loadFromTextFile (string fileName) {
	// Empty the database ???
	this -> table -> setLastPage (0);
	//MyDB_PageReaderWriter lastPage = new MyDB_PageReaderWriter(*this, this -> table -> lastPage());
	shared_pr <MyDB_PageReaderWriter> lastPage = make_shared<MyDB_PageReaderWriter>(*this, this -> table -> lastPage());
	lastPage -> clear();

	ifstream fileStream;
	fileStream.open(fileName, std::ofstream::in | std::ofstream::trunc);
	string line;

	if (fileStream.is_open()) {
		// Get an empty record created from schema for the table
		getEmptyRecord();
		// Read from the text file
		while (getLine(fileName, line)) {
			this -> emptyRecord -> fromString(line);
			append(this -> emptyRecord);
		}

		fileStream.close();
	}	
}

MyDB_RecordIteratorPtr MyDB_TableReaderWriter :: getIterator (MyDB_RecordPtr record) {
	return make_shared <MyDB_TableRecIterator> (*this, this -> table, record);
}

void MyDB_TableReaderWriter :: writeIntoTextFile (string fileName) {
	// Open the file use ofstream
	ofstream fileStream;
	fileStream.open(fileName, std::ofstream::out | std::ofstream::trunc);
	// When file is open
	if (fileStream.is_open()) {
		// Get an empty record
		getEmptyRecord();
		MyDB_RecordIteratorPtr recIter = getIterator(this -> emptyRecord);
		// Write records into text file
		while (recIter -> hasNext()) {
			recIter -> getNext();
		}

		// close file
		fileStream.close();
	}
}

#endif

