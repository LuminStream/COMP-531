
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include <string>

using namespace std;

MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr, long) {
	// First create a HashMap map<pair<MyDB_TablePtr, long>, page> as a map to store all used pages
	// Then check if we have current page in the map, if has, return the page
	// Else, create a new page and store it in to the hashmap
	return nullptr;		
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
	return nullptr;		
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr, long) {
	return nullptr;		
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
	return nullptr;		
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
}

// Create a new bufferManager
MyDB_BufferManager :: MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile) {
	this -> pageSize = pageSize;
	this -> numPages = numPages;
	this -> tempFile = tempFile;

	// Allocate memory
	char* buffer = (char*) malloc(pageSize * numPages);
}

MyDB_BufferManager :: ~MyDB_BufferManager () {
}
	
#endif


