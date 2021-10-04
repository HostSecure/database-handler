#include "databasehandler.h"
#include "databasedatafileparser.h"

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

namespace
{
   constexpr auto DEVICE_STATUS_UNKNOWN = "U";
   constexpr auto DEVICE_STATUS_WHITELISTED = "W";
   constexpr auto DEVICE_STATUS_BLACKLISTED = "B";
}

//!
//! \brief The DatabaseHandler constructor
//! Creates the database and its tables and populates the productvendor and virushash tables
//!
DatabaseHandler::DatabaseHandler(const QString& a_databasePath)
{
   bool exists = QFile::exists(a_databasePath);
   if(!exists)
   {
      QDir dir = QFileInfo(a_databasePath).absoluteDir();
      if(!dir.exists())
      {
         if(!dir.mkpath(dir.absolutePath()))
         {
            qCritical() << "Failed to create database path: " << dir.absolutePath();
            return;
         }
      }
   }

   QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
   db.setDatabaseName(a_databasePath);

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
         if(!query.exec("CREATE TABLE edgenode(macaddress VARCHAR(8) PRIMARY KEY, isonline BIT NOT NULL, lastheartbeat TIMESTAMP NOT NULL)"))
         {
            qFatal("Failed to create edgenode table: %s", query.lastError().text().toStdString().c_str());
         }

         if(!query.exec("CREATE TABLE virushash(hashkey VARCHAR(32) PRIMARY KEY, description VARCHAR(100))"))
         {
            qFatal("Failed to create virushash table: %s", query.lastError().text().toStdString().c_str());
         }

         if(!query.exec("CREATE TABLE productvendor(productid VARCHAR(4), vendorid VARCHAR(4), productname VARCHAR(30), vendorname VARCHAR(30), "
                        "PRIMARY KEY(productid, vendorid))"))
         {
            qFatal("Failed to create productvendor table: %s", query.lastError().text().toStdString().c_str());
         }

         // An auto ID is used to decrease the number of columns required in referencing tables
         if(!query.exec("CREATE TABLE device(id INTEGER PRIMARY KEY AUTOINCREMENT, productid VARCHAR(4), vendorid VARCHAR(4), serialnumber VARCHAR(8), status CHAR(1) NOT NULL, "
                        "UNIQUE(productid, vendorid, serialnumber), "
                        "FOREIGN KEY(productid, vendorid) REFERENCES productvendor(productid, vendorid))"))
         {
            qFatal("Failed to create device table: %s", query.lastError().text().toStdString().c_str());
         }

         if(!query.exec("CREATE TABLE connecteddevice(edgenodemacaddress VARCHAR(8), deviceid INTEGER, connecttime TIMESTAMP, "
                        "FOREIGN KEY (edgenodemacaddress) REFERENCES edgenode(macaddress), "
                        "FOREIGN KEY (deviceid) REFERENCES device(id), "
                        "PRIMARY KEY(edgenodemacaddress, deviceid))"))
         {
            qFatal("Failed to create connecteddevice table: %s", query.lastError().text().toStdString().c_str());
         }

         if(!query.exec("CREATE TABLE log(edgenodemacaddress VARCHAR(8), "
                        "deviceid INTEGER, "
                        "logtime TIMESTAMP, "
                        "loginfo VARCHAR(100), "
                        "FOREIGN KEY (edgenodemacaddress) REFERENCES edgenode(macaddress), "
                        "FOREIGN KEY (deviceid) REFERENCES device(id), "
                        "PRIMARY KEY(edgenodemacaddress, deviceid, logtime))"))
         {
            qFatal("Failed to create log table: %s", query.lastError().text().toStdString().c_str());
         }

         DatabaseDataFileParser::parseDeviceProductVendor(*this);
         DatabaseDataFileParser::parseVirusHash(*this);
      }
   }
}

//!
//! \brief The registerOrUpdateEdgeNode function
//! Registers a new Edge Node if it doesn't exist, or updates an existing one if it exists
//!
void DatabaseHandler::registerOrUpdateEdgeNode(const QString &a_macAddress, bool a_isOnline, const QString& a_lastHeartbeatTimestamp) const
{
   QSqlQuery query;
   // NOT an UPSERT, but it's ok as we update every attribute of the table and we don't use an auto id.
   query.prepare("INSERT OR REPLACE INTO edgenode(macaddress, isonline, lastheartbeat)"
                 "VALUES(?, ?, ?)");
   query.bindValue(0, a_macAddress);
   query.bindValue(1, (a_isOnline ? 1 : 0));
   query.bindValue(2, a_lastHeartbeatTimestamp);

   if(!query.exec())
   {
      qWarning() << __PRETTY_FUNCTION__ << "Failed to register edge node: " << query.lastError();
      throw std::runtime_error("Failed to register edge node");
   }
}

//!
//! \brief The getEdgeNode function
//! Retrieves an Edge Node
//!
bool DatabaseHandler::getEdgeNode(EdgeNode &a_edgeNode, const QString &a_macAddress) const
{
   bool success = false;
   QSqlQuery query;
   query.prepare("SELECT * FROM edgenode WHERE macaddress = ?");
   query.bindValue(0, a_macAddress);

   if(query.exec())
   {
      if(query.next())
      {
         a_edgeNode.macAddress = query.value(0).toString();
         a_edgeNode.isOnline = query.value(1).toBool();
         a_edgeNode.lastHeartbeat = query.value(2).toString();
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

//!
//! \brief The getAllEdgeNodeKeys function
//! Retrieves all Edge Node mac addresses
//!
void DatabaseHandler::getAllEdgeNodeKeys(QVector<QString> &a_macAddresses) const
{
   try
   {
      getKeysFromTable("macaddress", "edgenode",  a_macAddresses);
   }
   catch(std::exception& e)
   {
      qCritical() <<  __PRETTY_FUNCTION__ << e.what();
      throw e;
   }
}

//!
//! \brief The getAllEdgeNodes function
//! Retrieves all Eedge Nodes
//!
void DatabaseHandler::getAllEdgeNodes(std::vector<std::unique_ptr<EdgeNode> > &a_edgeNodes) const
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
         a_edgeNodes.push_back(std::move(node));
      }
   }
   else
   {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to get all edge nodes: " << query.lastError();
      throw std::runtime_error("Failed to get all edge nodes");
   }
}

//!
//! \brief The setEdgeNodeOnlineStatus function
//! Updates the online status of an Edge Node
//!
void DatabaseHandler::setEdgeNodeOnlineStatus(const QString &a_macAddress, bool a_isOnline, const QString &a_lastHeartbeatTimestamp)
{
   QSqlQuery query;
   if(a_lastHeartbeatTimestamp.isEmpty())
   {
      query.prepare("UPDATE edgenode SET isonline = ? WHERE macAddress = ?");
      query.bindValue(0, (a_isOnline ? 1 : 0));
      query.bindValue(1, a_macAddress);
   }
   else
   {
      query.prepare("UPDATE edgenode SET isonline = ?, lastheartbeat = ? WHERE macAddress = ?");
      query.bindValue(0, (a_isOnline ? 1 : 0));
      query.bindValue(1, a_lastHeartbeatTimestamp);
      query.bindValue(2, a_macAddress);
   }

   if(!query.exec())
   {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to set edge node online status: " << query.lastError();
      throw std::runtime_error("Failed to set edge node online status");
   }
}

//!
//! \brief The getOnlineEdgeNodes function
//! Retrieves all the online Edge Nodes
//!
void DatabaseHandler::getOnlineEdgeNodes(QVector<QString> &a_macAddresses) const
{
   QSqlQuery query;

   if(query.exec("SELECT macaddress FROM edgenode WHERE isonline = 1"))
   {
      while (query.next())
      {
         a_macAddresses.push_back( query.value(0).toString());
      }
   }
   else
   {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to set edge node online status: " << query.lastError();
      throw std::runtime_error("Failed to set edge node online status");
   }
}

//!
//! \brief The registerDevice function
//! Registers a Device if it does not already exist
//!
void DatabaseHandler::registerDevice(const QString &a_productId, const QString &a_vendorId, const QString &a_serialNumber ) const
{
   QSqlQuery query;

   // Ignore if it exists
   query.prepare("INSERT INTO device(productid, vendorid, serialnumber, status) "
                 "SELECT ?, ?, ?, ? "
                 "WHERE NOT EXISTS(SELECT * "
                 "FROM device "
                 "WHERE productid = ? AND vendorid = ? AND serialnumber = ?)");
   query.bindValue(0, a_productId);
   query.bindValue(1, a_vendorId);
   query.bindValue(2, a_serialNumber);
   query.bindValue(3, DEVICE_STATUS_UNKNOWN);
   query.bindValue(4, a_productId);
   query.bindValue(5, a_vendorId);
   query.bindValue(6, a_serialNumber);

   if(!query.exec())
   {
      qWarning() << __PRETTY_FUNCTION__ << "Failed to register device: " << query.lastError();
      throw std::runtime_error("Failed to register device");
   }
}

//!
//! \brief The getDevice function
//! Retrieves a Device
//!
bool DatabaseHandler::getDevice(Device &a_device, const QString &a_productId, const QString &a_vendorId, const QString &a_serialNumber) const
{
   bool success = false;
   QSqlQuery query;
   query.prepare("SELECT productid, vendorid, serialnumber FROM device WHERE productid = ? AND vendorid = ? AND serialnumber = ?");
   query.bindValue(0, a_productId);
   query.bindValue(1, a_vendorId);
   query.bindValue(2, a_serialNumber);

   if(query.exec())
   {
      if(query.next())
      {
         a_device.productId = query.value(0).toString();
         a_device.vendorId = query.value(1).toString();
         a_device.serialNumber = query.value(2).toString();
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

//!
//! \brief The getAllDevices function
//! Retrieves all Devices
//!
void DatabaseHandler::getAllDevices(std::vector<std::unique_ptr<Device> > &a_devices) const
{
   QSqlQuery query;
   if(query.exec("SELECT productid, vendorid, serialnumber FROM device"))
   {
      while(query.next())
      {
         std::unique_ptr<Device> device =  std::make_unique<Device>();
         device->productId = query.value(0).toString();
         device->vendorId = query.value(1).toString();
         device->serialNumber = query.value(2).toString();
         a_devices.push_back(std::move(device));
      }
   }
   else
   {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to get all devices: " << query.lastError();
      throw std::runtime_error("Failed to get all devices");
   }
}

//!
//! \brief The setDeviceBlacklisted function
//! Updates the Device status to Blacklisted
//!
void DatabaseHandler::setDeviceBlacklisted(const QString& a_productId, const QString& a_vendorId, const QString& a_serialNumber) const
{
   try
   {
      setDeviceStatus(a_productId, a_vendorId, a_serialNumber, DEVICE_STATUS_BLACKLISTED);
   }
   catch (std::exception& e)
   {
      qCritical() <<  __PRETTY_FUNCTION__ << e.what();
      throw e; // Forward the exception so it can be handled correctly by the caller";
   }
}

//!
//! \brief The isDeviceBlackListed function
//! Checks if a Device is Blacklisted
//!
bool DatabaseHandler::isDeviceBlackListed(const QString& a_productId, const QString& a_vendorId, const QString& a_serialNumber) const
{
   bool retVal = true; // Assume the worst
   try
   {
      retVal = checkDeviceStatus(a_productId, a_vendorId, a_serialNumber, DEVICE_STATUS_BLACKLISTED);
   }
   catch(std::exception& e)
   {
      qCritical() <<  __PRETTY_FUNCTION__ << e.what();
      throw e; // Forward the exception so it can be handled correctly by the caller";
   }

   return retVal;
}

//!
//! \brief The setDeviceWhitelisted function
//! Updates the Device status to Whitelisted
//!
void DatabaseHandler::setDeviceWhitelisted(const QString& a_productId, const QString& a_vendorId, const QString& a_serialNumber) const
{
   try
   {
      setDeviceStatus(a_productId, a_vendorId, a_serialNumber, DEVICE_STATUS_WHITELISTED);
   }
   catch (std::exception& e)
   {
      qCritical() <<  __PRETTY_FUNCTION__ << e.what();
      throw e; // Forward the exception so it can be handled correctly by the caller";
   }
}

//!
//! \brief The isDeviceWhiteListed function
//! Checks if a Device is Whitelisted
//!
bool DatabaseHandler::isDeviceWhiteListed(const QString& a_productId, const QString& a_vendorId, const QString& a_serialNumber) const
{
   bool retVal = false;
   try
   {
      retVal = checkDeviceStatus(a_productId, a_vendorId, a_serialNumber, DEVICE_STATUS_WHITELISTED);
   }
   catch(std::exception& e)
   {
      qCritical() <<  __PRETTY_FUNCTION__ << e.what();
      throw e; // Forward the exception so it can be handled correctly by the caller";
   }

   return retVal;
}

//!
//! \brief The registerConnectedDevice function
//! Registers a connection between a Device and the Edge Node it is connected to
//!
void DatabaseHandler::registerConnectedDevice(const QString &a_edgeNodeMacAddress, const QString &a_deviceProductId, const QString &a_deviceVendorId, const QString &a_deviceSerialNumber, const QString &a_timestamp)
{
   QSqlQuery query;
   query.prepare("INSERT INTO connecteddevice(edgenodemacaddress, deviceid, connecttime) "
                 "SELECT ?, device.id, ? "
                 "FROM device "
                 "WHERE device.productid = ? AND device.vendorid = ? AND device.serialnumber = ?");
   query.bindValue(0, a_edgeNodeMacAddress);
   query.bindValue(1, a_timestamp);
   query.bindValue(2, a_deviceProductId);
   query.bindValue(3, a_deviceVendorId);
   query.bindValue(4, a_deviceSerialNumber);

   if(!query.exec())
   {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to register connected device: " << query.lastError();
      throw std::runtime_error("Failed to register connected device");
   }
}

//!
//! \brief The unregisterConnectedDevicesOnEdgeNode function
//! Unregisters all Device connections to an Edge Node
//!
void DatabaseHandler::unregisterConnectedDevicesOnEdgeNode(const QString &a_edgeNodeMacAddress)
{
   QSqlQuery query;
   query.prepare("DELETE FROM connecteddevice "
                 "WHERE edgenodemacaddress = ?");
   query.bindValue(0, a_edgeNodeMacAddress);

   if(!query.exec())
   {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to unregister all connected devices on edge node: " << query.lastError();
      throw std::runtime_error("Failed to unregister all connected devices on edge node");
   }
}

//!
//! \brief The unregisterConnectedDevice function
//! Unregisters a connection between a Device and the Edge Node it was connected to
//!
void DatabaseHandler::unregisterConnectedDevice(const QString &a_edgeNodeMacAddress, const QString &a_deviceProductId, const QString &a_deviceVendorId, const QString &a_deviceSerialNumber)
{
   QSqlQuery query;
   query.prepare("DELETE FROM connecteddevice "
                 "WHERE edgenodemacaddress = ? AND deviceid = (SELECT id FROM device WHERE productid = ? AND vendorid = ? AND serialnumber = ?)");
   query.bindValue(0, a_edgeNodeMacAddress);
   query.bindValue(1, a_deviceProductId);
   query.bindValue(2, a_deviceVendorId);
   query.bindValue(3, a_deviceSerialNumber);

   if(!query.exec())
   {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to unregister connected device: " << query.lastError();
      throw std::runtime_error("Failed to unregister connected device");
   }
}

//!
//! \brief The registerConnectedDevice function
//! Retrieves all connnections between Devicees and Edge Nodes
//!
void DatabaseHandler::getAllConnectedDevices(std::vector<std::unique_ptr<ConnectedDevice> > &a_connectedDevices)
{
   QSqlQuery query;
   if(query.exec("SELECT edgenodemacaddress, productid, vendorid, serialnumber "
                 "FROM connecteddevice "
                 "INNER JOIN device ON device.id = connecteddevice.deviceid"))
   {
      while(query.next())
      {
         std::unique_ptr<ConnectedDevice> connectedDevice =  std::make_unique<ConnectedDevice>();
         connectedDevice->connectedEdgeNodeMacAddress = query.value(0).toString();
         connectedDevice->deviceProductId = query.value(1).toString();
         connectedDevice->deviceVendorId = query.value(2).toString();
         connectedDevice->deviceSerialNumber = query.value(3).toString();
         a_connectedDevices.push_back(std::move(connectedDevice));
      }
   }
   else
   {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to get all connected devices: " << query.lastError();
      throw std::runtime_error("Failed to get all connected devices");
   }
}

//!
//! \brief The registerProductVendor function
//! Registers a product vendor combination
//!
void DatabaseHandler::registerProductVendor(const QString &a_productId, const QString &a_productName, const QString &a_vendorId, const QString &a_vendorName)
{
   QSqlQuery query;
   query.prepare("INSERT INTO productvendor(productid, vendorid, productname, vendorname)"
                 "VALUES(?, ?, ?, ?)");
   query.bindValue(0, a_productId);
   query.bindValue(1, a_vendorId);
   query.bindValue(2, a_productName);
   query.bindValue(3, a_vendorName);

   if(!query.exec())
   {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to register productvendor: " << query.lastError();
      throw std::runtime_error("Failed to register productvendor");
   }
}

//!
//! \brief The getProductVendor function
//! Retrieves a product vendor combination
//!
bool DatabaseHandler::getProductVendor(ProductVendor& a_productVendor, const QString &a_productId, const QString a_vendorId)
{
   bool success = false;
   QSqlQuery query;
   query.prepare("SELECT * FROM productvendor WHERE productid = ? AND vendorid = ?");
   query.bindValue(0, a_productId);
   query.bindValue(1, a_vendorId);

   if(query.exec())
   {
      if(query.next())
      {
         a_productVendor.productId = query.value(0).toString();
         a_productVendor.vendorId = query.value(1).toString();
         a_productVendor.productName = query.value(2).toString();
         a_productVendor.vendorName = query.value(3).toString();
         success = true;
      }
   }
   else
   {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to get productvendor: " << query.lastError();
      throw std::runtime_error("Failed to get productvendor");
   }

   return success;
}

//!
//! \brief The getAllProductVendors function
//! Retrieves all product vendor combination
//!
void DatabaseHandler::getAllProductVendors(std::vector<std::unique_ptr<ProductVendor> >& a_productVendors)
{
   QSqlQuery query;
   if(query.exec("SELECT * FROM productvendor"))
   {
      while(query.next())
      {
         std::unique_ptr<ProductVendor> productVendor =  std::make_unique<ProductVendor>();
         productVendor->productId = query.value(0).toString();
         productVendor->vendorId = query.value(1).toString();
         productVendor->productName = query.value(2).toString();
         productVendor->vendorName = query.value(3).toString();
         a_productVendors.push_back(std::move(productVendor));
      }
   }
   else
   {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to get all productvendors: " << query.lastError();
      throw std::runtime_error("Failed to get all productvendors");
   }
}

//!
//! \brief The registerVirusHash function
//! Registers a virus file hash
//!
void DatabaseHandler::registerVirusHash(const QString &virusHash, const QString &a_description)
{
   QSqlQuery query;
   query.prepare("INSERT INTO virushash(hashkey, description)"
                 "VALUES(?, ?)");
   query.bindValue(0, virusHash);
   query.bindValue(1, a_description);

   if(!query.exec())
   {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to register virus hash: " << query.lastError();
      throw std::runtime_error("Failed to register virus hash");
   }
}

//!
//! \brief The getVirusHash function
//! Retrieves a virus hash with description
//!
bool DatabaseHandler::getVirusHash(VirusHash &a_vHash, const QString &a_virusHash) const
{
   bool success = false;
   QSqlQuery query;
   query.prepare("SELECT * FROM virushash WHERE hashkey = ?");
   query.bindValue(0, a_virusHash);

   if(query.exec())
   {
      if(query.next())
      {
         a_vHash.virusHash = query.value(0).toString();
         a_vHash.description = query.value(1).toString();
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

//!
//! \brief The getAllVirusHashKeys function
//! Retrieves all virus hashes
//!
void DatabaseHandler::getAllVirusHashKeys(QVector<QString> &a_virusHashes) const
{
   try
   {
      getKeysFromTable("hashkey", "virushash",  a_virusHashes);
   }
   catch(std::exception& e)
   {
      qCritical() <<  __PRETTY_FUNCTION__ << e.what();
      throw e;
   }
}

//!
//! \brief The getAllVirusHashes function
//! Retrieves all virus hashes with descriptions
//!
void DatabaseHandler::getAllVirusHashes(std::vector<std::unique_ptr<VirusHash> > &a_virusHashes) const
{
   QSqlQuery query;
   if(query.exec("SELECT * FROM virushash"))
   {
      while(query.next())
      {
         std::unique_ptr<VirusHash> virusHash =  std::make_unique<VirusHash>();
         virusHash->virusHash = query.value(0).toString();
         virusHash->description = query.value(1).toString();
         a_virusHashes.push_back(std::move(virusHash));
      }
   }
   else
   {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to get all virus hashes: " << query.lastError();
      throw std::runtime_error("Failed to get all virus hashes");
   }
}

//!
//! \brief The isHashInVirusDatabase function
//! Checks if a virus hash can be found in the database
//!
bool DatabaseHandler::isHashInVirusDatabase(const QString &a_hash) const
{
   bool found = true; // Assume the worst
   QSqlQuery query;
   query.prepare("SELECT * FROM virushash WHERE hashkey = ?");
   query.bindValue(0, a_hash);

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

//!
//! \brief The logEvent function
//! Logs an event related to a given Device on a given Edge Node
//!
void DatabaseHandler::logEvent(const QString& edgeNodeMacAddress, const QString& a_deviceProductId, const QString& a_deviceVendorId, const QString& a_deviceSerialNumber, const QString& a_timestamp, const QString& a_eventDescription)
{
   QSqlQuery query;
   query.prepare("INSERT INTO log(edgenodemacaddress, deviceid, logtime, loginfo) "
                 "SELECT ?, device.id, ?, ? "
                 "FROM device "
                 "WHERE device.productid = ? AND device.vendorid = ? AND device.serialnumber = ?");
   query.bindValue(0, edgeNodeMacAddress);
   query.bindValue(1, a_timestamp);
   query.bindValue(2, a_eventDescription);
   query.bindValue(3, a_deviceProductId);
   query.bindValue(4, a_deviceVendorId);
   query.bindValue(5, a_deviceSerialNumber);

   if(!query.exec())
   {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to log event: " << query.lastError();
      throw std::runtime_error("Failed to log event");
   }
}

//!
//! \brief The getLoggedEvent function
//! Retrieves a specific logged event
//!
bool DatabaseHandler::getLoggedEvent(LogEvent& logEvent, const QString& a_edgeNodeMacAddress, const QString& a_deviceProductId, const QString& a_deviceVendorId, const QString& a_deviceSerialNumber, const QString& a_timestamp) const
{
   bool success = false;
   QSqlQuery query;
   query.prepare("SELECT edgenodemacaddress, productid, vendorid, serialnumber, logtime, loginfo "
                 "FROM log "
                 "INNER JOIN device ON device.id = log.deviceid "
                 "WHERE edgenodemacaddress = ? "
                 "AND deviceid = (SELECT id "
                 "FROM device "
                 "WHERE productid = ? AND vendorid = ? AND serialnumber = ?) "
                 "AND logtime = ? ");
   query.bindValue(0, a_edgeNodeMacAddress);
   query.bindValue(1, a_deviceProductId);
   query.bindValue(2, a_deviceVendorId);
   query.bindValue(3, a_deviceSerialNumber);
   query.bindValue(4, a_timestamp);

   if(query.exec())
   {
      if(query.next())
      {
         logEvent.edgeNodeMacAddress = query.value(0).toString();
         logEvent.deviceProductId = query.value(1).toString();
         logEvent.deviceVendorId = query.value(2).toString();
         logEvent.deviceSerialNumber = query.value(3).toString();
         logEvent.timestamp = query.value(4).toString();
         logEvent.eventDescription = query.value(5).toString();
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

//!
//! \brief The getAllLoggedEvents function
//! Retrieves all logged events.
//!
void DatabaseHandler::getAllLoggedEvents(std::vector<std::unique_ptr<LogEvent> >& a_loggedEvents) const
{
   QSqlQuery query;
   if(query.exec("SELECT edgenodemacaddress, productid, vendorid, serialnumber, logtime, loginfo "
                 "FROM log "
                 "INNER JOIN device ON device.id = log.deviceid"))
   {
      while(query.next())
      {
         std::unique_ptr<LogEvent> logEvent =  std::make_unique<LogEvent>();
         logEvent->edgeNodeMacAddress = query.value(0).toString();
         logEvent->deviceProductId = query.value(1).toString();
         logEvent->deviceVendorId = query.value(2).toString();
         logEvent->deviceSerialNumber = query.value(3).toString();
         logEvent->timestamp = query.value(4).toString();
         logEvent->eventDescription = query.value(5).toString();
         a_loggedEvents.push_back(std::move(logEvent));
      }
   }
   else
   {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to get all logged events: " << query.lastError();
      throw std::runtime_error("Failed to get all logged events");
   }
}

//!
//! \brief The getKeysFromTable function
//! Helper function to retrieve the VARCHAR keys if a given table
//!
void DatabaseHandler::getKeysFromTable(const QString a_keyName, const QString &a_tableName, QVector<QString> &a_result) const
{
   QSqlQuery query;
   query.prepare(QString("SELECT %1 FROM %2").arg(a_keyName, a_tableName));

   if(query.exec())
   {
      while (query.next())
      {
         a_result.push_back( query.value(0).toString());
      }
   }
   else
   {
      throw std::runtime_error("Failed to get keys from table: " + query.lastError().text().toStdString());
   }
}

//!
//! \brief The setDeviceStatus function
//! Helper function to set the status of a given Device
//!
void DatabaseHandler::setDeviceStatus(const QString& a_productId, const QString& a_vendorId, const QString& a_serialNumber, const QString& a_status) const
{
   QSqlQuery query;
   query.prepare("UPDATE device SET status = ? WHERE productid = ? AND vendorid = ? AND serialnumber = ?");
   query.bindValue(0, a_status);
   query.bindValue(1, a_productId);
   query.bindValue(2, a_vendorId);
   query.bindValue(3, a_serialNumber);

   if(!query.exec())
   {
      throw std::runtime_error("Failed to set device status: " + query.lastError().text().toStdString());
   }
}

//!
//! \brief The checkDeviceStatus function
//! Helper function to check the status of a given Device
//!
bool DatabaseHandler::checkDeviceStatus(const QString& a_productId, const QString& a_vendorId, const QString& a_serialNumber, const QString& a_status) const
{
   bool retVal = false;
   QSqlQuery query;
   query.prepare("SELECT * FROM device WHERE productId = ? AND vendorid = ? AND serialnumber = ? AND status = ?");
   query.bindValue(0, a_productId);
   query.bindValue(1, a_vendorId);
   query.bindValue(2, a_serialNumber);
   query.bindValue(3, a_status);

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
