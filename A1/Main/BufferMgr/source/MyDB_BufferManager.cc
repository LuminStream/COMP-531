
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "../headers/MyDB_BufferManager.h"
#include "../headers/MyDB_LRU.h"
#include <string>
#include <utility>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>

using namespace std;

MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr whichTable, long i) {
	/* ensure the table pointer is not null */
	if (whichTable == nullptr) {
		exit(1); // Oops! there is some error with the pointer 
	}

	pair<MyDB_TablePtr, long> page_index = make_pair(whichTable, i);

	/* search the page in the all-pages map */
	map<pair<MyDB_TablePtr, long>, MyDB_PagePtr>::iterator iter;
	iter = this->all_page_map.find(page_index);
	/* if not found, create a new page and put it into the map, then return handle */
	if (iter == all_page_map.end()) {
		MyDB_PagePtr new_page = make_shared<MyDB_Page> (*this, whichTable, i);
		this->all_page_map[page_index] = new_page;
		return make_shared <MyDB_PageHandleBase> (new_page);
	/* if found, return the handle */
	} else {
		return make_shared <MyDB_PageHandleBase> (this->all_page_map[page_index]);
	}	
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
	pair<MyDB_TablePtr, long> anonymous_page_index = make_pair(nullptr, this->anonymous_page_counter);
	MyDB_PagePtr new_anonymous_page = make_shared<MyDB_Page> (*this, nullptr, this->anonymous_page_counter);
	this->all_page_map[anonymous_page_index] = new_anonymous_page;
	this->anonymous_page_counter += 1;
	return make_shared <MyDB_PageHandleBase> (new_anonymous_page);
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr whichTable, long i) {
	/* ensure the table pointer is not null */
	if (whichTable == nullptr) {
		exit(1); // Oops! there is some error with the pointer 
	}

	pair<MyDB_TablePtr, long> page_index = make_pair(whichTable, i);
	MyDB_PagePtr return_page;

	/* search the page in the all-pages map */
	map<pair<MyDB_TablePtr, long>, MyDB_PagePtr>::iterator iter;
	iter = this->all_page_map.find(page_index);
	/* if found, then check if the page is in the LRU list */
	if (iter != this->all_page_map.end()) {
		return_page = this->all_page_map[page_index];
		if (this->LRU_cache->findNode(page_index) != nullptr) {
			return_page->setPin(true);
		}
	} else {
		return_page = make_shared <MyDB_Page> (*this, whichTable, i);
		this->all_page_map[page_index] = return_page;
	}

	/* check if this page is put in the LRU list if not, put it in
	while there is available ram */
	if (return_page->bytes == nullptr) {
		if (this->availableRam.size() == 0) {
			this->moveOutLRUPage();
		}

		if (this->availableRam.size() == 0) {
			return nullptr;
		}

		/* allocate ram to the return page */
		this->LRU_cache->addToList(page_index, return_page);
		// return_page->setBytes(this->availableRam[this->availableRam.size() - 1]);
		// this->availableRam.pop_back();

		/* read it? */
		int file_descriptor;
		if (whichTable == nullptr) {
			file_descriptor = open (this->tempFile.c_str (), O_CREAT | O_RDWR | O_SYNC, 0666);
		} else {
			file_descriptor = open (whichTable->getStorageLoc ().c_str (), O_CREAT | O_RDWR | O_SYNC, 0666);
		}
		lseek (file_descriptor, return_page->getOffset() * this->pageSize, SEEK_SET);
		read (file_descriptor, return_page->bytes, pageSize);
		close (file_descriptor);

	}

	return make_shared <MyDB_PageHandleBase> (return_page);	
}


MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
	/* check if there is available ram */
	if (this->availableRam.size() == 0) {
		cout << "get pinned page has no more availableRam" << endl;
		this->moveOutLRUPage();
	}

	if (this->availableRam.size() == 0) {
		return nullptr;
	}

	MyDB_PageHandle return_handle = this->getPage();
	MyDB_PagePtr page = return_handle->getMyPage();
	pair<MyDB_TablePtr, long> page_index = make_pair(page->getParentTable(), page->getOffset());
	this->LRU_cache->addToList(page_index, page);
	// cout << "we assign a address to page: "<< availableRam[this->availableRam.size() - 1] << endl;
	// return_handle->getMyPage()->setBytes(this->availableRam[this->availableRam.size() - 1]);
	// this->availableRam.pop_back();
	cout << "==============================************************************========================" << endl;
	return return_handle;
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
	unpinMe->getMyPage()->setPin(false);
}


// Check if current page is in the LRU list
// If not add
// Call getByte() before called getPage() ?
void MyDB_BufferManager :: managePage (MyDB_TablePtr whichTable, long i) {
	pair<MyDB_TablePtr, long> page_index = make_pair(whichTable, i);
	if (this->LRU_cache->findNode(page_index) != nullptr) {
		cout << "find it!" << endl; 
		this->LRU_cache->moveToHead(this->LRU_cache->findNode(page_index));
	} else {
		cout << "there is no such page in LRU list" << endl;
		//if (this->availableRam.size() == 0) {
		  if (this->LRU_cache->cacheSize() == 16) {
			cout << "now I will kick one page out of LRU list" << endl;
			this->moveOutLRUPage();
			cout << "ok after kick out page" << endl;
		}
		
		//if (this->availableRam.size() == 0) {
		  if (this->LRU_cache -> cacheSize() == 16) {
			cout << "oh god! there is no available ram!" << endl;
			exit(1);
		}

		/* if the page is not in LRU list, put it in the LRU list */
		this->LRU_cache->addToList(page_index, this->all_page_map[page_index]);

		/* read it? */
		int file_descriptor;
		if (whichTable == nullptr) {
			file_descriptor = open (this->tempFile.c_str (), O_CREAT | O_RDWR | O_SYNC, 0666);
		} else {
			file_descriptor = open (whichTable->getStorageLoc ().c_str (), O_CREAT | O_RDWR | O_SYNC, 0666);
		}
		lseek (file_descriptor, this->all_page_map[page_index]->getOffset() * this->pageSize, SEEK_SET);
		read (file_descriptor, this->all_page_map[page_index]->bytes, pageSize);
		close (file_descriptor);
	}
}

void MyDB_BufferManager :: moveOutLRUPage () {

	this->LRU_cache->popTail();

	cout << "ok after pop tail" << endl;

	
}

// Remove page logic
// If page is in allPages, erase key
// If page is in LRU list, killNode
void MyDB_BufferManager :: killPage (MyDB_TablePtr whichTable, long i) {
	pair <MyDB_TablePtr, long> page_index = make_pair(whichTable, i);

	/* if anonymous page, just kill it */
	if (whichTable == nullptr) {
		if (this->all_page_map.find(page_index) != this->all_page_map.end()) {
			// If we have this page in global map allPages
			this->all_page_map.erase(page_index);
		}

		if (this->LRU_cache->findNode(page_index) != nullptr) {
			// We have this key stored in LRU hash map 
			this->LRU_cache->deleteNode(this->LRU_cache->findNode(page_index));
		}
	} else if (this->LRU_cache->findNode(page_index) != nullptr) {
		if (this->LRU_cache->findNode(page_index)->getPage()->isPinned() == true) {
			cout << "unpin the page" << endl;
			this->LRU_cache->findNode(page_index)->getPage()->setPin(false);
		}
	} else {
		this->all_page_map.erase(page_index);
	}

	
}

// Create a new bufferManager
MyDB_BufferManager :: MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile) {
	this -> pageSize = pageSize;
	this -> numPages = numPages;
	this -> tempFile = tempFile;
	this->anonymous_page_counter = 0;

	// Allocate memory
	for (size_t i = 0; i < numPages; i++) {
		this->availableRam.push_back((void *) malloc (pageSize));
	}	

	// Create a new LRU cache table
	this->LRU_cache = new LRUCache(numPages, *this);

}

MyDB_BufferManager :: ~MyDB_BufferManager () {
	// delete all pages in the all_page_map
	for (auto page_map : this->all_page_map) {
		MyDB_PagePtr page = page_map.second;
		// check if the bytes is written, if so, write bute back to disk
		if (page->bytes != nullptr) {
			if (page->isDirty() == true) {
				int file_descriptor;
				if (page->getParentTable() == nullptr) {
					file_descriptor = open (this->tempFile.c_str (), O_CREAT | O_RDWR | O_SYNC, 0666);
				} else {
					file_descriptor = open (page->getParentTable()->getStorageLoc ().c_str (), O_CREAT | O_RDWR | O_SYNC, 0666);
				}
				lseek (file_descriptor, page->getOffset() * pageSize, SEEK_SET);
				write (file_descriptor, page->bytes, pageSize);
				close (file_descriptor);
			}

			free (page->bytes);
			page->setBytes(nullptr);
		}
	}

	// free all memory of ram
	for (auto ram : this->availableRam) {
		free (ram);
	}

	// delete the temporary file
	unlink (this->tempFile.c_str());
}
	
#endif


