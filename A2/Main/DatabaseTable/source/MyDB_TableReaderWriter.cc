
#ifndef TABLE_RW_C
#define TABLE_RW_C

#include <fstream>
#include <sstream>

#include "../headers/MyDB_PageReaderWriter.h"
#include "../headers/MyDB_TableRecIterator.h"
#include "../headers/MyDB_TableReaderWriter.h"

using namespace std;

MyDB_TableReaderWriter :: MyDB_TableReaderWriter (MyDB_TablePtr table, MyDB_BufferManagerPtr bufferManager) {
	this -> table = table;
	this -> bufferManager = bufferManager;

	// If we dont have last page
	if (this -> table -> lastPage () == -1) {
		this -> table -> setLastPage (0);
		// Clear out the last page
		(*this)[0].clear();
	}
}

MyDB_PageReaderWriter MyDB_TableReaderWriter :: operator [] (size_t id) {
	// MyDB_PageReaderWriter temp;
	// return temp;	
	MyDB_PageHandle myPage = this->bufferManager->getPage(this->table, id);
	while (this->table->lastPage() < id) {
		this->table->setLastPage(this->table->lastPage() + 1);
		shared_ptr<MyDB_PageReaderWriter> temp_prw = make_shared<MyDB_PageReaderWriter> (myPage, this->bufferManager->getPageSize());
		temp_prw->clear();
	}

	this->pageMap[id] = make_shared<MyDB_PageReaderWriter> (myPage, this->bufferManager->getPageSize());
	return *this->pageMap[id];
}

MyDB_RecordPtr MyDB_TableReaderWriter :: getEmptyRecord () {
	this -> emptyRecord = make_shared <MyDB_Record> (this -> table -> getSchema());
	return this -> emptyRecord;
}

MyDB_PageReaderWriter MyDB_TableReaderWriter :: last () {
	int last = this->table->lastPage();
	return (*this)[last];	
}

void MyDB_TableReaderWriter :: append (MyDB_RecordPtr myRecord) {
	// cout << "here is table append" << endl;
	while (!this->last().append(myRecord)) {
        int last = this->table->lastPage() + 1;
        this->table->setLastPage(last);
        (*this)[last].clear();
    }
}

void MyDB_TableReaderWriter :: loadFromTextFile (string fileName) {

    for (int i = 0; i <= this->table->lastPage(); i++) {
        (*this)[i].clear();
    }

	this -> table -> setLastPage (0);

	// Reference: https://piazza.com/class/jqmhgy0qw0z5oe?cid=102
	ifstream fileStream;
	fileStream.open(fileName);
	string line;

	if (fileStream.is_open()) {
		// Get an empty record created from schema for the table
		getEmptyRecord();
		// Read from the text file
		while (getline(fileStream, line)) {
			this -> emptyRecord -> fromString(line);
			// cout << "empty record is: " << emptyRecord << endl;
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
	// Reference: http://www.cplusplus.com/reference/fstream/ofstream/open/
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

