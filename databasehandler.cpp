#include "databasehandler.h"

#include <QDebug>
#include <QFile>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

namespace
{
const QString DEVICE_STATUS_UNKNOWN = "U";
const QString DEVICE_STATUS_WHITELISTED = "W";
const QString DEVICE_STATUS_BLACKLISTED = "B";

//const QString CREATE = "CREATE TABLE ";
//const QString INSERT = "INSERT INTO ";
//const QString PRIMARY = "PRIMARY KEY";
//const QString FOREIGN = "FOREIGN KEY";

//const QString EDGENODE = "edgenode";
//const QString EDGENODEID = "macaddress";
//const QString EDGENODEIDTYPE = "VARCHAR(8)";
//const QString EDGENODEDESCRIPTTION = "nodedescription";
//const QString EDGENODEDESCRIPTIONTYPE = "VARCHAR(50)";

//const QString VENDOR = "vendor";
//const QString VENDORID = "vendorid";
//const QString VENDORIDTYPE = "VARCHAR(4)";
//const QString VENDORNAME = "vendorname";
//const QString VENDORNAMETYPE = "VARCHAR(30)";
}

DatabaseHandler::DatabaseHandler(const QString& databasePath)
{
    //TODO: Handle differently, e.g. by using a SELECT call on some table to make it support other db types (e.g. MariaDB/MySQL)
    bool exists = QFile::exists(databasePath);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databasePath);

    if(!db.open())
    {
        qFatal("Failed to open database: %s", db.lastError().text().toStdString().c_str());
    }
    else
    {
        QSqlQuery query;
        query.exec("PRAGMA foreign_keys = ON;");

        if(!exists)
        {
//            query.exec(CREATE + EDGENODE + "(" + EDGENODEID + " " + EDGENODEIDTYPE + " " + PRIMARY + ", " + EDGENODEDESCRIPTTION + " " + EDGENODEDESCRIPTIONTYPE + ")");
            query.exec("CREATE TABLE edgenode(macaddress VARCHAR(8) PRIMARY KEY, isonline BIT NOT NULL, lastheartbeat TIMESTAMP NOT NULL)");
//            query.exec(CREATE + VENDOR + "(" + VENDORID + " " + VENDORIDTYPE + " " + PRIMARY + ", " + VENDORNAME + " " + VENDORNAMETYPE + ")");
            query.exec("CREATE TABLE vendor(vendorid VARCHAR(4) PRIMARY KEY, vendorname VARCHAR(30))");
            query.exec("CREATE TABLE product(productid VARCHAR(4) PRIMARY KEY, productname VARCHAR(30))");
            query.exec("CREATE TABLE virushash(hashkey VARCHAR(32) PRIMARY KEY, description VARCHAR(100))");

            if(!query.exec("CREATE TABLE productvendor(productid VARCHAR(4), vendorid VARCHAR(4), "
                                                  "FOREIGN KEY(productid) REFERENCES product(productid), "
                                                  "FOREIGN KEY(vendorid) REFERENCES vendor(vendorid), "
                                                  "PRIMARY KEY(productid, vendorid))"))
            {
                qInfo() << "Failed";
            }

            query.exec("CREATE TABLE device(serialnumber VARCHAR(8) PRIMARY KEY, productid VARCHAR(4) NOT NULL, vendorid VARCHAR(4) NOT NULL, status CHAR(1) NOT NULL, "
                       "FOREIGN KEY(productid, vendorid) REFERENCES productvendor(productid, vendorid))");

            query.exec("CREATE TABLE log(edgenodemacaddress VARCHAR(8), "
                       "deviceid VARCHAR(8), "
                       "logtime TIMESTAMP, "
                       "loginfo VARCHAR(100), "
                       "FOREIGN KEY (edgenodemacaddress) REFERENCES edgenode(macaddress), "
                       "FOREIGN KEY (deviceid) REFERENCES device(serialnumber), "
                       "PRIMARY KEY(edgenodemacaddress, deviceid, logtime))");

            // TODO: Import data
        }
    }
}

void DatabaseHandler::registerEdgeNode(const QString &macaddress, bool isOnline, const QString& lastHeartbeatTimestamp) const
{
    QSqlQuery query;
    query.prepare("INSERT INTO edgenode(macaddress, isonline, lastheartbeat)"
                  "VALUES(?, ?, ?)");
    query.bindValue(0, macaddress);
    query.bindValue(1, (isOnline ? 1 : 0));
    query.bindValue(2, lastHeartbeatTimestamp);

    if(!query.exec())
    {
        qWarning() << __PRETTY_FUNCTION__ << "Failed to register edge node: " << query.lastError();
        throw std::runtime_error("Failed to register edge node");
    }
}

bool DatabaseHandler::getEdgeNode(const QString &macAddress, EdgeNode &edgeNode) const
{
    bool success = false;
    QSqlQuery query;
    query.prepare("SELECT * FROM edgenode WHERE macaddress = ?");
    query.bindValue(0, macAddress);

    if(query.exec())
    {
        if(query.next())
        {
            edgeNode.macAddress = query.value(0).toString();
            edgeNode.isOnline = query.value(1).toBool();
            edgeNode.lastHeartbeat = query.value(2).toString();
            success = true;
        }
    }
    else
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to get edge node: " << query.lastError();
        throw std::runtime_error("Failed to get edge node");
    }

    return success;
}

void DatabaseHandler::getAllEdgeNodeKeys(QVector<QString> &macAddresses) const
{
    try
    {
        getKeysFromTable("macaddress", "edgenode",  macAddresses);
    }
    catch(std::exception& e)
    {
        qCritical() <<  __PRETTY_FUNCTION__ << e.what();
        throw e;
    }
}

void DatabaseHandler::getAllEdgeNodes(std::vector<std::unique_ptr<EdgeNode> > &edgeNodes) const
{
    QSqlQuery query;
    if(query.exec("SELECT * FROM edgenode"))
    {
        while(query.next())
        {
            std::unique_ptr<EdgeNode> node =  std::make_unique<EdgeNode>();
            node->macAddress = query.value(0).toString();
            node->isOnline = query.value(1).toBool();
            node->lastHeartbeat = query.value(2).toString();
            edgeNodes.push_back(std::move(node));
        }
    }
    else
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to get all edge nodes: " << query.lastError();
        throw std::runtime_error("Failed to get all edge nodes");
    }
}

void DatabaseHandler::setEdgeNodeOnlineStatus(const QString &macAddress, bool isOnline, const QString &lastHeartbeatTimestamp)
{
    QSqlQuery query;
    query.prepare("UPDATE edgenode SET isonline = ?, lastheartbeat = ? WHERE macAddress = ?");
    query.bindValue(0, (isOnline ? 1 : 0));
    query.bindValue(1, lastHeartbeatTimestamp);
    query.bindValue(2, macAddress);

    if(!query.exec())
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to set edge node online status: " << query.lastError();
        throw std::runtime_error("Failed to set edge node online status");
    }
}

void DatabaseHandler::getOnlineEdgeNodes(QVector<QString> &macAddresses) const
{
    QSqlQuery query;

    if(query.exec("SELECT macaddress FROM edgenode WHERE isonline = 1"))
    {
        while (query.next())
        {
           macAddresses.push_back( query.value(0).toString());
        }
    }
    else
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to set edge node online status: " << query.lastError();
        throw std::runtime_error("Failed to set edge node online status");
    }
}

void DatabaseHandler::registerDevice(const QString &serialNumber, const QString &productId, const QString &vendorId) const
{
    QSqlQuery query;
    query.prepare("INSERT INTO device(serialnumber, productid, vendorid, status)"
                  "VALUES(?, ?, ?, ?)");
    query.bindValue(0, serialNumber);
    query.bindValue(1, productId);
    query.bindValue(2, vendorId);
    query.bindValue(3, DEVICE_STATUS_UNKNOWN);

    if(!query.exec())
    {
        qWarning() << __PRETTY_FUNCTION__ << "Failed to register device: " << query.lastError();
        throw std::runtime_error("Failed to register device");
    }
}

bool DatabaseHandler::getDevice(const QString &serialNumber, Device &device) const
{
    bool success = false;
    QSqlQuery query;
    query.prepare("SELECT * FROM device WHERE serialnumber = ?");
    query.bindValue(0, serialNumber);

    if(query.exec())
    {
        if(query.next())
        {
            device.serialNumber = query.value(0).toString();
            device.productId = query.value(1).toString();
            device.vendorId = query.value(2).toString();
            success = true;
        }
    }
    else
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to get device: " << query.lastError();
        throw std::runtime_error("Failed to get device");
    }

    return success;
}

void DatabaseHandler::getAllDeviceKeys(QVector<QString> &serialNumbers) const
{
    try
    {
        getKeysFromTable("serialnumber", "device",  serialNumbers);
    }
    catch(std::exception& e)
    {
        qCritical() <<  __PRETTY_FUNCTION__ << e.what();
        throw e;
    }
}

void DatabaseHandler::getAllDevices(std::vector<std::unique_ptr<Device> > &devices) const
{
    QSqlQuery query;
    if(query.exec("SELECT * FROM device"))
    {
        while(query.next())
        {
            std::unique_ptr<Device> device =  std::make_unique<Device>();
            device->serialNumber = query.value(0).toString();
            device->productId = query.value(1).toString();
            device->vendorId = query.value(2).toString();
            devices.push_back(std::move(device));
        }
    }
    else
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to get all devices: " << query.lastError();
        throw std::runtime_error("Failed to get all devices");
    }
}

void DatabaseHandler::setDeviceBlacklisted(const QString &serialNumber) const
{
    try
    {
        setDeviceStatus(serialNumber, DEVICE_STATUS_BLACKLISTED);
    }
    catch (std::exception& e)
    {
        qCritical() <<  __PRETTY_FUNCTION__ << e.what();
        throw e; // Forward the exception so it can be handled correctly by the caller";
    }
}

bool DatabaseHandler::isDeviceBlackListed(const QString &serialNumber) const
{
    bool retVal = true; // Assume the worst
    try
    {
        retVal = checkDeviceStatus(serialNumber, DEVICE_STATUS_BLACKLISTED);
    }
    catch(std::exception& e)
    {
        qCritical() <<  __PRETTY_FUNCTION__ << e.what();
        throw e; // Forward the exception so it can be handled correctly by the caller";
    }

    return retVal;
}

void DatabaseHandler::setDeviceWhitelisted(const QString &serialNumber) const
{
    try
    {
        setDeviceStatus(serialNumber, DEVICE_STATUS_WHITELISTED);
    }
    catch (std::exception& e)
    {
        qCritical() <<  __PRETTY_FUNCTION__ << e.what();
        throw e; // Forward the exception so it can be handled correctly by the caller";
    }
}

bool DatabaseHandler::isDeviceWhiteListed(const QString &serialNumber) const
{
    bool retVal = false;
    try
    {
        retVal = checkDeviceStatus(serialNumber, DEVICE_STATUS_WHITELISTED);
    }
    catch(std::exception& e)
    {
        qCritical() <<  __PRETTY_FUNCTION__ << e.what();
        throw e; // Forward the exception so it can be handled correctly by the caller";
    }

    return retVal;
}

void DatabaseHandler::registerVendor(const QString &vendorId, const QString &vendorName)
{
    QSqlQuery query;
    query.prepare("INSERT INTO vendor(vendorid, vendorname)"
                  "VALUES(?, ?)");
    query.bindValue(0, vendorId);
    query.bindValue(1, vendorName);

    if(!query.exec())
    {
        qWarning() << __PRETTY_FUNCTION__ << "Failed to register vendor: " << query.lastError();
        throw std::runtime_error("Failed to register vendor");
    }
}

bool DatabaseHandler::getVendor(const QString &vendorId, Vendor &vendor) const
{
    bool success = false;
    QSqlQuery query;
    query.prepare("SELECT * FROM vendor WHERE vendorid = ?");
    query.bindValue(0, vendorId);

    if(query.exec())
    {
        if(query.next())
        {
            vendor.vendorId = query.value(0).toString();
            vendor.vendorName = query.value(1).toString();
            success = true;
        }
    }
    else
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to get vendor: " << query.lastError();
        throw std::runtime_error("Failed to get vendor");
    }

    return success;
}

void DatabaseHandler::getAllVendorKeys(QVector<QString> &vendorIds) const
{
    try
    {
        getKeysFromTable("vendorid", "vendor",  vendorIds);
    }
    catch(std::exception& e)
    {
        qCritical() <<  __PRETTY_FUNCTION__ << e.what();
        throw e;
    }
}

void DatabaseHandler::getAllVendors(std::vector<std::unique_ptr<Vendor> >& vendors) const
{
    QSqlQuery query;
    if(query.exec("SELECT * FROM vendor"))
    {
        while(query.next())
        {
            std::unique_ptr<Vendor> vendor =  std::make_unique<Vendor>();
            vendor->vendorId = query.value(0).toString();
            vendor->vendorName = query.value(1).toString();
            vendors.push_back(std::move(vendor));
        }
    }
    else
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to get all vendors: " << query.lastError();
        throw std::runtime_error("Failed to get all vendors");
    }
}

void DatabaseHandler::registerProduct(const QString &productId, const QString &productName)
{
    QSqlQuery query;
    query.prepare("INSERT INTO product(productid, productname)"
                  "VALUES(?, ?)");
    query.bindValue(0, productId);
    query.bindValue(1, productName);

    if(!query.exec())
    {
        qWarning() << __PRETTY_FUNCTION__ << "Failed to register product: " << query.lastError();
        throw std::runtime_error("Failed to register product");
    }
}

bool DatabaseHandler::getProduct(const QString &productId, Product &product) const
{
    bool success = false;
    QSqlQuery query;
    query.prepare("SELECT * FROM product WHERE productid = ?");
    query.bindValue(0, productId);

    if(query.exec())
    {
        if(query.next())
        {
            product.productId = query.value(0).toString();
            product.productName = query.value(1).toString();
            success = true;
        }
    }
    else
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to get product: " << query.lastError();
        throw std::runtime_error("Failed to get product");
    }

    return success;
}

void DatabaseHandler::getAllProductKeys(QVector<QString> &productIds) const
{
    try
    {
        getKeysFromTable("productid", "product",  productIds);
    }
    catch(std::exception& e)
    {
        qCritical() <<  __PRETTY_FUNCTION__ << e.what();
        throw e;
    }
}

void DatabaseHandler::getAllProducts(std::vector<std::unique_ptr<Product> >& products) const
{
    QSqlQuery query;
    if(query.exec("SELECT * FROM product"))
    {
        while(query.next())
        {
            std::unique_ptr<Product> product =  std::make_unique<Product>();
            product->productId = query.value(0).toString();
            product->productName = query.value(1).toString();
            products.push_back(std::move(product));
        }
    }
    else
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to get all products: " << query.lastError();
        throw std::runtime_error("Failed to get all products");
    }
}

void DatabaseHandler::linkProductVendor(const QString &productId, const QString &vendorId)
{
    QSqlQuery query;
    query.prepare("INSERT INTO productvendor(productid, vendorid)"
                  "VALUES(?, ?)");
    query.bindValue(0, productId);
    query.bindValue(1, vendorId);

    if(!query.exec())
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to register productvendor: " << query.lastError();
        throw std::runtime_error("Failed to register productvendor");
    }
}

void DatabaseHandler::registerVirusHash(const QString &virusHash, const QString &description)
{
    QSqlQuery query;
    query.prepare("INSERT INTO virushash(hashkey, description)"
                  "VALUES(?, ?)");
    query.bindValue(0, virusHash);
    query.bindValue(1, description);

    if(!query.exec())
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to register virus hash: " << query.lastError();
        throw std::runtime_error("Failed to register virus hash");
    }
}

bool DatabaseHandler::getVirusHash(const QString &virusHash, VirusHash &vHash) const
{
    bool success = false;
    QSqlQuery query;
    query.prepare("SELECT * FROM virushash WHERE hashkey = ?");
    query.bindValue(0, virusHash);

    if(query.exec())
    {
        if(query.next())
        {
            vHash.virusHash = query.value(0).toString();
            vHash.description = query.value(1).toString();
            success = true;
        }
    }
    else
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to get virus hash: " << query.lastError();
        throw std::runtime_error("Failed to get virus hash");
    }

    return success;
}

void DatabaseHandler::getAllVirusHashKeys(QVector<QString> &virusHashes) const
{
    try
    {
        getKeysFromTable("hashkey", "virushash",  virusHashes);
    }
    catch(std::exception& e)
    {
        qCritical() <<  __PRETTY_FUNCTION__ << e.what();
        throw e;
    }
}

void DatabaseHandler::getAllVirusHashes(std::vector<std::unique_ptr<VirusHash> > &virusHashes) const
{
    QSqlQuery query;
    if(query.exec("SELECT * FROM virushash"))
    {
        while(query.next())
        {
            std::unique_ptr<VirusHash> virusHash =  std::make_unique<VirusHash>();
            virusHash->virusHash = query.value(0).toString();
            virusHash->description = query.value(1).toString();
            virusHashes.push_back(std::move(virusHash));
        }
    }
    else
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to get all virus hashes: " << query.lastError();
        throw std::runtime_error("Failed to get all virus hashes");
    }
}

bool DatabaseHandler::isVirusHashInDatabase(const QString &hash) const
{
    bool found = true; // Assume the worst
    QSqlQuery query;
    query.prepare("SELECT * FROM virushash WHERE hashkey = ?");
    query.bindValue(0, hash);

    if(query.exec())
    {
        if(!query.next())
        {
            found = false;
        }
    }
    else
    {
       qCritical() <<  __PRETTY_FUNCTION__ << "Failed to check virus hash: " << query.lastError();
       throw std::runtime_error("Failed to check virus hash");
    }

    return found;
}

void DatabaseHandler::logEvent(const QString &edgeNodeMacAddress, const QString &deviceSerialNumber, const QString &timestamp, const QString &eventDescription)
{
    QSqlQuery query;
    query.prepare("INSERT INTO log(edgenodemacaddress, deviceid, logtime, loginfo)"
                  "VALUES(?, ?, ?, ?)");
    query.bindValue(0, edgeNodeMacAddress);
    query.bindValue(1, deviceSerialNumber);
    query.bindValue(2, timestamp);
    query.bindValue(3, eventDescription);

    if(!query.exec())
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to log event: " << query.lastError();
        throw std::runtime_error("Failed to log event");
    }
}

bool DatabaseHandler::getLoggedEvent(const QString &edgeNodeMacAddress, const QString deviceSerialNumber, const QString &timestamp, LogEvent& logEvent) const
{
    bool success = false;
    QSqlQuery query;
    query.prepare("SELECT * FROM log WHERE edgenodemacaddress = ? AND deviceid = ? AND logtime = ?");
    query.bindValue(0, edgeNodeMacAddress);
    query.bindValue(1, deviceSerialNumber);
    query.bindValue(2, timestamp);

    if(query.exec())
    {
        if(query.next())
        {
            logEvent.edgeNodeMacAddress = query.value(0).toString();
            logEvent.deviceSerialNumber = query.value(1).toString();
            logEvent.timestamp = query.value(2).toString();
            logEvent.eventDescription = query.value(3).toString();
            success = true;
        }
    }
    else
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to get log event: " << query.lastError();
        throw std::runtime_error("Failed to get log event");
    }

    return success;
}

void DatabaseHandler::getAllLoggedEvents(std::vector<std::unique_ptr<LogEvent> >& loggedEvents) const
{
    QSqlQuery query;
    if(query.exec("SELECT * FROM log"))
    {
        while(query.next())
        {
            std::unique_ptr<LogEvent> logEvent =  std::make_unique<LogEvent>();
            logEvent->edgeNodeMacAddress = query.value(0).toString();
            logEvent->deviceSerialNumber = query.value(1).toString();
            logEvent->timestamp = query.value(2).toString();
            logEvent->eventDescription = query.value(3).toString();
            loggedEvents.push_back(std::move(logEvent));
        }
    }
    else
    {
        qCritical() << __PRETTY_FUNCTION__ << "Failed to get all logged events: " << query.lastError();
        throw std::runtime_error("Failed to get all logged events");
    }
}

void DatabaseHandler::getKeysFromTable(const QString keyName, const QString &tableName, QVector<QString> &result) const
{
    QSqlQuery query;
    query.prepare(QString("SELECT %1 FROM %2").arg(keyName, tableName));

    if(query.exec())
    {
        while (query.next())
        {
           result.push_back( query.value(0).toString());
        }
    }
    else
    {
       throw std::runtime_error("Failed to get keys from table: " + query.lastError().text().toStdString());
    }
}

void DatabaseHandler::setDeviceStatus(const QString &serialNumber, const QString &status) const
{
    QSqlQuery query;
    query.prepare("UPDATE device SET status = ? WHERE serialnumber = ?");
    query.bindValue(0, status);
    query.bindValue(1, serialNumber);

    if(!query.exec())
    {
        throw std::runtime_error("Failed to set device status: " + query.lastError().text().toStdString());
    }
}

bool DatabaseHandler::checkDeviceStatus(const QString& serialNumber, const QString &status) const
{
    bool retVal = false;
    QSqlQuery query;
    query.prepare("SELECT * FROM device WHERE serialnumber = ? AND status = ?");
    query.bindValue(0, serialNumber);
    query.bindValue(1, status);

    if(query.exec())
    {
        if(query.next())
        {
            retVal = true;
        }
    }
    else
    {
        throw std::runtime_error("Failed to check device status: " + query.lastError().text().toStdString());
    }

    return retVal;
}
