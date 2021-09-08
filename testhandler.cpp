#include "testhandler.h"

#include "databasehandler.h"

#include <QSqlQuery>

TestHandler::TestHandler(const QString &databasePath)
{
    m_DBHandler = new DatabaseHandler(databasePath);
}

void TestHandler::testCaseEdgeNode()
{
    QSqlQuery query;
    query.exec("DELETE FROM edgenode");

    try
    {
        DatabaseHandler::EdgeNode node;
        m_DBHandler->registerEdgeNode("ABCD", true, "2021-08-27 09:19:00.000");
        m_DBHandler->registerEdgeNode("EFGH", false, "2011-04-15 17:33:04.372");
        m_DBHandler->registerEdgeNode("IJKL", true, "2016-04-16 07:36:03.987");
        m_DBHandler->getEdgeNode("IJKL", node);
        Q_ASSERT(node.lastHeartbeat == "2016-04-16 07:36:03.987");

        QVector<QString> allKeys;
        m_DBHandler->getAllEdgeNodeKeys(allKeys);
        Q_ASSERT(allKeys.size() == 3);

        std::vector<std::unique_ptr<DatabaseHandler::EdgeNode>> edges;
        m_DBHandler->getAllEdgeNodes(edges);
        Q_ASSERT(edges.size() == 3);
        Q_ASSERT((*(edges[2])).macAddress == "IJKL");

        QVector<QString> allOnlineEdges;
        m_DBHandler->getOnlineEdgeNodes(allOnlineEdges);
        Q_ASSERT(allOnlineEdges.size() == 2);

        m_DBHandler->getEdgeNode(edges[0]->macAddress, node);
        Q_ASSERT(node.isOnline  == true);
        m_DBHandler->setEdgeNodeOnlineStatus(edges[0]->macAddress, false);
        m_DBHandler->getEdgeNode(edges[0]->macAddress, node);
        Q_ASSERT(node.isOnline == false);

        allOnlineEdges.clear();
        m_DBHandler->getOnlineEdgeNodes(allOnlineEdges);
        Q_ASSERT(allOnlineEdges.size() == 1);

        qCritical() << __PRETTY_FUNCTION__ << "Completed successfully";
    }
    catch(std::exception& e)
    {
        qInfo() << "Test failed with exception = " << e.what();
    }
}

void TestHandler::testCaseVendor()
{
    QSqlQuery query;
    query.exec("DELETE FROM vendor");
}

void TestHandler::testCaseProduct()
{
    QSqlQuery query;
    query.exec("DELETE FROM product");
}

void TestHandler::testVirus()
{
    QSqlQuery query;
    query.exec("DELETE FROM virushash");
}

void TestHandler::testCaseDevice()
{
    QSqlQuery query;
    query.exec("DELETE FROM device");
}

void TestHandler::testCaseLog()
{
    QSqlQuery query;
    query.exec("DELETE FROM log");
}

void TestHandler::testCaseAll()
{
    testCaseEdgeNode();
    testCaseVendor();
    testCaseProduct();
    testVirus();
    testCaseDevice();
    testCaseLog();
}
