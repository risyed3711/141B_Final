//
// Created by root on 5/24/24.
//

#ifndef LOGGER_H
#define LOGGER_H

#define LOG_ENABLED 1

#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <string>

// Define log levels
enum class LogLevel {
    None = 0,
    Error,
    Warning,
    Info,
    Debug,
    Trace,
};

class Logger {
private:
    static Logger* logger;
    LogLevel logLevel;
    std::ofstream fileStream;
    std::ostream* outputStream;
    Logger() : logLevel(LogLevel::Trace), fileStream("db.log"), outputStream(&std::cout) {}
    std::map<LogLevel, std::string> logEscape = {
            {LogLevel::None,  "\033[0m"},
            {LogLevel::Error, "\033[31m"},
            {LogLevel::Warning, "\033[33m"},
            {LogLevel::Debug, "\033[36m"},
            {LogLevel::Info, "\033[37m"},
            {LogLevel::Trace, "\033[90m"}
    };
public:

    static Logger& getInstance(){
        if (logger== nullptr){
            logger = new Logger();
        }
        return *logger;
    }

    ~Logger() {
        if (fileStream.is_open()) {
            fileStream.close();
        }
    }

    Logger& setStream(std::ostream& os) {
        outputStream = &os;
        return *this;
    }

    //template <typename T>
    static void log(LogLevel level, std::string message) {
        if (logger== nullptr){
            logger = new Logger();
        }
        if (LOG_ENABLED){
            if (level <= logger->logLevel) {
                std::stringstream ss;
                (ss << message);
                *logger->outputStream << logger->logEscape.at(level) << ss.str() << std::endl;
                *logger->outputStream << logger->logEscape.at(LogLevel::None);
            }
        }
    }
};




#endif