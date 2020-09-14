#include "Logger.h"

#include <iostream>
#include <string>

// Global Helpers
#define DEBUG 1

// Logs a value with a newline suffix
void logLine(const std::string& s) {
    DEBUG&& std::cout << s << std::endl;
}

void logLine(const std::string& s, bool cond) {
    cond&& DEBUG&& std::cout << s << std::endl;
}

void logLine() {
    logLine("");
}

void logLine(int i) {
    logLine(std::to_string(i));
}

// Logs a value with a space suffix instead of a newline suffic
void logWord(const std::string& s) {
    DEBUG&& std::cout << s << " ";
}

void logWord(int i) {
    logWord(std::to_string(i));
}

void logTNode(const backend::TNode& node) {
    DEBUG&& std::cout << node.toString() << std::endl;
}
