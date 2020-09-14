#ifndef AUTOTESTER_LOGGER_H
#define AUTOTESTER_LOGGER_H

#include "TNode.h"
#include <string>

// Logs a value with a newline suffix
void logLine(const std::string& s);

void logLine(const std::string& s, bool cond);

void logLine(int i);

void logLine();

// Logs a value with a space suffix instead of a newline suffix
void logWord(const std::string& s);

void logWord(int i);

void logTNode(const backend::TNode& node);

#endif // AUTOTESTER_LOGGER_H
