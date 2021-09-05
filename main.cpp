#include <QCoreApplication>

#include "loghandler.h"
#include "databasehandler.h"


void testCaseEdgeNode(DatabaseHandler& db)
{
}

void testCaseVendor(DatabaseHandler& db)
{
}

void testCaseProduct(DatabaseHandler& db)
{
}

void testVirus(DatabaseHandler& db)
{
}

void testCaseDevice(DatabaseHandler& db)
{
}

void testCaseEdgeNodeOnlineStatus(DatabaseHandler& db)
{
}

void testCaseDeviceStatus(DatabaseHandler& db)
{
}

void testCaseLog(DatabaseHandler& db)
{
}

void testCaseAll(DatabaseHandler& db)
{
    testCaseEdgeNode(db);
    testCaseVendor(db);
    testCaseProduct(db);
    testVirus(db);
    testCaseDevice(db);
    testCaseEdgeNodeOnlineStatus(db);
    testCaseDeviceStatus(db);
    testCaseLog(db);
}

int main(int argc, char *argv[])
{
    bool test = false;
    LogHandler logger;

    QCoreApplication a(argc, argv);

    // TODO: Nobody likes hardcoded paths
    if(test)
    {
        DatabaseHandler dbHandler("/home/kali/QtProjects/TestDatabase/testcases.db");
        testCaseAll(dbHandler);
    }
    else
    {
        DatabaseHandler dbHandler("/home/kali/QtProjects/TestDatabase/test.db");
    }

    return a.exec();
}
