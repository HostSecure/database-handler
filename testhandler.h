#ifndef TESTHANDLER_H
#define TESTHANDLER_H

#include <QString>

class DatabaseHandler;
class TestHandler
{
public:
    TestHandler(const QString& databasePath);

    void testCaseEdgeNode();
    void testCaseVendor();
    void testCaseProduct();
    void testVirus();
    void testCaseDevice();
    void testCaseLog();
    void testCaseAll();

private:
    DatabaseHandler* m_DBHandler;
};

#endif // TESTHANDLER_H
