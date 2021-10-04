#pragma once
#include <QString>

class DatabaseHandler;
//!
//! \brief The TestHandler class
//! A class used to test and verify the databasehandler API and its queries.
//!
class TestHandler
{
public:
    TestHandler(const QString& a_databasePath);

    void testCaseEdgeNode();
    void testVirus();
    void testCaseProductVendor();
    void testCaseDevice(bool a_requiredDataExists = false);
    void testCaseConnectedDevice(bool a_requiredDataExists = false);
    void testCaseLog(bool a_requiredDataExists = false);
    void testCaseAll();

private:
    bool checkString(const QString& a_query, const QString& a_target1, const QString& a_target2, const QString& a_target3);
    DatabaseHandler* m_DBHandler;
};
