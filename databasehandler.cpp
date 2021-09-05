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
        if(!exists)
        {
            QSqlQuery query;
//            query.exec(CREATE + EDGENODE + "(" + EDGENODEID + " " + EDGENODEIDTYPE + " " + PRIMARY + ", " + EDGENODEDESCRIPTTION + " " + EDGENODEDESCRIPTIONTYPE + ")");
            query.exec("CREATE TABLE edgenode(macaddress VARCHAR(8) PRIMARY KEY, isonline BIT NOT NULL, lastheartbeat TIMESTAMP NOT NULL)");
//            query.exec(CREATE + VENDOR + "(" + VENDORID + " " + VENDORIDTYPE + " " + PRIMARY + ", " + VENDORNAME + " " + VENDORNAMETYPE + ")");
            query.exec("CREATE TABLE vendor(vendorid VARCHAR(4) PRIMARY KEY, vendorname VARCHAR(30))");
            query.exec("CREATE TABLE product(productid VARCHAR(4) PRIMARY KEY, productname VARCHAR(30))");
            query.exec("CREATE TABLE virushash(hashkey VARCHAR(32) PRIMARY KEY, description VARCHAR(100))");

            query.exec("CREATE TABLE devicedata(productid VARCHAR(4), vendorid VARCHAR(4), "
                                               "FOREIGN KEY(productid) REFERENCES product(productid), "
                                               "FOREIGN KEY (vendorid) REFERENCES vendor(vendorid), "
                                               "PRIMARY KEY(productid, vendorid))");

            query.exec("CREATE TABLE device(serialnumber VARCHAR(8) PRIMARY KEY, productid VARCHAR(4) NOT NULL, vendorid VARCHAR(4) NOT NULL, status CHAR(1) NOT NULL, "
                                           "FOREIGN KEY(productid) REFERENCES devicedata(productid), "
                                           "FOREIGN KEY(vendorid) REFERENCES devicedata(vendorid))");

            query.exec("CREATE TABLE log(edgenodemacaddress VARCHAR(8), "
                                        "deviceid VARCHAR(8), "
                                        "logtime TIMESTAMP, "
                                        "loginfo VARCHAR(100), "
                                        "FOREIGN KEY (edgenodemacaddress) REFERENCES edgenode(macaddress), "
                                        "FOREIGN KEY (deviceid) REFERENCES device(serialnumber), "
                                        "PRIMARY KEY(edgenodemacaddress, deviceid, logtime))");
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

void DatabaseHandler::registerDevice(const QString &serialNumber, const QString &vendorId, const QString &productId) const
{
    QSqlQuery query;
    query.prepare("INSERT INTO device(serialnumber, productid, vendorid, status)"
                  "VALUES(?, ?, ?, ?");
    query.bindValue(0, serialNumber);
    query.bindValue(1, vendorId);
    query.bindValue(2, productId);
    query.bindValue(3, DEVICE_STATUS_UNKNOWN);

    if(!query.exec())
    {
        qWarning() << __PRETTY_FUNCTION__ << "Failed to register device: " << query.lastError();
        throw std::runtime_error("Failed to register device");
    }
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

void DatabaseHandler::logEvent(const QString &edgeNodeMacAddress, const QString &deviceSerialNumber, const QString &timestamp, const QString &eventDescription)
{
    QSqlQuery query;
    // TODO: Need to format logtime correctly somewhere
    query.prepare("INSERT INTO log(edgenodemacaddress, deviceid, logtime, loginfo)"
                  "VALUES(?, ?, ?, ?, ?");
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

bool DatabaseHandler::checkVirusHash(const QString &hash) const
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

void DatabaseHandler::getOnlineEdgeNodes(QVector<QString> &macAddresses)
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

void DatabaseHandler::setBlacklisted(const QString &serialNumber) const
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

bool DatabaseHandler::isBlackListed(const QString &serialNumber) const
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

void DatabaseHandler::setWhitelisted(const QString &serialNumber) const
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

bool DatabaseHandler::isWhiteListed(const QString &serialNumber) const
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

void DatabaseHandler::setDeviceStatus(const QString &serialNumber, const QString &status) const
{
    QSqlQuery query;
    query.prepare("UPDATE device SET status = ? WHERE serialnumber = ?");
    query.bindValue(0, status);
    query.bindValue(1, serialNumber);

    if(!query.exec())
    {
//        if(query.lastError().type() != QSqlError::NoError)
//        {
            throw std::runtime_error("Failed to set device status: " + query.lastError().text().toStdString());
//        }
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
//        if(query.lastError().type() != QSqlError::NoError)
//        {
            throw std::runtime_error("Failed to check device status: " + query.lastError().text().toStdString());
//        }
    }

    return retVal;
}
