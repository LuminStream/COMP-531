#ifndef BPLUS_C
#define BPLUS_C

#include "../../Record/headers/MyDB_INRecord.h"
#include "../headers/MyDB_BPlusTreeReaderWriter.h"
#include "../headers/MyDB_PageReaderWriter.h"
#include "../headers/MyDB_PageListIteratorSelfSortingAlt.h"
#include "../headers/RecordComparator.h"

MyDB_BPlusTreeReaderWriter :: MyDB_BPlusTreeReaderWriter (string orderOnAttName, MyDB_TablePtr forMe, 
	MyDB_BufferManagerPtr myBuffer) : MyDB_TableReaderWriter (forMe, myBuffer) {

	// find the ordering attribute
	auto res = forMe->getSchema ()->getAttByName (orderOnAttName);

	// remember information about the ordering attribute
	orderingAttType = res.second;
	whichAttIsOrdering = res.first;

	// and the root location
	rootLocation = getTable ()->getRootLocation ();
}

MyDB_RecordIteratorAltPtr MyDB_BPlusTreeReaderWriter :: getSortedRangeIteratorAlt (MyDB_AttValPtr low, MyDB_AttValPtr high) {
	// page list to iterate
	vector<MyDB_PageReaderWriter> page_list;
	this->discoverPages (this->rootLocation, page_list, low, high);

	// set conparator
	MyDB_RecordPtr lhs = getEmptyRecord();
	MyDB_RecordPtr rhs = getEmptyRecord();
	function <bool ()> comparator = buildComparator(lhs, rhs);

	MyDB_RecordPtr myRec = getEmptyRecord();
	MyDB_INRecordPtr low_record = getINRecord();
	low_record->setKey(low);
	MyDB_INRecordPtr high_record = getINRecord();
	high_record->setKey(high);

	function <bool ()> low_comparator = buildComparator(myRec, low_record);
	function <bool ()> high_comparator = buildComparator(high_record, myRec);

	return make_shared<MyDB_PageListIteratorSelfSortingAlt>(page_list, lhs, rhs, comparator, myRec, low_comparator, high_comparator, true);

}

MyDB_RecordIteratorAltPtr MyDB_BPlusTreeReaderWriter :: getRangeIteratorAlt (MyDB_AttValPtr low, MyDB_AttValPtr high) {
	// page list to iterate
	vector<MyDB_PageReaderWriter> page_list;
	this->discoverPages (this->rootLocation, page_list, low, high);

	// set conparator
	MyDB_RecordPtr lhs = getEmptyRecord();
	MyDB_RecordPtr rhs = getEmptyRecord();
	function <bool ()> comparator = buildComparator(lhs, rhs);

	MyDB_RecordPtr myRec = getEmptyRecord();
	MyDB_INRecordPtr low_record = getINRecord();
	low_record->setKey(low);
	MyDB_INRecordPtr high_record = getINRecord();
	high_record->setKey(high);

	function <bool ()> low_comparator = buildComparator(myRec, low_record);
	function <bool ()> high_comparator = buildComparator(high_record, myRec);

	return make_shared<MyDB_PageListIteratorSelfSortingAlt>(page_list, lhs, rhs, comparator, myRec, low_comparator, high_comparator, false);
}

// gets a list of pages that might have data for an iterator... any leaf page that can possibly
// have a value in the range [low, high], inclusive should be returned from this call
bool MyDB_BPlusTreeReaderWriter :: discoverPages (int whichPage, vector <MyDB_PageReaderWriter> &list, MyDB_AttValPtr low, MyDB_AttValPtr high) {
	// the page to discover
	MyDB_PageReaderWriter my_page = (*this)[whichPage];

	// if it is a regular page, put it in the list and return true
	if (my_page.getType() == MyDB_PageType :: RegularPage) {
		list.push_back(my_page);
		return true;
	// else we put all possible pages within the range [low, high] to the list
	} else {
		MyDB_RecordIteratorAltPtr iter = my_page.getIteratorAlt();

		MyDB_INRecordPtr myRec = getINRecord();
		MyDB_INRecordPtr low_record = getINRecord();
		low_record->setKey(low);
		MyDB_INRecordPtr high_record = getINRecord();
		high_record->setKey(high);

		function <bool ()> low_comparator = buildComparator(myRec, low_record);
		function <bool ()> high_comparator = buildComparator(high_record, myRec);

		bool reach_low_bound = false;
		while (iter->advance()) {
			iter->getCurrent(myRec);

			// reach the low bound
			if (!low_comparator()) {
				reach_low_bound = true;
			}

			if (reach_low_bound) {
				this->discoverPages(myRec->getPtr(), list, low, high);
			}

			// if we reach the high bound, jump out of the loop
			if (high_comparator()) {
				break;
			}
		}
		return false;	
	}

	return false;
}


// append a record to the B+-Tree
void MyDB_BPlusTreeReaderWriter :: append (MyDB_RecordPtr appendMe) {
	// cout << "called by who?" << endl;
	// first we check if there is a valid B+-tree, a valid B+-tree has at least two pages.
	// if it is not a valid B+-tree, we just add page to its root
	if (getNumPages() < 2) {
		// cout << "not valid B+-tree" << endl;
		MyDB_PageReaderWriter root = (*this)[0]; // at the begining, the root location is -1
		this->rootLocation = 0;
		getTable()->setRootLocation(0);
		root.clear();
		root.setType(MyDB_PageType :: DirectoryPage);

		MyDB_INRecordPtr internalRecordNode = getINRecord();
		internalRecordNode->setPtr(1);
		getTable()->setLastPage(1);
		root.append(internalRecordNode); // pointer to the page
		
		MyDB_PageReaderWriter leaf = (*this)[1];
		leaf.setType(MyDB_PageType :: RegularPage);
		leaf.clear();
		leaf.append(appendMe);

	} else {
		// cout << "insert to a valid B+-tree" << endl;
		auto rec = this->append(this->rootLocation, appendMe);

		// if the return value is nullptr, there is enough space to contain the appendMe record
		if (rec == nullptr) {
			return ;
		// else we need to add new space(page) to contain the record
		} else {
			// create new page to contain the record 
			int new_rootLocation = getTable()->lastPage() + 1;
			getTable()->setLastPage(new_rootLocation);
			
			// insert the record and pointer which points to corresponding page
			MyDB_PageReaderWriter new_root = (*this)[new_rootLocation];
			new_root.clear();
			new_root.setType(MyDB_PageType :: DirectoryPage);
			new_root.append(rec); // record

			MyDB_INRecordPtr new_recordNode = getINRecord();
			new_recordNode->setPtr(this->rootLocation); // debug: not the new root location
			new_root.append(new_recordNode); // pointer to the page

			this->rootLocation = new_rootLocation;
			getTable()->setRootLocation(this->rootLocation);
		}
	}
}

// splits the given page (plus the record andMe) around the median.  A MyDB_INRecordPtr is returned that
// points to the record holding the (key, ptr) pair pointing to the new page.  Note that the new page
// always holds the lower 1/2 of the records on the page; the upper 1/2 remains in the original page
MyDB_RecordPtr MyDB_BPlusTreeReaderWriter::split(MyDB_PageReaderWriter splitMe, MyDB_RecordPtr addMe) { 
	// create two new pages
    int new_pageLocation = getTable()->lastPage() + 1;
    getTable()->setLastPage(new_pageLocation);
    MyDB_PageReaderWriter new_page1_for_split = (*this)[new_pageLocation];
    new_page1_for_split.clear();

    int new_pageLocation_ = getTable()->lastPage() + 1;
    getTable()->setLastPage(new_pageLocation_);
    MyDB_PageReaderWriter new_page2_for_split = (*this)[new_pageLocation_];
    new_page2_for_split.clear();

	MyDB_INRecordPtr return_ptr = getINRecord(); // the return value
	
    // if the splitMe is a regular page
    if (splitMe.getType() == MyDB_PageType::RegularPage) {
        new_page1_for_split.setType(MyDB_PageType::RegularPage);
        new_page2_for_split.setType(MyDB_PageType::RegularPage);

        // sort the records in splitMe
        MyDB_RecordPtr comp_rec1 = getEmptyRecord();
        MyDB_RecordPtr comp_rec2 = getEmptyRecord();
        function<bool()> comparator = buildComparator(comp_rec1, comp_rec2);
        splitMe.sortInPlace(comparator, comp_rec1, comp_rec2);

		// calculate the number of records in splitMe
        MyDB_RecordIteratorAltPtr iter = splitMe.getIteratorAlt();
        int num_of_records = 0;
        MyDB_RecordPtr temp_rec = getEmptyRecord();
        while (iter->advance()) {
            iter->getCurrent(temp_rec);
            num_of_records ++;
        }

		// split the records into two pages
        int mid = num_of_records / 2 - 1, counter = 0;
        iter = splitMe.getIteratorAlt();
        temp_rec = getEmptyRecord();
        while (iter->advance()) {
            iter->getCurrent(temp_rec);
            if (counter == mid){
				new_page1_for_split.append(temp_rec);
                return_ptr->setPtr(new_pageLocation);
                return_ptr->setKey(getKey(temp_rec));
            } else if (counter < mid) {
				new_page1_for_split.append(temp_rec);
			} else {
				new_page2_for_split.append(temp_rec);
			}

            counter ++;
        }

		// find the position to insert the addMe record.
        function<bool()> comparator_for_addme = buildComparator(addMe, return_ptr);
        if (comparator_for_addme()) {
            new_page1_for_split.append(addMe);
            new_page1_for_split.sortInPlace(comparator, comp_rec1, comp_rec2);
        } else {
            new_page2_for_split.append(addMe);
            new_page2_for_split.sortInPlace(comparator, comp_rec1, comp_rec2);
        }

        // copy the higher half records in new page2 back to splitMe
        splitMe.clear();
        iter = new_page2_for_split.getIteratorAlt();
        temp_rec = getEmptyRecord();
        while (iter->advance()) {
            iter->getCurrent(temp_rec);
            splitMe.append(temp_rec);
        }
        new_page2_for_split.clear();

	// if the splitMe is not a leaf page, we should set it as a directory page
    } else {
		new_page1_for_split.setType(MyDB_PageType::DirectoryPage);
        new_page2_for_split.setType(MyDB_PageType::DirectoryPage);

        // sort the records in splitMe
        MyDB_RecordPtr comp_rec1 = getINRecord();
        MyDB_RecordPtr comp_rec2 = getINRecord();
        function<bool()> comparator = buildComparator(comp_rec1, comp_rec2);
        splitMe.sortInPlace(comparator, comp_rec1, comp_rec2);

		// calculate the number of records in splitMe
        MyDB_RecordIteratorAltPtr iter = splitMe.getIteratorAlt();
        int num_of_records = 0;
        MyDB_RecordPtr temp_rec = getINRecord();
        while (iter->advance()) {
            iter->getCurrent(temp_rec);
            num_of_records ++;
        }

		// split the records into two pages
        int mid = num_of_records / 2 - 1, counter = 0;
        iter = splitMe.getIteratorAlt();
        temp_rec = getINRecord();
        bool flag = true;
        while (iter->advance()) {
            iter->getCurrent(temp_rec);

            if (counter == mid){
				new_page1_for_split.append(temp_rec);
                return_ptr->setPtr(new_pageLocation);
                return_ptr->setKey(getKey(temp_rec));
            } else if (counter < mid) {
				new_page1_for_split.append(temp_rec);
			} else {
				new_page2_for_split.append(temp_rec);
			}

            counter ++;
        }

		// find the position to insert the addMe record.
        function<bool()> comparator_for_addme = buildComparator(addMe, return_ptr);
        if (comparator_for_addme()) {
            new_page1_for_split.append(addMe);
            new_page1_for_split.sortInPlace(comparator, comp_rec1, comp_rec2);
        } else {
            new_page2_for_split.append(addMe);
            new_page2_for_split.sortInPlace(comparator, comp_rec1, comp_rec2);
        }

        // copy the higher half records in new page2 back to splitMe
        splitMe.clear();
		splitMe.setType(MyDB_PageType::DirectoryPage);
        iter = new_page2_for_split.getIteratorAlt();
        temp_rec = getINRecord();
        while (iter->advance()) {
            iter->getCurrent(temp_rec);
            splitMe.append(temp_rec);
        }
        new_page2_for_split.clear();
    }

    return return_ptr;
}


// appends a record to the named page; if there is a split, then an MyDB_INRecordPtr is returned that
// points to the record holding the (key, ptr) pair pointing to the new page.  Note that the new page
// always holds the lower 1/2 of the records on the page; the upper 1/2 remains in the original page
MyDB_RecordPtr MyDB_BPlusTreeReaderWriter :: append (int whichPage, MyDB_RecordPtr appendMe) {
	// cout << "call another appened" << endl;
	MyDB_PageReaderWriter my_page = (*this)[whichPage];

	// if our page is a regular page, we directly put the page into it.
	if (my_page.getType() == MyDB_PageType :: RegularPage) {
		// if there is enough memory to cpontain the record, we return nullptr
		// else we should split the page
		if (my_page.append(appendMe) == true) {
			return nullptr;
		} else {
			return this->split(my_page, appendMe);
		}
	} else {
		MyDB_RecordIteratorAltPtr iter = my_page.getIteratorAlt();

		MyDB_INRecordPtr temp_rec = getINRecord();
		function <bool ()> comparator = buildComparator(appendMe, temp_rec);

		while (iter->advance()) {
			// cout << "enter the advance loop" << endl;
			iter->getCurrent(temp_rec);

			// if we find record that is larger than the appendMe record, 
			// we just recursively find the final insert position along the 
			// path of the B+-tree
			if (comparator()) {
				MyDB_RecordPtr rec = append(temp_rec->getPtr(), appendMe);

				// if the return value is not nullptr, there must be a split operation
				// so we should add the lower half of the records to our page
				if (rec != nullptr) {
					if (my_page.append(rec) == true) {
						MyDB_INRecordPtr temp_rec = getINRecord();
						function <bool ()> comparator = buildComparator(rec, temp_rec);
						my_page.sortInPlace(comparator, rec, temp_rec);
						return nullptr;
					}
					
					// if there is not enough space, continue splitting
					return this->split(my_page, rec);
				}
				return nullptr;
			}
		}
	}

	return nullptr;
}

MyDB_INRecordPtr MyDB_BPlusTreeReaderWriter :: getINRecord () {
	return make_shared <MyDB_INRecord> (orderingAttType->createAttMax ());
}

void MyDB_BPlusTreeReaderWriter :: printTree () {
	printHelper(this->rootLocation, 0);
}

void MyDB_BPlusTreeReaderWriter :: printHelper (int whichPage, int depth) {
	MyDB_PageReaderWriter my_page = (*this)[whichPage];
	MyDB_RecordIteratorAltPtr iter;

	// if the page is a leaf page, we just print the records in the page
	if (my_page.getType () == MyDB_PageType :: RegularPage) {
		MyDB_RecordPtr temp_rec = getEmptyRecord ();
		iter = my_page.getIteratorAlt ();
		while (iter->advance ()) {
			iter->getCurrent(temp_rec);
			for (int i = 0; i < depth; i++) {
				cout << "\t";
			}
			cout << temp_rec << "\n";
		}

	// if the page is a directory page, we recursively iterate its children node and print them.
	} else {
		MyDB_INRecordPtr temp_rec = getINRecord ();
		iter = my_page.getIteratorAlt ();
		while (iter->advance ()) {
			iter->getCurrent (temp_rec);
			printHelper (temp_rec->getPtr (), depth + 1);
			for (int i = 0; i < depth; i++) {
				cout << "\t";
			}
			cout << (MyDB_RecordPtr) temp_rec << "\n";
		}
	}
}

MyDB_AttValPtr MyDB_BPlusTreeReaderWriter :: getKey (MyDB_RecordPtr fromMe) {

	// in this case, got an IN record
	if (fromMe->getSchema () == nullptr) 
		return fromMe->getAtt (0)->getCopy ();

	// in this case, got a data record
	else 
		return fromMe->getAtt (whichAttIsOrdering)->getCopy ();
}

function <bool ()>  MyDB_BPlusTreeReaderWriter :: buildComparator (MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {

	MyDB_AttValPtr lhAtt, rhAtt;

	// in this case, the LHS is an IN record
	if (lhs->getSchema () == nullptr) {
		lhAtt = lhs->getAtt (0);	

	// here, it is a regular data record
	} else {
		lhAtt = lhs->getAtt (whichAttIsOrdering);
	}

	// in this case, the LHS is an IN record
	if (rhs->getSchema () == nullptr) {
		rhAtt = rhs->getAtt (0);	

	// here, it is a regular data record
	} else {
		rhAtt = rhs->getAtt (whichAttIsOrdering);
	}
	
	// now, build the comparison lambda and return
	if (orderingAttType->promotableToInt ()) {
		return [lhAtt, rhAtt] {return lhAtt->toInt () < rhAtt->toInt ();};
	} else if (orderingAttType->promotableToDouble ()) {
		return [lhAtt, rhAtt] {return lhAtt->toDouble () < rhAtt->toDouble ();};
	} else if (orderingAttType->promotableToString ()) {
		return [lhAtt, rhAtt] {return lhAtt->toString () < rhAtt->toString ();};
	} else {
		cout << "This is bad... cannot do anything with the >.\n";
		exit (1);
	}
}


#endif





