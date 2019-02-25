
#ifndef SORT_C
#define SORT_C

#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableRecIterator.h"
#include "MyDB_TableRecIteratorAlt.h"
#include "MyDB_TableReaderWriter.h"
#include "Sorting.h"

using namespace std;

void mergeIntoFile (MyDB_TableReaderWriter &sortMe, vector <MyDB_RecordIteratorAltPtr> &sortIntoMe, function <bool ()> comparatorIn, MyDB_RecordPtr lhsIn,  MyDB_RecordPtr rhsIn) {

	
}

vector <MyDB_PageReaderWriter> mergeIntoList (MyDB_BufferManagerPtr, MyDB_RecordIteratorAltPtr, MyDB_RecordIteratorAltPtr, function <bool ()>, 
	MyDB_RecordPtr, MyDB_RecordPtr) {return vector <MyDB_PageReaderWriter> (); } 
	
void sort (int runSize, MyDB_TableReaderWriter &sortMe, MyDB_TableReaderWriter &sortIntoMe, function <bool ()> comparatorIn, MyDB_RecordPtr lhsIn,  MyDB_RecordPtr rhsIn) {
	// runSize is the number of pages
	// sortMe whose contents are to be sorted
	// sortIntoMe are to be written into. The result of the sort should be append to the end of this file
	// function <bool ()> comparatorIn, MyDB_RecordPtr lhsIn,  MyDB_RecordPtr rhsIn are the comparator

	// Load a run of pages into RAM
	vector <vector <MyDB_PageReaderWriter>> pagesToSort;

	// This is the total number of pages in the current file
	int numPages = sortMe.getNumPages();

	// This is the list of all sorted records
	vector <MyDB_RecordIteratorAltPtr> recordIter;

	for (int i = 0; i < numPages; i++) {
		// Sort each individual page in the vector
		// MyDB_PageReaderWriter :: sort (function <bool ()> comparator, MyDB_RecordPtr lhs,  MyDB_RecordPtr rhs)
		// The return value is MyDB_PageReaderWriter obj with sorted records
		vector <MyDB_PageReaderWriter> individualPage;
		MyDB_PageReaderWriter currentPage = *sortMe[i].sort(comparator, lhs, rhs);
		individualPage.push_back(currentPage);
		pagesToSort.push_back(individualPage);

		// We need to sort allocated pages during current run 
		// When pagesToSort.size() = runSize
		// Or we are reaching the end of the page list
		if (pagesToSort.size() % runSize == 0 || i == numPages - 1) {

			// For each adjacent pair of pages
			// We need to get two last adjacent lists from pagesToSort
			while (pagesToSort.size() > 1) {
				// Get two toBeSorted vector from pagesToSort
				vector <MyDB_PageReaderWriter> list1 = pagesToSort.back(); 
				pagesToSort.pop_back();
				vector <MyDB_PageReaderWriter> list2 = pagesToSort.back();
				pagesToSort.pop_back();

				// Then merge the contents of two lists
				vector <MyDB_PageReaderWriter> mergedList = mergeIntoList(sortMe.getBufferMgr(), getIteratorAlt(list1), getIteratorAlt(list2), comparator, lhs, rhs);
				// And add to the last of the pagesToSort list
				pagesToSort.push_back(mergedList);
			}
			
			// Now we have a single fully sorted list
			recordIter.push_back(getIteratorAlt(pagesToSort[0]));
			pagesToSort.clear();
		}
	}
	
	//
	mergeIntoFile(sortIntoMe, recordIter, comparator, lhs, rhs);
} 

#endif
