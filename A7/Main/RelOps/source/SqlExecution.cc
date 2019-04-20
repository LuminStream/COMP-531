
#include "SqlExecution.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <set>
#include <time.h>
#include <stdio.h>
#include <string>

SqlExecution :: SqlExecution(SQLStatement *mySql, MyDB_CatalogPtr Catalog, map<string, MyDB_TableReaderWriterPtr> inputTable,
            MyDB_BufferManagerPtr bufferMgr){
    this -> mySql = mySql;
    this -> Catalog = Catalog;
    this -> myInputTable = inputTable;
    this -> bufferMgr = bufferMgr;
}

void SqlExecution :: run() {
    clock_t start_t, end_t;
    /* Create needed params for RelOps function

        RegularSelection (MyDB_TableReaderWriterPtr input, MyDB_TableReaderWriterPtr output,
		    string selectionPredicate, vector <string> projections); 

        Aggregate (MyDB_TableReaderWriterPtr input, MyDB_TableReaderWriterPtr output,
		    vector <pair <MyDB_AggType, string>> aggsToCompute,
		    vector <string> groupings, string selectionPredicate);
    */
    MyDB_TableReaderWriterPtr inputTable;
    MyDB_TableReaderWriterPtr outputTable;
    string selectionPredicate;
    vector<string> projections;
    vector<pair<MyDB_AggType, string>> aggsToCompute;
    vector<string> groupings;
    
    // Get the sfwquery object
    SFWQuery sfwQuery = mySql -> myQuery;
    // Get all fields of sfwquey
    vector<ExprTreePtr> valuesToSelect = sfwQuery.valuesToSelect;
    vector<pair<string, string>> tablesToProcess = sfwQuery.tablesToProcess;
    vector<ExprTreePtr> allDisjunctions = sfwQuery.allDisjunctions;
    vector<ExprTreePtr> groupingClauses = sfwQuery.groupingClauses;

    // First deal with table name and table alais
    string tableName = tablesToProcess[0].first;
    string tableAlias = tablesToProcess[0].second;

    // Rename the table att
    myInputTable[tableName] -> getTable() -> getSchema() -> setAtts(tableAlias);
    // Set input table
    // Load input table
    inputTable = myInputTable[tableName];
    inputTable -> loadFromTextFile(tableName + ".tbl");

    // Start timer
    start_t = clock();
    MyDB_SchemaPtr mySchemaOut = make_shared<MyDB_Schema>();

    // First append grouping value
    for(auto v : valuesToSelect){
        //cout << "type: " << v -> getType() << endl;
        if(v -> getType().compare("SUM") != 0 && v -> getType().compare("AVG") != 0){
            string v_string = v -> toString();
            projections.push_back(v_string);
            mySchemaOut -> appendAtt(v -> getAttPair(Catalog, tableName));
            groupings.push_back(v_string);
        }
    }

    for(auto v : valuesToSelect){
        if(v -> getType().compare("SUM") == 0 || v -> getType().compare("AVG") == 0){
            string v_string = v -> toString();
            projections.push_back(v_string);
            string attName = v_string.substr(3);
            mySchemaOut -> appendAtt(v -> getAttPair(Catalog, tableName));

            if(v -> getType().compare("SUM") == 0){
                aggsToCompute.push_back(make_pair(MyDB_AggType::SUM, attName));
            } 
            else if(v -> getType().compare("AVG") == 0){
                aggsToCompute.push_back(make_pair(MyDB_AggType::AVG, attName));
            }
        }
    }

    // Now deal with the where clause
    vector<string> allDisjunctionsInString;
    // First get the disjunction strings
    for(int i = 0; i < allDisjunctions.size(); i++){
        string disjunction = allDisjunctions[i] -> toString();
        allDisjunctionsInString.push_back(disjunction);
    }
    
    // We set predicate to something like "&& ( == ([l_suppkey], [r_suppkey]), == ([l_name], [r_name]))"
    if(allDisjunctionsInString.size() == 1){
        selectionPredicate = allDisjunctionsInString[0];
    }
    else{
        selectionPredicate = "&& (" + allDisjunctionsInString[0] + "," + allDisjunctionsInString[1] + ")";
        for(int i = 2; i < allDisjunctionsInString.size(); i++){
            selectionPredicate = "&& (" + selectionPredicate + "," + allDisjunctionsInString[i] + ")";
        }
    }

    // Create outputTable
    MyDB_TablePtr myTableOut = make_shared<MyDB_Table>("output", "output.bin", mySchemaOut);
    outputTable = make_shared<MyDB_TableReaderWriter>(myTableOut, bufferMgr);

    // Apply selection function by determin the agg types
    if (aggsToCompute.size() == 0) {
        RegularSelection regularSelection(inputTable, outputTable, selectionPredicate, projections);
        regularSelection.run();
    } else {
        Aggregate aggregate(inputTable, outputTable, aggsToCompute, groupings, selectionPredicate);
        aggregate.run();
    }

    // Start print out results
    MyDB_RecordPtr temp = outputTable -> getEmptyRecord();
    MyDB_RecordIteratorAltPtr myIter = outputTable -> getIteratorAlt();

    cout << endl;
    cout << "=============================" << endl;
    cout << "Print out top 30 results: " << endl;
    
    // Create number counters
    int counter = 0;
    int totalNum = 0;

    while(myIter -> advance()){
        myIter -> getCurrent(temp);
        stringstream output;
        output << temp;
        if(counter < 30){
            cout << output.str() << endl;
            counter++;
        }
        totalNum++;
    }
    // Print the total number of results
    cout << "Total number of results from the query is: " << totalNum << endl;
    // Stop the timer and print out the use time
    end_t = clock();
    double total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
    printf("Total running time is: %f second(s)\n", total_t);
    cout << "=============================" << endl;
    cout << endl;
}