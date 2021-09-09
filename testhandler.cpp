#include "testhandler.h"

#include "databasehandler.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFile>

#include <stdlib.h>

// The test code produces 3 instances of everything necessary.
// This is to make testing more predictable and simpler to code,
// decreasing the complexity and the potential of bugs in the test code

TestHandler::TestHandler(const QString &databasePath)
{
    QFile file(databasePath);
    if(file.exists())
    {
        file.remove();
    }
    m_DBHandler = new DatabaseHandler(databasePath);
}

void TestHandler::testCaseEdgeNode()
{
    try
    {
        QSqlQuery query;
        query.exec("DELETE FROM edgenode");

        DatabaseHandler::EdgeNode node;
        m_DBHandler->registerEdgeNode("ABCD", true, "2021-08-27 09:19:00.000");
        m_DBHandler->registerEdgeNode("EFGH", false, "2011-04-15 17:33:04.372");
        m_DBHandler->registerEdgeNode("IJKL", true, "2016-04-16 07:36:03.987");
        Q_ASSERT(m_DBHandler->getEdgeNode("IJKL", node));
        Q_ASSERT(node.lastHeartbeat == "2016-04-16 07:36:03.987");

        QVector<QString> allKeys;
        m_DBHandler->getAllEdgeNodeKeys(allKeys);
        Q_ASSERT(allKeys.size() == 3);
        Q_ASSERT(checkString(allKeys[0], "ABCD", "EFGH", "IJKL"));

        std::vector<std::unique_ptr<DatabaseHandler::EdgeNode>> edges;
        m_DBHandler->getAllEdgeNodes(edges);
        Q_ASSERT(edges.size() == 3);
        Q_ASSERT(checkString((*(edges[2])).macAddress, "ABCD", "EFGH", "IJKL"));

        QVector<QString> allOnlineEdges;
        m_DBHandler->getOnlineEdgeNodes(allOnlineEdges);
        Q_ASSERT(allOnlineEdges.size() == 2);

        Q_ASSERT(m_DBHandler->getEdgeNode("ABCD", node));
        Q_ASSERT(node.isOnline  == true);
        m_DBHandler->setEdgeNodeOnlineStatus("ABCD", false);
        Q_ASSERT(m_DBHandler->getEdgeNode("ABCD", node));
        Q_ASSERT(node.isOnline == false);

        allOnlineEdges.clear();
        m_DBHandler->getOnlineEdgeNodes(allOnlineEdges);
        Q_ASSERT(allOnlineEdges.size() == 1);
        Q_ASSERT(allOnlineEdges[0] == "IJKL");

        qCritical() << __PRETTY_FUNCTION__ << "Completed successfully";
    }
    catch(std::exception& e)
    {
        qFatal("testCaseEdgeNode failed with exception = %s", e.what());
    }
}

void TestHandler::testCaseVendor()
{
    try
    {
        QSqlQuery query;
        query.exec("DELETE FROM vendor");

        DatabaseHandler::Vendor vendor;
        m_DBHandler->registerVendor("DCBA", "Sabaton");
        m_DBHandler->registerVendor("HGFE", "Babymetal");
        m_DBHandler->registerVendor("LKJI", "Nightwish");

        Q_ASSERT(m_DBHandler->getVendor("HGFE", vendor));
        Q_ASSERT(vendor.vendorName == "Babymetal");

        QVector<QString> allKeys;
        m_DBHandler->getAllVendorKeys(allKeys);
        Q_ASSERT(allKeys.size() == 3);
        Q_ASSERT(checkString(allKeys[1], "DCBA", "HGFE", "LKJI"));

        std::vector<std::unique_ptr<DatabaseHandler::Vendor>> vendors;
        m_DBHandler->getAllVendors(vendors);
        Q_ASSERT(vendors.size() == 3);
        Q_ASSERT(checkString((*(vendors[2])).vendorName, "Sabaton", "Babymetal", "Nightwish"));

        qCritical() << __PRETTY_FUNCTION__ << "Completed successfully";
    }
    catch(std::exception& e)
    {
        qFatal("testCaseVendor failed with exception = %s", e.what());
    }
}

void TestHandler::testCaseProduct()
{
    try
    {
        QSqlQuery query;
        query.exec("DELETE FROM product");

        DatabaseHandler::Product product;
        m_DBHandler->registerProduct("QWER", "Make");
        m_DBHandler->registerProduct("TYUI", "Pepsi Twist");
        m_DBHandler->registerProduct("ASDF", "Again");

        Q_ASSERT(m_DBHandler->getProduct("ASDF", product));
        Q_ASSERT(product.productName == "Again");

        QVector<QString> allKeys;
        m_DBHandler->getAllProductKeys(allKeys);
        Q_ASSERT(allKeys.size() == 3);
        Q_ASSERT(checkString(allKeys[2], "QWER", "TYUI", "ASDF"));

        std::vector<std::unique_ptr<DatabaseHandler::Product>> products;
        m_DBHandler->getAllProducts(products);
        Q_ASSERT(products.size() == 3);
        Q_ASSERT(checkString((*(products[0])).productName, "Make", "Pepsi Twist", "Again"));

        qCritical() << __PRETTY_FUNCTION__ << "Completed successfully";
    }
    catch(std::exception& e)
    {
        qFatal("testCaseProduct failed with exception = %s", e.what());
    }
}

void TestHandler::testVirus()
{
    try
    {
        QSqlQuery query;
        query.exec("DELETE FROM virushash");

        DatabaseHandler::VirusHash virusHash;
        m_DBHandler->registerVirusHash("UVUUNNU", "Totally");
        m_DBHandler->registerVirusHash("YUCWZXB", "not a");
        m_DBHandler->registerVirusHash("OPMIMOIBTV", "virus");

        Q_ASSERT(m_DBHandler->getVirusHash("UVUUNNU", virusHash));
        Q_ASSERT(virusHash.description == "Totally");

        QVector<QString> allKeys;
        m_DBHandler->getAllVirusHashKeys(allKeys);
        Q_ASSERT(allKeys.size() == 3);
        Q_ASSERT(checkString(allKeys[0], "UVUUNNU", "YUCWZXB", "OPMIMOIBTV"));

        std::vector<std::unique_ptr<DatabaseHandler::VirusHash>> virusHashes;
        m_DBHandler->getAllVirusHashes(virusHashes);
        Q_ASSERT(virusHashes.size() == 3);
        Q_ASSERT(checkString((*(virusHashes[2])).description, "Totally", "not a", "virus"));

        Q_ASSERT(m_DBHandler->isVirusHashInDatabase("OPMIMOIBTV"));
        Q_ASSERT(!(m_DBHandler->isVirusHashInDatabase("NO HASH HERE")));

        qCritical() << __PRETTY_FUNCTION__ << "Completed successfully";
    }
    catch(std::exception& e)
    {
        qFatal("testVirus failed with exception = %s", e.what());
    }
}

void TestHandler::testCaseProductVendor(bool requiredDataExists)
{
    try
    {
        QSqlQuery query;
        query.exec("DELETE FROM productvendor");

        if(!requiredDataExists)
        {
            testCaseProduct();
            testCaseVendor();
        }

        QVector<QString> productKeys;
        QVector<QString> vendorKeys;

        m_DBHandler->getAllProductKeys(productKeys);
        m_DBHandler->getAllVendorKeys(vendorKeys);
        Q_ASSERT(productKeys.size() == vendorKeys.size());
        Q_ASSERT(productKeys.size() == 3);

        for(int i = 0; i < productKeys.size(); ++i)
        {
            // Foreign key constraints will cause this to fail if there are any problems
            m_DBHandler->linkProductVendor(productKeys[i], vendorKeys[i]);
        }

        qCritical() << __PRETTY_FUNCTION__ << "Completed successfully";
    }
    catch(std::exception& e)
    {
        qFatal("testCaseProductVendor failed with exception = %s", e.what());
    }
}

void TestHandler::testCaseDevice(bool requiredDataExists)
{
    try
    {
        QSqlQuery query;
        query.exec("DELETE FROM device");

        if(!requiredDataExists)
        {
            testCaseProductVendor();
        }

        QVector<QString> productKeys;
        QVector<QString> vendorKeys;

        m_DBHandler->getAllProductKeys(productKeys);
        m_DBHandler->getAllVendorKeys(vendorKeys);
        Q_ASSERT(productKeys.size() == vendorKeys.size());
        Q_ASSERT(productKeys.size() == 3);

        for(int i = 0; i < productKeys.size(); ++i)
        {
            m_DBHandler->registerDevice(QString::number(i + 1000), productKeys[i], vendorKeys[i]);
        }

        DatabaseHandler::Device device;
        Q_ASSERT(m_DBHandler->getDevice("1000", device));
        Q_ASSERT(device.productId == productKeys[0]);
        Q_ASSERT(device.vendorId == vendorKeys[0]);

        QVector<QString> allKeys;
        m_DBHandler->getAllDeviceKeys(allKeys);
        Q_ASSERT(allKeys.size() == 3);
        Q_ASSERT(checkString(allKeys[0], "1000", "1001", "1002"));

        std::vector<std::unique_ptr<DatabaseHandler::Device>> devices;
        m_DBHandler->getAllDevices(devices);
        Q_ASSERT(devices.size() == 3);
        Q_ASSERT(checkString((*(devices[0])).productId, productKeys[0], productKeys[1], productKeys[2]));

        Q_ASSERT(!m_DBHandler->isDeviceBlackListed(device.serialNumber));
        Q_ASSERT(!m_DBHandler->isDeviceWhiteListed(device.serialNumber));
        m_DBHandler->setDeviceWhitelisted(device.serialNumber);
        Q_ASSERT(!m_DBHandler->isDeviceBlackListed(device.serialNumber));
        Q_ASSERT(m_DBHandler->isDeviceWhiteListed(device.serialNumber));
        m_DBHandler->setDeviceBlacklisted(device.serialNumber);
        Q_ASSERT(m_DBHandler->isDeviceBlackListed(device.serialNumber));
        Q_ASSERT(!m_DBHandler->isDeviceWhiteListed(device.serialNumber));

        Q_ASSERT(!m_DBHandler->isDeviceBlackListed("1234"));
        Q_ASSERT(!m_DBHandler->isDeviceWhiteListed("1234"));

        qCritical() << __PRETTY_FUNCTION__ << "Completed successfully";
    }
    catch(std::exception& e)
    {
        qFatal("testCaseDevice failed with exception = %s", e.what());
    }
}

void TestHandler::testCaseLog(bool requiredDataExists)
{
    try
    {
        QSqlQuery query;
        query.exec("DELETE FROM log");

        if(!requiredDataExists)
        {
            testCaseDevice(false);
        }

        QVector<QString> edgeKeys;
        QVector<QString> deviceKeys;

        m_DBHandler->getAllEdgeNodeKeys(edgeKeys);
        m_DBHandler->getAllDeviceKeys(deviceKeys);
        Q_ASSERT(edgeKeys.size() == deviceKeys.size());
        Q_ASSERT(edgeKeys.size() == 3);

        for(int i = 0; i < edgeKeys.size(); ++i)
        {
            m_DBHandler->logEvent(edgeKeys[i], deviceKeys[i], "2021:09:09 22:36:00:00" + QString::number(i), "Number " + QString::number(i));
        }

        DatabaseHandler::LogEvent logEvent;
        Q_ASSERT(m_DBHandler->getLoggedEvent(edgeKeys[2], deviceKeys[2], "2021:09:09 22:36:00:002", logEvent));
        Q_ASSERT(logEvent.eventDescription == "Number 2");

        std::vector<std::unique_ptr<DatabaseHandler::LogEvent>> logEvents;
        m_DBHandler->getAllLoggedEvents(logEvents);
        Q_ASSERT(logEvents.size() == 3);
        Q_ASSERT(checkString((*(logEvents[0])).edgeNodeMacAddress, edgeKeys[0], edgeKeys[1], edgeKeys[2]));
        Q_ASSERT(checkString((*(logEvents[1])).timestamp, "2021:09:09 22:36:00:000", "2021:09:09 22:36:00:001", "2021:09:09 22:36:00:002"));

        qCritical() << __PRETTY_FUNCTION__ << "Completed successfully";
    }
    catch(std::exception& e)
    {
        qFatal("testCaseLog failed with exception = %s", e.what());
    }
}

void TestHandler::testCaseAll()
{
    testCaseEdgeNode();
    testCaseProduct();
    testCaseVendor();
    testVirus();
    testCaseProductVendor(true);
    testCaseDevice(true);
    testCaseLog(true);
}

bool TestHandler::checkString(const QString &query, const QString &target1, const QString &target2, const QString &target3)
{
    if((query == target1) ||
        (query == target2) ||
        (query == target3))
    {
        return true;
    }
    else
    {
        return false;
    }
}
