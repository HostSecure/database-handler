#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H

#include <QString>

class DatabaseHandler
{
public:
    DatabaseHandler(const QString& databasePath);

    //TODO: Create description of timestamp format

    // All functions should be assumed to throw std::exceptions upon failure
    void registerEdgeNode(const QString& macAddress, bool isOnline, const QString& lastHeartbeatTimestamp) const;
    void registerDevice(const QString& serialNumber, const QString& vendorId, const QString& productId) const;
    void registerVendor(const QString& vendorId, const QString& vendorName);
    void registerProduct(const QString& productId, const QString& productName);
    void registerVirusHash(const QString& virusHash, const QString& description);

    void logEvent(const QString& edgeNodeMacAddress, const QString& deviceSerialNumber, const QString& timestamp, const QString& eventDescription);

    bool checkVirusHash(const QString& hash) const;

    void setEdgeNodeOnlineStatus(const QString& macAddress, bool isOnline, const QString& lastHeartbeatTimestamp = "");
    void getOnlineEdgeNodes(QVector<QString>& macAddresses);

    void setBlacklisted(const QString& serialNumber) const;
    bool isBlackListed(const QString& serialNumber) const;

    void setWhitelisted(const QString& serialNumber) const;
    bool isWhiteListed(const QString& serialNumber) const;

private:
    void setDeviceStatus(const QString& serialNumber, const QString& status) const;
    bool checkDeviceStatus(const QString& serialNumber, const QString& status) const;
};

#endif // DATABASEHANDLER_H
