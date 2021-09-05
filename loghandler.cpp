#include "loghandler.h"

#include <QFile>
#include <QDir>
#include <QDateTime>

namespace
{
QFile logFile;;
}

// Taken from qInstallMessageHandler example code
void loghandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", msg.toStdString().c_str(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", msg.toStdString().c_str(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
    {
        if(logFile.open(QIODevice::WriteOnly| QIODevice::Append))
        {
            logFile.write(msg.toStdString().c_str());
            logFile.write("\n");
            logFile.close();
        }
        else
        {
            fprintf(stderr, "Failed to open log file: %s\n", logFile.errorString().toStdString().c_str());
        }

        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", msg.toStdString().c_str(), context.file, context.line, context.function);
        break;
    }
    case QtCriticalMsg:
    {
        if(logFile.open(QIODevice::WriteOnly| QIODevice::Append))
        {
            logFile.write(msg.toStdString().c_str());
            logFile.write("\n");
            logFile.close();
        }
        else
        {
            fprintf(stderr, "Failed to open log file: %s\n", logFile.errorString().toStdString().c_str());
        }
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", msg.toStdString().c_str(), context.file, context.line, context.function);
        break;
    }
    case QtFatalMsg:
    {
        if(logFile.open(QIODevice::WriteOnly| QIODevice::Append))
        {
            logFile.write(msg.toStdString().c_str());
            logFile.write("\n");
            logFile.close();
        }
        else
        {
            fprintf(stderr, "Failed to open log file: %s\n", logFile.errorString().toStdString().c_str());
        }
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", msg.toStdString().c_str(), context.file, context.line, context.function);
        abort();
    }
    default:
        fprintf(stderr, "Undefined message type: %s (%s:%u, %s)\n", msg.toStdString().c_str(), context.file, context.line, context.function);
        break;
    }
}

LogHandler::LogHandler()
{
    QString logFileName = QDateTime::currentDateTime().toString() + ".txt";
    QString logFilePath;

    const char* tempPath = getenv("LOG_FILE_DIR");
    if(tempPath == nullptr)
    {
        logFilePath = QDir::currentPath() + "/logs";
    }
    else
    {
        logFilePath = tempPath;
    }

    QDir dir(logFilePath + "/");
    if(!dir.exists())
    {
        if(!dir.mkpath(logFilePath))
        {
            qCritical() << "Failed to create logging path: " << logFilePath;
            return;
        }
    }

    logFile.setFileName(logFilePath + logFileName);
    if(logFile.open(QIODevice::ReadWrite))
    {
        logFile.close();
        qInstallMessageHandler(loghandler);
    }
    else
    {
        qCritical() << "Failed to create or open log file: " << logFile.errorString();
    }
}

LogHandler::~LogHandler()
{
    qInstallMessageHandler(0);
    logFile.close();
}
