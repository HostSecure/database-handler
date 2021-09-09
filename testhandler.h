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
    void testCaseProductVendor(bool requiredDataExists = false);
    void testCaseDevice(bool requiredDataExists = false);
    void testCaseLog(bool requiredDataExists = false);
    void testCaseAll();

private:
    bool checkString(const QString& query, const QString& target1, const QString& target2, const QString& target3);
    DatabaseHandler* m_DBHandler;
};

#endif // TESTHANDLER_H
