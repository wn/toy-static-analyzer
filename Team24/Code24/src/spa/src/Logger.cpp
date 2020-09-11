#include "Logger.h"

#include <iostream>
#include <string>

// Logs a value with a newline suffix
void logLine(const std::string& s) {
    DEBUG&& std::cout << s << std::endl;
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
