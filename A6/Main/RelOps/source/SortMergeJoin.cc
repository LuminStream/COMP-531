
#ifndef SORTMERGE_CC
#define SORTMERGE_CC

#include "Aggregate.h"
#include "MyDB_Record.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include "SortMergeJoin.h"
#include "Sorting.h"
#include <string>

SortMergeJoin :: SortMergeJoin (MyDB_TableReaderWriterPtr leftInput, MyDB_TableReaderWriterPtr rightInput,
                MyDB_TableReaderWriterPtr output, string finalSelectionPredicate, 
                vector <string> projections,
                pair <string, string> equalityCheck, string leftSelectionPredicate,
                string rightSelectionPredicate) {
    this -> leftInput = leftInput;
    this -> rightInput = rightInput;
    this -> output = output;
    this -> finalSelectionPredicate = finalSelectionPredicate;
    this -> projections = projections;
    this -> equalityCheck = equalityCheck;
    this -> leftSelectionPredicate = leftSelectionPredicate;
    this -> rightSelectionPredicate = rightSelectionPredicate;
}

void SortMergeJoin :: run () {
    // https://blog.csdn.net/jameshadoop/article/details/76861713
    // Set up the run size
    int runSize = leftInput -> getBufferMgr() -> numPages / 2;
	
    // We create a output record
    MyDB_RecordPtr outputRec = output -> getEmptyRecord();
	
	// Create left and right input record
    MyDB_RecordPtr leftInputRec = leftInput -> getEmptyRecord();
    MyDB_RecordPtr leftInputRec2 = leftInput -> getEmptyRecord();

    MyDB_RecordPtr rightInputRec = rightInput -> getEmptyRecord();
    MyDB_RecordPtr rightInputRec2 = rightInput -> getEmptyRecord();
    
	// Then create comparators for both input
    // Sort the left relation using equalityCheck.first
	// Sort the right relation using equalityCheck.second
	function<bool()> leftComparator = buildRecordComparator(leftInputRec, leftInputRec2, equalityCheck.first);
    function<bool()> rightComparator = buildRecordComparator(rightInputRec, rightInputRec2, equalityCheck.second);
	
	// Create sorted left and right records
	MyDB_RecordIteratorAltPtr sortedLeft = buildItertorOverSortedRuns(runSize, *leftInput, leftComparator, leftInputRec, leftInputRec2, leftSelectionPredicate);
    MyDB_RecordIteratorAltPtr sortedRight = buildItertorOverSortedRuns(runSize, *rightInput, rightComparator, rightInputRec, rightInputRec2, rightSelectionPredicate);

    // Get schema for combined record
    MyDB_SchemaPtr schemaOut = make_shared<MyDB_Schema>();
    for(auto &s : leftInput -> getTable() -> getSchema() -> getAtts()){
		schemaOut -> appendAtt(s);
	}       
    for(auto &s : rightInput -> getTable() -> getSchema() -> getAtts()){
		schemaOut -> appendAtt(s);
	}
        
    // Get combined record
    MyDB_RecordPtr combinedRec = make_shared<MyDB_Record>(schemaOut);
    combinedRec -> buildFrom(leftInputRec, rightInputRec);

    // Get predicate over combined record
    func predicate = combinedRec -> compileComputation(finalSelectionPredicate);

    // Get the list of finalcomputations
    vector<func> finalComputations;
    for(string s : projections){
        finalComputations.push_back(combinedRec -> compileComputation(s));
    }

    // By the reference, we get the compare function to compare sorted left and sorted right
    func areEqual = combinedRec -> compileComputation(" == (" + equalityCheck.first + ", " + equalityCheck.second + ")");
    func leftSmall = combinedRec -> compileComputation(" < (" + equalityCheck.first + ", " + equalityCheck.second + ")");
    func rightSmall = combinedRec -> compileComputation(" > (" + equalityCheck.first + ", " + equalityCheck.second + ")");
    
    // Create pages
    vector<MyDB_PageReaderWriter> allLeftPages;
    vector<MyDB_PageReaderWriter> allRightPages;
    MyDB_PageReaderWriter leftPage(true, *(leftInput -> getBufferMgr()));
    MyDB_PageReaderWriter rightPage(true, *(rightInput -> getBufferMgr()));

	// Create a new reverse left comparator
    function<bool()> leftComparator2 = buildRecordComparator(leftInputRec2, leftInputRec, equalityCheck.first);

    if(sortedLeft -> advance() && sortedRight -> advance()){
        while(true){
            // Set a flag
            bool flag = false;
			
            sortedLeft -> getCurrent(leftInputRec);
            sortedRight -> getCurrent(rightInputRec);

            // If left and right are not equal, check for the next value on both side
            if((leftSmall() -> toBool() && !sortedLeft -> advance()) || (rightSmall() -> toBool() && !sortedRight -> advance())){
                //flag = true;
                break;                    
            }
            // If sorted left and sorted right are equal, we could perform merge
			else if(areEqual()->toBool()){
                // Add left side and right side to the pages
                leftPage.clear();
                rightPage.clear();
                allLeftPages.clear();
                allRightPages.clear();
                allLeftPages.push_back(leftPage);
                allRightPages.push_back(rightPage);

                // Find left equal pages
                while(true){
                    sortedLeft -> getCurrent(leftInputRec2); 
                    // If it is the same
                    if(!leftComparator() && !leftComparator2()){
                        if(!leftPage.append(leftInputRec2)){
                            MyDB_PageReaderWriter newPage(true, *(leftInput -> getBufferMgr()));
                            leftPage = newPage;
                            allLeftPages.push_back(leftPage);
                            leftPage.append(leftInputRec2);
                        }
                    } 
                    else{
                        break;
                    }
                    if(!sortedLeft -> advance()){
                        flag = true;
                        break;
                    }
                }

                // Find right equal pages
                while(true){
                    // Are the same
                    if(areEqual() -> toBool()){
                        if (!rightPage.append(rightInputRec)){
                            MyDB_PageReaderWriter newPage(true, *(rightInput->getBufferMgr()));
                            rightPage = newPage;
                            allRightPages.push_back(rightPage);
                            rightPage.append(rightInputRec);
                        }

                        MyDB_RecordIteratorAltPtr myIterLeft;
                        myIterLeft = getIteratorAlt(allLeftPages);

                        // Check for the left and right match
                        while(myIterLeft -> advance()){
                            myIterLeft->getCurrent(leftInputRec);

                            // Check for the predicate
                            if(predicate() -> toBool()){
                                int i = 0;
                                for(auto &f : finalComputations){
                                    outputRec->getAtt(i++)->set(f());
                                }
                                // Add new record to the output table
                                outputRec -> recordContentHasChanged();
                                output -> append(outputRec);
                            }
                        }
                    } 
                    else{
                        break;
                    }
                    if(!sortedRight->advance()){
                        flag = true;
                        break;
                    }  
                    // Get to the next right page
                    sortedRight -> getCurrent(rightInputRec);
                }
            }
            if(flag){
                break;
            }       
        }
    }
}

#endif
