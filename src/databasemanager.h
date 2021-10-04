#pragma once
#include <QObject>

class MsgEdge;
class MsgDevice;
class DatabaseHandler;
class DatabaseMqttClient;
//!
//! \brief The DatabaseManager class
//! The manager of the databasehandler component. It is responsible for the communication between the Mqtt client
//! and the databasehandler API.
//!
class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager( const QString& a_databaseName, QObject* a_parent = nullptr );

private slots:
    void edgeChanged( const QString& a_edgeId, const MsgEdge& a_sample );
    void edgeRemoved( const QString& a_edgeId );

    void deviceChanged( const QString& a_edgeId, const QString& a_deviceId, const MsgDevice& a_sample );
    void deviceRemoved( const QString& a_edgeId, const QString& a_deviceId, const QString& a_deviceSerial );

private:
    std::shared_ptr<DatabaseHandler> m_DatabaseHandler;
    std::shared_ptr<DatabaseMqttClient> m_MqttCient;
};
