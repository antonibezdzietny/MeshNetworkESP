#include <Arduino.h>
#include <painlessMesh.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

//TODO If you want do same network changes use mesh_configuration.h
//Include specify configuration for this module
#include "configuration.h"
#include "mesh_logger.h"
#include "messageQueue.hpp"

//WiFi & MQTT client 
WiFiClient wifiClient;
PubSubClient mqttClient( MQTT_SERVER, MQTT_PORT, wifiClient );
IPAddress myIP(0,0,0,0);

// Mesh 
Scheduler scheduler;
painlessMesh mesh;
MeshNetworkLoger::LogClass logger;
MessageQueue messageQueue(10, true);
StaticJsonDocument <512> jsonDoc;

// Prototypes Callback pinlessMesh
void receivedCallback( uint32_t from, String &msg );
void newConnectionCallback( uint32_t nodeId );
void changedConnectionCallback( void );
void droppedConnectionCallback( uint32_t nodeId );

void sendMessagesToMQTTServer( void );
bool deserializationMessage( String msg );
bool isConnectionToWiFi = false;
void checkConnectionToWiFi( void );
Task tCheckConnectionToWiFi( TASK_MILLISECOND * 500, TASK_FOREVER, &checkConnectionToWiFi, &scheduler);
void checkConnectionToMQTT( void );
Task tCheckConnectionToMQTT( TASK_SECOND, TASK_FOREVER, &checkConnectionToMQTT, &scheduler);


void setup() {
  Serial.begin(115200);
  mesh.setDebugMsgTypes( ERROR | STARTUP );  
  logger.setLogLevel( MeshNetworkLoger::DEBUG );

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &scheduler, MESH_PORT );
  mesh.onReceive( &receivedCallback );
  mesh.onNewConnection( &newConnectionCallback );
  mesh.onChangedConnections( &changedConnectionCallback );
  mesh.onDroppedConnection( &droppedConnectionCallback );

  mesh.stationManual( NETWORK_SSID, NETWORK_PASSWORD );
  mesh.setHostname( HOSTNAME );

  mesh.setRoot();
  mesh.setContainsRoot();

  tCheckConnectionToWiFi.enableDelayed( TASK_SECOND );
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
  mqttClient.loop();

}


void receivedCallback( uint32_t from, String &msg ) {
  logger( MeshNetworkLoger::MESH_RECEIVE, "Received from %u message.",from );
  messageQueue.push( msg );
  sendMessagesToMQTTServer();
}

void newConnectionCallback(uint32_t nodeId) {
  logger( MeshNetworkLoger::MESH_NEW_CON, "New Connection, nodeId = %u",nodeId );
}

void changedConnectionCallback() {
  logger( MeshNetworkLoger::MESH_CHANGED_CON, "Changed connections" );
}

void droppedConnectionCallback(uint32_t nodeId) {
  logger(MeshNetworkLoger::MESH_DROPPED_CON, "Droped connection to nodeID %u", nodeId);
}


void checkConnectionToWiFi(){
  if( myIP != mesh.getStationIP() ){
    myIP = mesh.getStationIP();
    if( myIP == IPAddress(0,0,0,0) ){
      // No connection to WiFi
      isConnectionToWiFi = false;
      logger( MeshNetworkLoger::DEBUG, "No connection to WiFi" );
    }else{
      // Connection to WiFi 
      isConnectionToWiFi = true;
      logger(MeshNetworkLoger::DEBUG, "Connection to WiFi as IP %s",myIP.toString().c_str() );
      tCheckConnectionToMQTT.enable();
    }
  } 
} 

void checkConnectionToMQTT(){
  logger( MeshNetworkLoger::DEBUG, "Try connect to MQTT server ..." );

  if( isConnectionToWiFi == false ){
    tCheckConnectionToMQTT.disable();
    logger( MeshNetworkLoger::DEBUG, "No connection to WiFi" );
    return;
  }
    
  if( mqttClient.connect(MQTT_CLIENT_ID) ){
    logger( MeshNetworkLoger::DEBUG, "Connect to MQTT Server" );
    tCheckConnectionToMQTT.disable();
    sendMessagesToMQTTServer();
  }
}

bool deserializeMessage( String msg ){
  jsonDoc.clear();
  DeserializationError error = deserializeJson( jsonDoc, msg );

  if( error ){
    logger(MeshNetworkLoger::DEBUG, "deserializeJson() failed: %s",error.f_str());
    return false;
  }
  logger(MeshNetworkLoger::DEBUG, "deserializeJson() correct");
  return true;
}

void sendMessagesToMQTTServer(){
  if( messageQueue.isEmpty())
    return; // No message to send

  while( !messageQueue.isEmpty() ){
    if( !deserializeMessage( messageQueue.front() ) ){
      messageQueue.pop();
      return;
    }
    
    for( JsonObject nodeMeasurements : jsonDoc[FIELD_ARRAY].as<JsonArray>() ){
      const char* cTopic = nodeMeasurements[FIELD_TOPIC]; 
      float fValue = nodeMeasurements[FIELD_VALUE];
      String strValue = String( fValue );
      const char* cValue = strValue.c_str();
      logger(MeshNetworkLoger::DEBUG, "Try send to %s value %s",cTopic, cValue);

      if( !mqttClient.publish(cTopic,cValue)){
        tCheckConnectionToMQTT.enable();
        logger(MeshNetworkLoger::DEBUG, "MQTT Error send");
        return;
      }
      logger(MeshNetworkLoger::DEBUG, "Send to %s value %s",cTopic, cValue);
    }

    messageQueue.pop();
  }
}