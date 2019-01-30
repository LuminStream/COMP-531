
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include "MyDB_PageHandle.h"

#include <iostream>
using namespace std;

void *MyDB_PageHandleBase :: getBytes () {
	return this->page->getBytes();
}

void MyDB_PageHandleBase :: wroteBytes () {
	this->page->wroteBytes();
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {
	this->page->reduceReference();
}

/* constructor, creating a new handle to a page */
MyDB_PageHandleBase :: MyDB_PageHandleBase (MyDB_PagePtr page) {
	this->page = page;
	/* add a fererence to this page */
	this->page->addReference();
}

MyDB_PagePtr MyDB_PageHandleBase :: getMyPage() {
	return this->page;
}

#endif

