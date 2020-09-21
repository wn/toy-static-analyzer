#ifndef TESTWRAPPER_H
#define TESTWRAPPER_H

#include "PKB.h"

#include <iostream>
#include <list>
#include <string>

// include your other headers here
#include "AbstractWrapper.h"

class TestWrapper : public AbstractWrapper {
  public:
    // default constructor
    TestWrapper();

    // destructor
    ~TestWrapper();

    // PKB to store information of SIMPLE program
    backend::PKB* pkb;

    // method for parsing the SIMPLE source
    virtual void parse(std::string filename);

    // method for evaluating a query
    virtual void evaluate(std::string query, std::list<std::string>& results);
};

#endif
