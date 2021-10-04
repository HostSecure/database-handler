#include <QCoreApplication>

#include <loghandler.h>
#include <databasemanager.h>
#include <testhandler.h>

int main(int argc, char *argv[])
{
    bool test = false;
    LogHandler logger;

    QCoreApplication a(argc, argv);

    const char* dataDir = getenv("HOSTSECURE_DATA_DIR");
    if(dataDir == nullptr)
    {
        dataDir = ".";
    }

    if(test)
    {
        TestHandler testHandler(QString(dataDir).append("/Databases/testcases.db"));
        testHandler.testCaseAll();
        return a.exec();
    }
    else
    {
        DatabaseManager dbAdmin(QString(dataDir).append("/Databases/HostSecure.db"), &a);
        return a.exec();
    }
}
