
#ifndef AGG_CC
#define AGG_CC

#include "MyDB_Record.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include "Aggregate.h"
#include <unordered_map>

using namespace std;

Aggregate :: Aggregate (MyDB_TableReaderWriterPtr input, MyDB_TableReaderWriterPtr output,
	            vector <pair <MyDB_AggType, string>> aggsToCompute,
                vector <string> groupings, string selectionPredicate) { 
    this -> input = input;
    this -> output = output;
    this -> aggsToCompute = aggsToCompute;
    this -> groupings = groupings;
    this -> selectionPredicate = selectionPredicate;
}

void Aggregate :: run () {
    // Get a input record
    MyDB_RecordPtr inputRec = input -> getEmptyRecord();

	// Get a output record
	MyDB_RecordPtr outputRec = output->getEmptyRecord ();

    // Get predicate from input
    func predicate = inputRec -> compileComputation(selectionPredicate);

    // Parameter aggsToCompute lists aggregates that should be computed
    vector <func> aggComputation;
    for (auto a : aggsToCompute){
        aggComputation.push_back(inputRec -> compileComputation(a.second));
    }

    // Paramter groupings lists all computations that need to group by
    vector <func> groupComputation;
    for(auto group : groupings){
        groupComputation.push_back(inputRec -> compileComputation(group));
    }
    
    // This is the hashmap we need to use
    unordered_map <size_t, vector <void *>> myHash;

    // Get number of groupings and aggs
    int groupNum = groupings.size();
    int aggNum = aggsToCompute.size();
    int totalNum = groupNum + aggNum;

    // Create a schema that can store all of the required aggregate and grouping attributes
    MyDB_SchemaPtr schemaOut = make_shared <MyDB_Schema>();
 
    for(int i = 0; i < groupNum + aggNum; i++){
        if(i < groupNum){
            schemaOut -> appendAtt(make_pair("Group" + to_string(i), output -> getTable() -> getSchema() -> getAtts()[i].second));
        }
        else{
            schemaOut -> appendAtt(make_pair("Agg" + to_string(i - groupNum), output -> getTable() -> getSchema() -> getAtts()[i].second));
        }
    }
    schemaOut -> appendAtt(make_pair("Cnt", make_shared<MyDB_IntAttType>()));
    
    // Now we get the schemaOut, then create a record that have this schema
    MyDB_RecordPtr aggRec = make_shared <MyDB_Record>(schemaOut);

    // Create schema for combined record
    MyDB_SchemaPtr combinedSchema = make_shared <MyDB_Schema>();
    for(auto &a : input -> getTable() -> getSchema() -> getAtts()){
        combinedSchema -> appendAtt(a);
    }
    for(auto &a : schemaOut -> getAtts()){
        combinedSchema -> appendAtt(a);
    }

    // Then get the record having the combined schema
    MyDB_RecordPtr combinedRec = make_shared <MyDB_Record>(combinedSchema);
    combinedRec -> buildFrom(inputRec, aggRec);

    // Now try to get the aggregates function for update record
    vector <func> aggList;
    vector <func> avgList;
    string functionStr = "";

	for(int i = 0; i < aggNum; i++){
        auto aggs = aggsToCompute[i];
        // We first get the sum value of atts
		if(aggs.first == MyDB_AggType :: sum || aggs.first == MyDB_AggType :: avg){
            functionStr = "+ (" + aggs.second + ", [" + schemaOut -> getAtts()[i + groupNum].first + "])";
			aggList.push_back(combinedRec -> compileComputation(functionStr));
            //cout << "agg function is: " << functionStr << endl;
		} 
        else if(aggs.first == MyDB_AggType :: cnt){
            functionStr = "+ (int[1], [" + schemaOut -> getAtts()[i + groupNum].first + "])";
			aggList.push_back(combinedRec -> compileComputation(functionStr));
            //cout << "agg function is: " << functionStr << endl;
		}

        // Consider the average function
		if(aggs.first == MyDB_AggType :: avg){
            functionStr = "/ ([" + schemaOut -> getAtts()[i + groupNum].first + "], [Cnt])";
			avgList.push_back(combinedRec -> compileComputation(functionStr));
		} 
        else{
            functionStr = "[" + schemaOut -> getAtts()[i + groupNum].first + "]";
			avgList.push_back(combinedRec -> compileComputation(functionStr));
		}
	}
	aggList.push_back(combinedRec -> compileComputation("+ (int[1], [Cnt])"));
    
    // To create a function that verifies group by match up
    func groupCheck;
    if(groupNum == 0){
        groupCheck = combinedRec -> compileComputation("bool[true]");	
    }
    else{
        // Use checkStr to composite the equality check function
        string checkStr;
        for(int i = 0; i < groupNum; i++){
            string groupStr = groupings[i];
		    string equalCheck = "== (" + groupStr + ", [" + schemaOut -> getAtts()[i].first + "])";
		    if (i == 0) {
			    checkStr = equalCheck;
		    } else {
			    checkStr = "&& (" + equalCheck + ", " + checkStr + ")";
		    }
	    }
	    groupCheck = combinedRec -> compileComputation(checkStr);
    }

    // Create a list of page to store all agg records
	vector <MyDB_PageReaderWriter> allPages;
    // Write agg records to this page
	MyDB_PageReaderWriter lastPage (true, *(input->getBufferMgr ()));
	allPages.push_back(lastPage);
    
    // Now we iterate the record to check if we have match up hash value in the hash table
	MyDB_RecordIteratorPtr recIter = input -> getIterator(inputRec);
	while(recIter -> hasNext()){

		recIter -> getNext();

		// see if it is accepted by the preicate
        // We need to make sure the input record is accepted by predicate
		if(predicate() -> toBool()){

		    // Hash the current record
		    size_t hashVal = 0;
		    for(auto &f : groupComputation){
			    hashVal ^= f ()->hash ();
		    }

		    // If there is a match, then get the list of matches
		    vector <void *> &potentialMatches = myHash[hashVal];
		    void *location = nullptr;

		    // Iterate though the potential matches, checking each of them
            for(int i = 0; i < potentialMatches.size(); i++){
                auto matchRecord = potentialMatches[i];
                // Get the agg value
			    aggRec -> fromBinary(matchRecord);

			    // If input record and agg value are match, we could perform agg function
			    if(groupCheck() -> toBool()){
                    location = matchRecord;
			        break;
                }
		    }

            // If we dont have a match
		    if(location == nullptr){
                // Set recrod with group and agg computation
                int index = 0;
                for(int i = 0; i < groupNum; i++){
                    auto f = groupComputation[i];
                    aggRec -> getAtt(index++) -> set(f());
                }
			    for(int j = 0; j < aggList.size (); j++){
				    aggRec -> getAtt(index++) -> set(make_shared <MyDB_IntAttVal> ());
			    }

                // Update the agg rec
                for(int i = 0; i < aggList.size(); i++){
                    auto f = aggList[i];
                    aggRec -> getAtt(groupNum + i) -> set(f());
                }

                aggRec -> recordContentHasChanged();
                // Get a new location
			    location = lastPage.appendAndReturnLocation(aggRec);
			    aggRec -> fromBinary(location);
                // Add new value to the hash value vector
			    myHash [hashVal].push_back(location);
		    } 
            else {
                // Update the agg rec
                for(int i = 0; i < aggList.size(); i++){
                    auto f = aggList[i];
                    aggRec -> getAtt(groupNum + i) -> set(f());
                }

		        // Then write to old location
		        aggRec -> recordContentHasChanged();
			    aggRec -> toBinary(location);
		    }
        }
	}

	// We pass through one more time for the avg situation
	MyDB_RecordIteratorAltPtr avgIter = getIteratorAlt(allPages);
	while(avgIter -> advance()){

		avgIter -> getCurrent(aggRec);
        
        int i = 0;
		// Perform grouping function
		for(i = 0; i < groupNum; i++){
			outputRec -> getAtt(i) -> set(aggRec -> getAtt(i));
		}

		// Perform avg function
		for(auto &f : avgList){
			outputRec -> getAtt(i++) -> set(f());
		}
		outputRec -> recordContentHasChanged();
		output -> append(outputRec);
	}
}

#endif

