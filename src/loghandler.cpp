#include "loghandler.h"

#include <QFile>
#include <QDir>
#include <QDateTime>

namespace
{
   QFile logFile;
}

//!
//! \brief The loghandler function
//!  Automcatically called by Qt when a qInfo, qDebug,  qWarning, qCritical, or qFatal is called
//!  Logs qWarning, qCritical, and qFatal three to file,
//!  The code is strongly inspired by the qInstallMessageHandler example code
//!
void loghandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
   switch (type) {
   case QtDebugMsg:
      fprintf(stderr, "Debug: %s \t\t(%s:%u, %s)\n", msg.toStdString().c_str(), context.file, context.line, context.function);
      break;
   case QtInfoMsg:
      fprintf(stderr, "Info: %s \t\t(%s:%u, %s)\n", msg.toStdString().c_str(), context.file, context.line, context.function);
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

      fprintf(stderr, "Warning: %s \t\t(%s:%u, %s)\n", msg.toStdString().c_str(), context.file, context.line, context.function);
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
      fprintf(stderr, "Critical: %s \t\t(%s:%u, %s)\n", msg.toStdString().c_str(), context.file, context.line, context.function);
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
      fprintf(stderr, "Fatal: %s \t\t(%s:%u, %s)\n", msg.toStdString().c_str(), context.file, context.line, context.function);
      abort();
   }
   default:
      fprintf(stderr, "Undefined message type: %s \t\t(%s:%u, %s)\n", msg.toStdString().c_str(), context.file, context.line, context.function);
      break;
   }
}

//!
//! \brief The LogHandler constructor
//! Handles the logging setup
//!
LogHandler::LogHandler()
{
   QString logFileName = QDateTime::currentDateTime().toString() + ".txt";

   const char* dataDir = getenv("HOSTSECURE_DATA_DIR");
   if(dataDir == nullptr)
   {
      dataDir = ".";
   }

   QString logFilePath(dataDir);
   logFilePath.append("/Logs/");

   QDir dir(logFilePath);
   if(!dir.exists())
   {
      if(!dir.mkpath(dir.absolutePath()))
      {
         qCritical() << "Failed to create logging path: " << dir.absolutePath();
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

//!
//! \brief The LogHandler desctructor
//! Handles cleanup of the logger
//!
LogHandler::~LogHandler()
{
   qInstallMessageHandler(0);
   logFile.close();
}
