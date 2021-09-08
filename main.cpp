#include <QCoreApplication>

#include "loghandler.h"
#include "databasehandler.h"
#include "testhandler.h"

int main(int argc, char *argv[])
{
    bool test = true;
    LogHandler logger;

    QCoreApplication a(argc, argv);

    // TODO: Nobody likes hardcoded paths
    if(test)
    {
        TestHandler testHandler("/home/kali/QtProjects/TestDatabase/testcases.db");
        testHandler.testCaseAll();
    }
    else
    {
        DatabaseHandler dbHandler("/home/kali/QtProjects/TestDatabase/test.db");
    }

    return a.exec();
}
