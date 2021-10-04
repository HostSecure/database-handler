#pragma once
class QFile;

//!
//! \brief The LogHandler class
//! A very simple logger to log qWarning, qCritical, and qFatal messages to file
//! The name of each log file is based on the current time to make every log file unique
//!
class LogHandler
{
public:
    friend int main(int argc, char *argv[]);

private:
    LogHandler();
    ~LogHandler();
};
