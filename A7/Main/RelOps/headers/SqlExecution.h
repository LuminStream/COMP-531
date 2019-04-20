
#ifndef SQL_EXE_H                                
#define SQL_EXE_H

#include "Aggregate.h"
#include "BPlusSelection.h"
#include "RegularSelection.h"
#include "ScanJoin.h"
#include "SortMergeJoin.h"
#include "ParserTypes.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_Catalog.h"

class SqlExecution{

public:
    SqlExecution(SQLStatement *mySql, MyDB_CatalogPtr Catalog, map<string, MyDB_TableReaderWriterPtr> inputTable,
           MyDB_BufferManagerPtr bufferMgr);

    void run();

private:
    SQLStatement *mySql;
    MyDB_CatalogPtr Catalog;
    map<string, MyDB_TableReaderWriterPtr> myInputTable;
    MyDB_BufferManagerPtr bufferMgr;
};

#endif
