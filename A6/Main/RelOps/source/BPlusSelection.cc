
#ifndef BPLUS_SELECTION_C                                        
#define BPLUS_SELECTION_C

#include "BPlusSelection.h"

BPlusSelection :: BPlusSelection (MyDB_BPlusTreeReaderWriterPtr input, MyDB_TableReaderWriterPtr output,
                MyDB_AttValPtr low, MyDB_AttValPtr high,
                string selectionPredicate, vector <string> projections) {
    this -> input = input;
    this -> output = output;
    this -> low = low;
    this -> high = high;
    this -> selectionPredicate = selectionPredicate;
    this -> projections = projections;
}

void BPlusSelection :: run () {
    // Get the input record
    MyDB_RecordPtr inputRec = input -> getEmptyRecord();

    // Now get the predicate
    // The predicate must be true for record in order to be stored into ouput table
    func predicate = inputRec -> compileComputation(selectionPredicate);

    // Get the final set of computations that we will used to build the output record
    vector <func> finalComputations;
    for(string s : projections){
        finalComputations.push_back(inputRec -> compileComputation(s));
    }

    // Create a ouput record
    MyDB_RecordPtr outputRec = output -> getEmptyRecord();

    // Now we iterate through the B+ tree 
    MyDB_RecordIteratorAltPtr myIter = input -> getRangeIteratorAlt(low, high);

    while(myIter -> advance()){
        myIter -> getCurrent(inputRec);

        // Check to see if it is accept by the predicate
        if(predicate() -> toBool()){
            // run all computations
            int i = 0;
            for(auto &f : finalComputations){
                outputRec -> getAtt(i++) -> set(f());
            }

            // Now we add outputRec to the output table
            outputRec -> recordContentHasChanged();
            output -> append(outputRec);
        }
    }
}

#endif
