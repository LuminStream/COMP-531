
#ifndef REG_SELECTION_C                                        
#define REG_SELECTION_C

#include "RegularSelection.h"

RegularSelection :: RegularSelection (MyDB_TableReaderWriterPtr input, MyDB_TableReaderWriterPtr output,
                string selectionPredicate, vector <string> projections) {
    // Initialize all private fields
    this -> input = input;
    this -> output = output;
    // Only records predicate evaluates to true should be appended to the output
    this -> selectionPredicate = selectionPredicate;
    // Projections contains all computations that are performed to create the output record
    this -> projections = projections;

}

void RegularSelection :: run () {
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

    // Now we iterate through the input record
    MyDB_RecordIteratorAltPtr myIter = input -> getIteratorAlt();

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
