
#ifndef BUFFER_MGR_H
#define BUFFER_MGR_H

#include "MyDB_PageHandle.h"
#include "MyDB_Table.h"
#include "MyDB_LRU.h"
#include <vector>
#include <map>
#include <list>

using namespace std;

class MyDB_BufferManager {

public:

	// THESE METHODS MUST APPEAR AND THE PROTOTYPES CANNOT CHANGE!

	// gets the i^th page in the table whichTable... note that if the page
	// is currently being used (that is, the page is current buffered) a handle 
	// to that already-buffered page should be returned
	MyDB_PageHandle getPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page that will no longer exist (1) after the buffer manager
	// has been destroyed, or (2) there are no more references to it anywhere in the
	// program.  Typically such a temporary page will be used as buffer memory.
	// since it is just a temp page, it is not associated with any particular 
	// table
	MyDB_PageHandle getPage ();

	// gets the i^th page in the table whichTable... the only difference 
	// between this method and getPage (whicTable, i) is that the page will be 
	// pinned in RAM; it cannot be written out to the file
	MyDB_PageHandle getPinnedPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page, like getPage (), except that this one is pinned
	MyDB_PageHandle getPinnedPage ();

	// un-pins the specified page
	void unpin (MyDB_PageHandle unpinMe);

	// creates an LRU buffer manager... params are as follows:
	// 1) the size of each page is pageSize 
	// 2) the number of pages managed by the buffer manager is numPages;
	// 3) temporary pages are written to the file tempFile
	MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile);
	
	// when the buffer manager is destroyed, all of the dirty pages need to be
	// written back to disk, any necessary data needs to be written to the catalog,
	// and any temporary files need to be deleted
	~MyDB_BufferManager ();

	// FEEL FREE TO ADD ADDITIONAL PUBLIC METHODS 

	// When called getByte, manage current page into list and apply LRU
	void managePage (MyDB_TablePtr whichTable, long i);

	// move a page ut of LRU list
	void moveOutLRUPage ();

	// kill a page if there is no reference to him
	void killPage (MyDB_TablePtr whichTable, long i);

	/* available ram in void type to accept any type of data */
	vector <void *> availableRam;
	
private:

	// YOUR STUFF HERE
	friend class LRUCache;
	
	/* all pages that are got recently */
	map<pair<MyDB_TablePtr, long>, MyDB_PagePtr> all_page_map;

	/* counter for anonymous pages */
	long anonymous_page_counter;

	/* LRU cache */
	LRUCache *LRU_cache;

	// the size of each page is pageSize
	size_t pageSize;

	// the number of pages managed by the buffer manager is numPages
	size_t numPages;
	
	// temporary pages are written to the file tempFile
	string tempFile;
};

#endif


