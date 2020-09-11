#ifndef AUTOTESTER_LOGGER_H
#define AUTOTESTER_LOGGER_H

#include <iostream>
#include <string>

// Global Helpers
#define DEBUG 1

// Logs a value with a newline suffix
void logLine(const std::string& s);

void logLine(int i);

// Logs a value with a space suffix instead of a newline suffix
void logWord(const std::string& s);

void logWord(int i);

#endif // AUTOTESTER_LOGGER_H
