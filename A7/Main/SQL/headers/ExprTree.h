
#ifndef SQL_EXPRESSIONS
#define SQL_EXPRESSIONS

#include "MyDB_AttType.h"
#include "MyDB_Catalog.h"
#include <string>
#include <vector>

// create a smart pointer for database tables
using namespace std;
class ExprTree;
typedef shared_ptr<ExprTree> ExprTreePtr;

// this class encapsules a parsed SQL expression (such as "this.that > 34.5 AND 4 = 5")

// class ExprTree is a pure virtual class... the various classes that implement it are below
class ExprTree {

public:
    virtual string toString() = 0;
    virtual ~ExprTree() {};
    // Return variable type
    virtual string getType() = 0;
    // Return attTypePtr
    virtual pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) = 0;

};

class BoolLiteral : public ExprTree {

private:
    bool myVal;
public:

    BoolLiteral(bool fromMe) {
        myVal = fromMe;
    }

    string toString() {
        if (myVal) {
            return "bool[true]";
        } else {
            return "bool[false]";
        }
    }

    string getType() {
        return "BOOL";
    }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) {
        return make_pair("", make_shared<MyDB_BoolAttType>());
    }
};

class DoubleLiteral : public ExprTree {

private:
    double myVal;
public:

    DoubleLiteral(double fromMe) {
        myVal = fromMe;
    }

    string toString() {
        return "double[" + to_string(myVal) + "]";
    }

    ~DoubleLiteral() {}

    string getType() {
        return "DOUBLE";
    }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) {
        return make_pair("", make_shared<MyDB_DoubleAttType>());
    }
};

// this implement class ExprTree
class IntLiteral : public ExprTree {

private:
    int myVal;
public:

    IntLiteral(int fromMe) {
        myVal = fromMe;
    }

    string toString() {
        return "int[" + to_string(myVal) + "]";
    }

    ~IntLiteral() {}

    string getType() {
        return "INT";
    }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) {
        return make_pair("", make_shared<MyDB_IntAttType>());
    }
};

class StringLiteral : public ExprTree {

private:
    string myVal;
public:

    StringLiteral(char *fromMe) {
        fromMe[strlen(fromMe) - 1] = 0;
        myVal = string(fromMe + 1);
    }

    string toString() {
        return "string[" + myVal + "]";
    }

    ~StringLiteral() {}

    string getType() {
        return "STRING";
    }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) {
        return make_pair("", make_shared<MyDB_StringAttType>());
    }
};

class Identifier : public ExprTree {

private:
    string tableName;
    string attName;
    string type;
public:

    Identifier(char *tableNameIn, char *attNameIn) {
        tableName = string(tableNameIn);
        attName = string(attNameIn);
    }

    string toString() {
        return "[" + tableName + "_" + attName + "]";
    }

    ~Identifier() {}

    string getType() {
        return "IDENTIFIER";
    }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) {
        string attType;
        bool flag = myCatalog->getString(tableName + "." + attName + ".type", attType);
        if(attType.compare("int") == 0){
            return make_pair("[" + attName + "]", make_shared<MyDB_IntAttType>());
        }
        else if(attType.compare("string") == 0){
            return make_pair("[" + attName + "]", make_shared<MyDB_StringAttType>());
        }
        else{
            return make_pair("[" + attName + "]", make_shared<MyDB_BoolAttType>());
        }
    }
};

class MinusOp : public ExprTree {

private:

    ExprTreePtr lhs;
    ExprTreePtr rhs;
    string type;

public:

    MinusOp(ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
        lhs = lhsIn;
        rhs = rhsIn;
    }

    string toString() {
        return "- (" + lhs->toString() + ", " + rhs->toString() + ")";
    }

    ~MinusOp() {}

    string getType() {
        return "INT";
    }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName){
        return make_pair("", make_shared<MyDB_IntAttType>());
    }    
};

class PlusOp : public ExprTree {

private:

    ExprTreePtr lhs;
    ExprTreePtr rhs;
    string type;

public:

    PlusOp(ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
        lhs = lhsIn;
        rhs = rhsIn;
        type = "NUMERIC";
    }

    string toString() {
        return "+ (" + lhs->toString() + ", " + rhs->toString() + ")";
    }

    ~PlusOp() {}

    string getType() {
        return "STRING";
    }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) {
        return make_pair("", make_shared<MyDB_StringAttType>());
    }
};

class TimesOp : public ExprTree {

private:

    ExprTreePtr lhs;
    ExprTreePtr rhs;

public:

    TimesOp(ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
        lhs = lhsIn;
        rhs = rhsIn;
    }

    string toString() {
        return "* (" + lhs->toString() + ", " + rhs->toString() + ")";
    }

    ~TimesOp() {}

    string getType() {
        return "INT";
    }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) {
        return make_pair("", make_shared<MyDB_IntAttType>());
    }
};

class DivideOp : public ExprTree {

private:

    ExprTreePtr lhs;
    ExprTreePtr rhs;

public:

    DivideOp(ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
        lhs = lhsIn;
        rhs = rhsIn;
    }

    string toString() {
        return "/ (" + lhs->toString() + ", " + rhs->toString() + ")";
    }

    ~DivideOp() {}

    string getType() {}

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) {}
};

class GtOp : public ExprTree {

private:

    ExprTreePtr lhs;
    ExprTreePtr rhs;

public:

    GtOp(ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
        lhs = lhsIn;
        rhs = rhsIn;
    }

    string toString() {
        return "> (" + lhs->toString() + ", " + rhs->toString() + ")";
    }

    ~GtOp() {}

    string getType() { }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) { }
};

class LtOp : public ExprTree {

private:

    ExprTreePtr lhs;
    ExprTreePtr rhs;

public:

    LtOp(ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
        lhs = lhsIn;
        rhs = rhsIn;
    }

    string toString() {
        return "< (" + lhs->toString() + ", " + rhs->toString() + ")";
    }

    ~LtOp() {}

    string getType() { }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) { }
};

class NeqOp : public ExprTree {

private:

    ExprTreePtr lhs;
    ExprTreePtr rhs;

public:

    NeqOp(ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
        lhs = lhsIn;
        rhs = rhsIn;
    }

    string toString() {
        return "!= (" + lhs->toString() + ", " + rhs->toString() + ")";
    }

    ~NeqOp() {}

    string getType() { }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) { }
};

class OrOp : public ExprTree {

private:

    ExprTreePtr lhs;
    ExprTreePtr rhs;

public:

    OrOp(ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
        lhs = lhsIn;
        rhs = rhsIn;
    }

    string toString() {
        return "|| (" + lhs->toString() + ", " + rhs->toString() + ")";
    }

    ~OrOp() {}

    string getType() { }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) { }
};

class EqOp : public ExprTree {

private:

    ExprTreePtr lhs;
    ExprTreePtr rhs;

public:

    EqOp(ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
        lhs = lhsIn;
        rhs = rhsIn;
    }

    string toString() {
        return "== (" + lhs->toString() + ", " + rhs->toString() + ")";
    }

    ~EqOp() {}

    string getType() { }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) { }
};

class NotOp : public ExprTree {

private:

    ExprTreePtr child;

public:

    NotOp(ExprTreePtr childIn) {
        child = childIn;
    }

    string toString() {
        return "!(" + child->toString() + ")";
    }

    ~NotOp() {}

    string getType() { }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) { }
};

class SumOp : public ExprTree {

private:

    ExprTreePtr child;

public:

    SumOp(ExprTreePtr childIn) {
        child = childIn;
    }

    string toString() {
        return "sum(" + child->toString() + ")";
    }

    ~SumOp() {}

    string getType() {
        return "SUM";
    }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) {
        if (child->getType().compare("INT") == 0) {
            return make_pair("", make_shared<MyDB_IntAttType>());
        } 
        else {
            return child->getAttPair(myCatalog, tableName);
        }
    }
};

class AvgOp : public ExprTree {

private:

    ExprTreePtr child;

public:

    AvgOp(ExprTreePtr childIn) {
        child = childIn;
    }

    string toString() {
        return "avg(" + child->toString() + ")";
    }

    ~AvgOp() {}

    string getType() {
        return "AVG";
    }

    pair<string, MyDB_AttTypePtr> getAttPair(MyDB_CatalogPtr myCatalog, string tableName) {
        return make_pair("", make_shared<MyDB_DoubleAttType>());
    }
};

#endif
