#include "TestWrapper.h"

#include "Lexer.h"
#include "Logger.h"
#include "PKBImplementation.h"
#include "Parser.h"
#include "QueryEvaluator.h"
#include "QueryPreprocessor.h"

#include <fstream>
#include <iterator>
#include <sstream>
#include <sys/stat.h>

// implementation code of WrapperFactory - do NOT modify the next 5 lines
AbstractWrapper* WrapperFactory::wrapper = 0;
AbstractWrapper* WrapperFactory::createWrapper() {
    if (wrapper == 0) wrapper = new TestWrapper;
    return wrapper;
}
// Do not modify the following line
volatile bool AbstractWrapper::GlobalStop = false;

// a default constructor
TestWrapper::TestWrapper() {
    // create any objects here as instance variables of this class
    // as well as any initialization required for your spa program
}

// method for parsing the SIMPLE source
void TestWrapper::parse(std::string filename) {
    struct stat buffer;
    if (stat(filename.c_str(), &buffer) != 0) {
        logLine("File does not exist: " + filename);
        exit(1);
    }
    std::ifstream inputFileStream;
    std::cout << "Parsing SIMPLE source file: " + filename << std::endl;
    inputFileStream.open(filename);
    backend::TNode ast = backend::Parser(backend::lexer::tokenize(inputFileStream)).parse();
    pkb = backend::PKBImplementation(ast);
}

// method to evaluating a query
void TestWrapper::evaluate(std::string query, std::list<std::string>& results) {
    std::cout << "Query string: " << query << std::endl;
    //    for (auto i : pkb.getAllStatements())
    //        std::cout << i << "aaa" << std::endl;
    try {
        std::stringstream stream(query);
        std::vector<backend::lexer::Token> tokens = backend::lexer::tokenize(stream);
        qpbackend::Query queryStruct = querypreprocessor::parseTokens(tokens);
        std::cout << "Query struct: " << queryStruct.toString() << std::endl;
        qpbackend::queryevaluator::QueryEvaluator queryEvaluator(&pkb);
        std::vector<std::string> queryResults = queryEvaluator.evaluateQuery(queryStruct);
        std::copy(queryResults.begin(), queryResults.end(), std::back_inserter(results));
    } catch (const std::exception& e) {
        std::cout << "Invalid query" << std::endl;
        return;
    }
    // store the answers to the query in the results list (it is initially empty)
    // each result must be a string.
}
