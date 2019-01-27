#ifndef MYDB_PAGE_H
#define MYDB_PAGE_H

#include <string>
using namespace std;

class Page {

public:
    // Initialize
    Page(string id, int table, long i){
        this -> id = id;
        this -> pageId.first = table;
        this -> pageId.second = i;
    }

    // Get value
    string getId(){
        return id;
    }

    // Get pageId 
    pair<int, long> getPair(){
        return pageId;
    }

private:
    // Id of the page
    string id;    

    // Pair of table id and long i
    pair<int, long> pageId;
};

#endif 