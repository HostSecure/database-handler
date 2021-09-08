#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H

#include <QString>

class QSqlQuery;

class DatabaseHandler
{
public:
    DatabaseHandler(const QString& databasePath);

    // Timestamp format is "YYYY:MM:DD [HH:MM:SS.SSS]".
    // All functions should be assumed to throw std::exceptions upon failure

    // Edge node
    struct EdgeNode
    {
        QString macAddress = "";
        bool isOnline = false;
        QString lastHeartbeat = "";
    };
    void registerEdgeNode(const QString& macAddress, bool isOnline, const QString& lastHeartbeatTimestamp) const;
    bool getEdgeNode(const QString& macAddress, EdgeNode& edgeNode) const;
    void getAllEdgeNodeKeys(QVector<QString>& macAddresses) const;
    void getAllEdgeNodes(std::vector<std::unique_ptr<EdgeNode>>& edgeNodes) const;
    void setEdgeNodeOnlineStatus(const QString& macAddress, bool isOnline, const QString& lastHeartbeatTimestamp = "");
    void getOnlineEdgeNodes(QVector<QString>& macAddresses) const;

    //Device
    struct Device
    {
        QString serialNumber = "";
        QString vendorId = "";
        QString productId = "";
    };
    void registerDevice(const QString& serialNumber, const QString& vendorId, const QString& productId) const;
    bool getDevice(const QString& serialNumber, Device& device) const;
    void getAllDeviceKeys(QVector<QString>& serialNumbers) const;
    void getAllDevices(std::vector<std::unique_ptr<Device>>& devices) const;
    void setDeviceBlacklisted(const QString& serialNumber) const;
    bool isDeviceBlackListed(const QString& serialNumber) const;
    void setDeviceWhitelisted(const QString& serialNumber) const;
    bool isDeviceWhiteListed(const QString& serialNumber) const;

    // Vendor
    struct Vendor
    {
        QString vendorId = "";
        QString vendorName = "";
    };
    void registerVendor(const QString& vendorId, const QString& vendorName);
    bool getVendor(const QString& vendorId, Vendor& vendor) const;
    void getAllVendorKeys(QVector<QString>& vendorIds) const;
    void getAllVendors(std::vector<std::unique_ptr<Vendor>> vendors) const;

    //Product
    struct Product
    {
        QString productId = "";
        QString productName = "";
    };
    void registerProduct(const QString& productId, const QString& productName);
    bool getProduct(const QString& productId, Product& product) const;
    void getAllProductKeys(QVector<QString>& productIds) const;
    void getAllProducts(std::vector<std::unique_ptr<Product>> products) const;

    // Virus
    struct VirusHash
    {
        QString virusHash = "";
        QString description = "";
    };
    void registerVirusHash(const QString& virusHash, const QString& description);
    bool getVirusHash(const QString& virusHash, VirusHash& vHash) const;
    void getAllVirusHashKeys(QVector<QString>& virusHashes) const;
    void getAllVirusHashes(std::vector<std::unique_ptr<VirusHash>>& virusHashes) const ;
    bool isVirusHashInDatabase(const QString& hash) const;

    // Event logging
    struct LogEvent
    {
        QString edgeNodeMacAddress = "";
        QString deviceSerialNumber = "";
        QString timestamp = "";
        QString eventDescription = "";
    };
    void logEvent(const QString& edgeNodeMacAddress, const QString& deviceSerialNumber, const QString& timestamp, const QString& eventDescription);
    bool getLoggedEvent(const QString& edgeNodeMacAddress, const QString deviceSerialNumber, const QString& timestamp, LogEvent& logEvent) const;
    void getAllLoggedEvents(std::vector<std::unique_ptr<LogEvent>> loggedEvents) const;

private:
    void getKeysFromTable(const QString keyName, const QString& tableName, QVector<QString>& result) const;
    void setDeviceStatus(const QString& serialNumber, const QString& status) const;
    bool checkDeviceStatus(const QString& serialNumber, const QString& status) const;
};

#endif // DATABASEHANDLER_H
