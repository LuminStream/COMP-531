
#ifndef SORT_C
#define SORT_C

#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableRecIterator.h"
#include "MyDB_TableRecIteratorAlt.h"
#include "MyDB_TableReaderWriter.h"
#include "Sorting.h"

using namespace std;

// comparator to help sort record iter
class IteratorComparator {

public:

    IteratorComparator(function<bool()> comparatorIn, MyDB_RecordPtr lhsIn, MyDB_RecordPtr rhsIn) {
        comparator = comparatorIn;
        lhs = lhsIn;
        rhs = rhsIn;
    }

    bool operator()(const MyDB_RecordIteratorAltPtr leftIter, const MyDB_RecordIteratorAltPtr rightIter) const {
        leftIter->getCurrent(lhs);
        rightIter->getCurrent(rhs);
        return !comparator();
    }

private:
    function<bool()> comparator;
    MyDB_RecordPtr lhs;
    MyDB_RecordPtr rhs;
};

void sort (int runSize, MyDB_TableReaderWriter &sortMe, MyDB_TableReaderWriter &sortIntoMe, function <bool ()> comparator, MyDB_RecordPtr lhs,  MyDB_RecordPtr rhs) {
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

	mergeIntoFile(sortIntoMe, recordIter, comparator, lhs, rhs);
} 

void _append(MyDB_PageReaderWriter &myPageRW, vector <MyDB_PageReaderWriter> &appendIntoMe, MyDB_RecordPtr myRecord, MyDB_BufferManagerPtr parent) {
	if (!myPageRW.append(myRecord)) {
		appendIntoMe.push_back(myPageRW);
		
		// if current page reader/writer has no space, create a new reader/writer
		MyDB_PageReaderWriter newPageRW(*parent);
		newPageRW.append(myRecord);
		myPageRW = newPageRW;
	}
}

// helper function.  Gets two iterators, leftIter and rightIter.  It is assumed that these are iterators over
// sorted lists of records.  This function then merges all of those records into a list of anonymous pages,
// and returns the list of anonymous pages to the caller.  The resulting list of anonymous pages is sorted.
// Comparisons are performed using comparator, lhs, rhs
vector <MyDB_PageReaderWriter> mergeIntoList (MyDB_BufferManagerPtr parent, MyDB_RecordIteratorAltPtr leftIter, MyDB_RecordIteratorAltPtr rightIter, function <bool ()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {
	MyDB_PageReaderWriter myPage(*parent);
	vector <MyDB_PageReaderWriter> return_list;
	string status = "";
	while (true) {
		leftIter->getCurrent(lhs);
		rightIter->getCurrent(rhs);

		// if left < right
		if (comparator()) {
			_append(myPage, return_list, lhs, parent); 

			// if there is no next record in left, append all records in right
			if (!leftIter->advance()) {
                _append (myPage, return_list, rhs, parent);
				
                while (rightIter->advance ()) {
                    rightIter->getCurrent (rhs);
                    _append (myPage, return_list, rhs, parent);
                }

				break;
			}
		} else { // if left >= right
			_append(myPage, return_list, rhs, parent);

			// if there is no next record in right, append all records in left
			if (!rightIter->advance()) {
                _append (myPage, return_list, lhs, parent);
				
                while (leftIter->advance ()) {
                    leftIter->getCurrent (lhs);
                    _append (myPage, return_list, lhs, parent);
                }

				break;
			}
		}
	}

	return_list.push_back(myPage);
	return return_list;
}

// accepts a list of iterators called mergeUs.  It is assumed that these are all iterators over sorted lists
// of records.  This function then merges all of those records and appends them to the file sortIntoMe.  If
// all of the iterators are over sorted lists of records, then all of the records appended onto the end of
// sortIntoMe will be sorted.  Comparisons are performed using comparator, lhs, rhs
void mergeIntoFile (MyDB_TableReaderWriter &sortIntoMe, vector <MyDB_RecordIteratorAltPtr> &mergeUs, function <bool ()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {
	IteratorComparator comp (comparator, lhs, rhs);
	priority_queue <MyDB_RecordIteratorAltPtr, vector <MyDB_RecordIteratorAltPtr>, IteratorComparator> pq(comp);

	for (MyDB_RecordIteratorAltPtr iter : mergeUs) {
		if (iter->advance()) {
			pq.push(iter);
		}
	}

	// put each record in pq to the sortIntoMe table
	while (pq.size() > 0) {
		MyDB_RecordIteratorAltPtr iter = pq.top();
		iter->getCurrent(lhs);
		sortIntoMe.append(lhs);

		pq.pop();

		// if iter still has next record, we put it into the pq again
		// but its position in the pq might change.
		if (iter->advance()) {
			pq.push(iter);
		}
	}
}
	
#endif
