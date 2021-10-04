#include "databasemqttclient.h"
#include <QJsonDocument>

//!
//! \brief The DatabaseMqttClient constructor
//!  Automatically connects to the Mqtt system
//!
DatabaseMqttClient::DatabaseMqttClient( QObject* a_parent )
   : MqttClientBase { a_parent }
{
   connectToHost();
}

//!
//! \brief The brokerConnected function
//!  Called when the client connects to the Mqtt system.
//!  Sets up subscriptions to relevant updates
//!
void DatabaseMqttClient::brokerConnected()
{
   const QMqttTopicFilter topicFilter( "edges/#" ); //! Subscribe with wildcard
   auto edgeSub = subscribe( topicFilter );
   if ( edgeSub != nullptr )
   {
      QObject::connect( edgeSub, &QMqttSubscription::messageReceived,
                        this, &DatabaseMqttClient::incomingEdge );
   }
}

//!
//! \brief The incomingEdge function
//!  Called when an Edge Node related message is received
//!  Parses the message to see if its related to the Edge Node as a whole,
//!  or to a specific Device connected to the Edge Node, and forwards the data accordingly
//!
void DatabaseMqttClient::incomingEdge( QMqttMessage a_sample )
{
   QJsonDocument document = QJsonDocument::fromJson( a_sample.payload() );
   const auto levelCount = a_sample.topic().levelCount();
   const auto levels = a_sample.topic().levels();

   if ( levelCount >= 2 )
   {
      const auto edgeName = levels.at( 1 );

      if ( levelCount == 2 )
      {
         MsgEdge sample;
         if ( sample.fromJson( document.object() ) )
            emit edgeChanged( edgeName, sample );
         else
            emit edgeRemoved( edgeName );
      }
      else if ( levelCount == 3 )
      {
         const auto deviceId = levels.at( 2 );

         MsgDevice sample;
         if ( sample.fromJson( document.object() ) )
            emit deviceChanged( edgeName, deviceId, sample );
         else
            emit deviceRemoved( edgeName, deviceId, sample.deviceSerial );
      }
   }
}
