#ifndef PAGE_H
#define PAGE_H

#include "../../Catalog/headers/MyDB_Table.h"

/*  smart pointer for my pages  */
class MyDB_Page;
typedef shared_ptr<MyDB_Page> MyDB_PagePtr;

/*  forward declaration of the manager class to 
    support MyDB_BufferManager type property */
class MyDB_BufferManager;

class MyDB_Page {
    public:
        /* constructor */
        MyDB_Page(MyDB_BufferManager &parentManager, MyDB_TablePtr parentTable, size_t offset);

        /* destructor */
        ~MyDB_Page();

        /* add a reference to this page */
        void addReference();

        /* reduce a reference to this page */
        void reduceReference();

        /* access the raw bytes in this page... if the page is not currently 
        in the buffer, then the contents of the page are loaded from 
        secondary storage.  */
        void *getBytes();

        /* let the page know we have written to the bytes */
        void wroteBytes();

        bool isDirty();

        bool isPinned();

        void setBytes(void *ram);
        
        void setPin(bool condition);

        void setDirty(bool dirty);

        size_t getOffset();

        pair<MyDB_TablePtr, size_t> getPageIndex();

        MyDB_TablePtr getParentTable();

        long getReferenceCounter();

        void* returnBytes();

    private:
        friend class MyDB_BufferManager;
        friend class LRUCache;


        /* the buffer manager managing this page */
        MyDB_BufferManager &parentManager;

        /* the table containing this page */
        MyDB_TablePtr parentTable;

        /* the offset of page in its table */
        long offset;

        /* the number of references to this page */
        long reference_counter;

        /* dirty status, whether the bytes is written */
        bool dirty;

        /* pointer to the page's bytes address use 
        void pointer to match any other type of data */
        void *bytes;

        /* pinned status, whether the page is pinned in RAM */
        bool pinned;

};

#endif