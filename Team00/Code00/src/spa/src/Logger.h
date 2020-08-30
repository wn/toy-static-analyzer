//
// Created by Ang Wei Neng on 30/8/20.
//

#ifndef AUTOTESTER_LOGGER_H
#define AUTOTESTER_LOGGER_H

// Global Helpers
#define DEBUG 1
void log(const std::string& s) {
    DEBUG&& std::cout << "LOG: " << s << '\n';
}
void log(int i) {
    log(std::to_string(i));
}

#endif // AUTOTESTER_LOGGER_H
