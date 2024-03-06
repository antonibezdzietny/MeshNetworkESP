#include <Arduino.h>
#include <painlessMesh.h>
#include <cmath>

#include "logger.hpp"
#include "sensors.hpp"
#include "config.hpp"

Scheduler scheduler;
painlessMesh mesh;
StaticJsonDocument<512> docJSON;
Logger::Logger logger_main( Logger::LoggerLevel::DEBUG );

void receivedCallback( uint32_t from, String &msg );
void newConnectionCallback( uint32_t nodeId );
void changedConnectionCallback( void );
void droppedConnectionCallback( uint32_t nodeId );

void measurementReadyCallback(bool isMeasurement);
void createJSONDoc( void );
uint32_t getRootNodeID( painlessmesh::protocol::NodeTree nodeTree );
void sendMessageToRoot( void );
Task tSendMessageToRoot( TASK_MINUTE, TASK_FOREVER, &sendMessageToRoot, &scheduler );

uint32_t rootID = 0;


void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &scheduler, MESH_PORT, WIFI_STA, MESH_CHANEL );
  mesh.onReceive( &receivedCallback );
  mesh.onNewConnection( &newConnectionCallback );
  mesh.onChangedConnections( &changedConnectionCallback );
  mesh.onDroppedConnection( &droppedConnectionCallback );

  Sensors::setLoggerLevel( Logger::DEBUG );
  Sensors::setMeasurementsReadyCallback( &measurementReadyCallback );
  Sensors::setPinout();
  Sensors::setScheduler(&scheduler);
  Sensors::setMeasurementLoop( TASK_MINUTE * 10 );
}

void loop() {
  mesh.update();
}



void receivedCallback( uint32_t from, String &msg ){
  logger_main( Logger::LoggerLevel::DEBUG_MESH, "MESH: Recive from ID: %u",from );
};

void newConnectionCallback( uint32_t nodeId ){
  logger_main( Logger::LoggerLevel::DEBUG_MESH, "MESH: New connection ID: %u",nodeId );
  rootID = getRootNodeID( mesh.asNodeTree() );
};

void changedConnectionCallback( void ){
  logger_main( Logger::LoggerLevel::DEBUG_MESH, "MESH: Change connection");
  rootID = getRootNodeID( mesh.asNodeTree() );
};

void droppedConnectionCallback( uint32_t nodeId ){
  logger_main( Logger::LoggerLevel::DEBUG_MESH, "MESH: Deopped connection ID: %u",nodeId );
  rootID = 0;
};


void measurementReadyCallback(bool isMeasurement){
  if( !isMeasurement ){
    Sensors::setDefaultStatusForSensors();
    return;
  }

  createJSONDoc();
  Sensors::setDefaultStatusForSensors();
}

void createJSONDoc( void ){
  docJSON.clear();
  docJSON[ FIELD_NODE_NAME ] = MQTT_USER_NAME;

  JsonArray measurements = docJSON.createNestedArray( FIELD_ARRAY );

  JsonObject measurement;
  for( int i=0; i<Sensors::SensorType::ALL; ++i ){
    Sensors::SensorStatus sensorStatus = Sensors::getSensorStatus( (Sensors::SensorType) i);
    if( sensorStatus.isError )
      break;

    measurement = measurements.createNestedObject();

    switch (i){
      case Sensors::SensorType::DHT_TEMP:
        measurement[ FIELD_TOPIC ] = MQTT_TOPIC_TEMP;
        break;
      case Sensors::SensorType::DHT_HUM:
        measurement[ FIELD_TOPIC ] = MQTT_TOPIC_HUM;
        break;
      case Sensors::SensorType::BMP_PRE:
        measurement[ FIELD_TOPIC ] = MQTT_TOPIC_PRESSURE;
        break;
      case Sensors::SensorType::BH_LIG:
        measurement[ FIELD_TOPIC ] = MQTT_TOPIC_LIGHT;
        break;
    }
    measurement[ FIELD_VALUE ] = std::round(sensorStatus.measurement * 100.0)/100.0;
  }
  //Send message to root
  serializeJsonPretty(docJSON,Serial);
  tSendMessageToRoot.enable();
}

uint32_t getRootNodeID( painlessmesh::protocol::NodeTree nodeTree ){
  if( nodeTree.root )
    return nodeTree.nodeId;

  for( auto&& sub : nodeTree.subs ){
    auto findedID = getRootNodeID( sub );
    if( findedID != 0)
      return findedID;
  }
  return 0;
}

void sendMessageToRoot( void ){
  if( rootID == 0 ){
    logger_main( Logger::LoggerLevel::ERROR_FUNCTION, "No rootID");
    return;
  }

  String msg;
  serializeJson( docJSON, msg );
  mesh.sendSingle( rootID, msg );
  tSendMessageToRoot.disable();
  logger_main( Logger::LoggerLevel::DEBUG_FUNCTION, "Send message to Root");
}