#pragma once
#include <QString>

class QSqlQuery;
//!
//! \brief The DatabaseHandler class
//! Creates the required database tables and provides an API to run predefined queries
//! All functions should be assumed to throw an std::exception upon failure
//!
class DatabaseHandler
{
public:
    DatabaseHandler( const QString& a_databasePath );

    // Edge node
    struct EdgeNode
    {
        QString macAddress = "";
        bool isOnline = false;
        QString lastHeartbeat = "";
    };
    void registerOrUpdateEdgeNode(const QString& a_macAddress, bool a_isOnline, const QString& a_lastHeartbeatTimestamp) const;
    bool getEdgeNode(EdgeNode& a_edgeNode, const QString& a_macAddress) const;
    void getAllEdgeNodeKeys(QVector<QString>& a_macAddresses) const;
    void getAllEdgeNodes(std::vector<std::unique_ptr<EdgeNode>>& a_edgeNodes) const;
    void setEdgeNodeOnlineStatus(const QString& a_macAddress, bool a_isOnline, const QString& a_lastHeartbeatTimestamp = "");
    void getOnlineEdgeNodes(QVector<QString>& a_macAddresses) const;

    //Device
    struct Device
    {
        QString vendorId = "";
        QString productId = "";
        QString serialNumber = "";
    };
    void registerDevice(const QString& a_productId, const QString& a_vendorId, const QString& a_serialNumber) const;
    bool getDevice(Device& a_device, const QString& a_productId, const QString& a_vendorId, const QString& a_serialNumber) const;
    void getAllDevices(std::vector<std::unique_ptr<Device>>& a_devices) const;
    void setDeviceBlacklisted(const QString& a_productId, const QString& a_vendorId, const QString& a_serialNumber) const;
    bool isDeviceBlackListed(const QString& a_productId, const QString& a_vendorId, const QString& a_serialNumber) const;
    void setDeviceWhitelisted(const QString& a_productId, const QString& a_vendorId, const QString& a_serialNumber) const;
    bool isDeviceWhiteListed(const QString& a_productId, const QString& a_vendorId, const QString& a_serialNumber) const;

    // Connected devices
    struct ConnectedDevice
    {
        QString connectedEdgeNodeMacAddress = "";
        QString deviceProductId = "";
        QString deviceVendorId = "";
        QString deviceSerialNumber = "";
    };
    void registerConnectedDevice(const QString& a_edgeNodeMacAddress, const QString& a_deviceProductId, const QString& a_deviceVendorId, const QString& a_deviceSerialNumber, const QString& a_timestamp);
    void unregisterConnectedDevicesOnEdgeNode(const QString& a_edgeNodeMacAddress);
    void unregisterConnectedDevice(const QString& a_edgeNodeMacAddress, const QString& a_deviceProductId, const QString& a_deviceVendorId, const QString& a_deviceSerialNumber);
    void getAllConnectedDevices(std::vector<std::unique_ptr<ConnectedDevice>>& a_connectedDevices);


    // ProductVendor
    struct ProductVendor
    {
        QString productId = "";
        QString productName = "";
        QString vendorId = "";
        QString vendorName = "";
    };
    void registerProductVendor(const QString& a_productId, const QString& a_productName, const QString& a_vendorId, const QString& a_vendorName);
    bool getProductVendor(ProductVendor& a_productVendor, const QString& a_productId, const QString a_vendorId);
    void getAllProductVendors(std::vector<std::unique_ptr<ProductVendor>>& a_productVendors);

    // Virus
    struct VirusHash
    {
        QString virusHash = "";
        QString description = "";
    };
    void registerVirusHash(const QString& a_virusHash, const QString& a_description);
    bool getVirusHash(VirusHash& a_vHash, const QString& a_virusHash) const;
    void getAllVirusHashKeys(QVector<QString>& a_virusHashes) const;
    void getAllVirusHashes(std::vector<std::unique_ptr<VirusHash>>& a_virusHashes) const ;
    bool isHashInVirusDatabase(const QString& a_hash) const;

    // Event logging
    struct LogEvent
    {
        QString edgeNodeMacAddress = "";
        QString deviceProductId = "";
        QString deviceVendorId = "";
        QString deviceSerialNumber = "";
        QString timestamp = "";
        QString eventDescription = "";
    };
    void logEvent(const QString& a_edgeNodeMacAddress, const QString& a_deviceProductId, const QString& a_deviceVendorId, const QString& a_deviceSerialNumber, const QString& a_timestamp, const QString& a_eventDescription);
    bool getLoggedEvent(LogEvent& a_logEvent, const QString& a_edgeNodeMacAddress, const QString& a_deviceProductId, const QString& a_deviceVendorId, const QString& a_deviceSerialNumber, const QString& a_timestamp) const;
    void getAllLoggedEvents(std::vector<std::unique_ptr<LogEvent>>& a_loggedEvents) const;

private:
    void getKeysFromTable(const QString a_keyName, const QString& a_tableName, QVector<QString>& a_result) const;
    void setDeviceStatus(const QString& a_productId, const QString& a_vendorId, const QString& a_serialNumber, const QString& a_status) const;
    bool checkDeviceStatus(const QString& a_productId, const QString& a_vendorId, const QString& a_serialNumber, const QString& a_status) const;
};
