#include <Arduino.h>
#include <painlessMesh.h>
#include <DHTesp.h>

#include "config.hpp"
#include "logger.hpp"

Scheduler scheduler;
painlessMesh mesh;
StaticJsonDocument <512> docJSON;
uint32_t rootID = 0;
//Logger::Logger logger;

DHTesp dht;
TempAndHumidity dhtMeasurement;
const uint8_t DHT_PIN = 2;
const uint8_t DHT_POWER_PIN = 1;

const unsigned long DHT_MIN_DELAY = 3000;
const unsigned int DHT_ATTEMPTS   = 5;
bool isCorrectMeasurement = false;

const unsigned long INTERVAL_CONDITION_MESSAGE = 5 * TASK_SECOND;
const int ATTEMPTS_CONDITION_MESSAGE = 10;
const unsigned long DELAY_SHUT_DOWN = 1000;


//Callback painlessMesh
void receivedCallback( uint32_t from, String &msg );
void newConnectionCallback( uint32_t nodeId );
void changedConnectionCallback( void );
void droppedConnectionCallback( uint32_t nodeId );

//Declaration functions and tasks
void setPinout( void );
void sendMessageToRoot( void );
String createJSONMessage( void );
uint32_t getRootNodeID( painlessmesh::protocol::NodeTree nodeTree );
void readDHTAndCheckCorrect( void );
Task tReadDHTAndCheckCorrect( DHT_MIN_DELAY, DHT_ATTEMPTS, &readDHTAndCheckCorrect, &scheduler);
void sendMessageUnderCondition( void );
Task tSendMessageUnderCondition( INTERVAL_CONDITION_MESSAGE, 
    ATTEMPTS_CONDITION_MESSAGE, &sendMessageUnderCondition, &scheduler );
void disconnectAndSleep( void );
Task tDisconnectAndSleep( 0, TASK_ONCE, &disconnectAndSleep, &scheduler );

void setup() {
  Serial.begin(115200);
  //logger( Logger::LoggerLevel::DEBUG, "Module wake up");

  mesh.setDebugMsgTypes( ERROR | STARTUP );
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &scheduler, MESH_PORT, WIFI_STA, MESH_CHANEL );
  mesh.onReceive( &receivedCallback );
  mesh.onNewConnection( &newConnectionCallback );
  mesh.onChangedConnections( &changedConnectionCallback );
  mesh.onDroppedConnection( &droppedConnectionCallback ); 

  setPinout();
  tReadDHTAndCheckCorrect.enableDelayed( DHT_MIN_DELAY );
  tSendMessageUnderCondition.enableDelayed( INTERVAL_CONDITION_MESSAGE );
}

void loop() {
  mesh.update();
}


void setPinout( void ){
  //Function set pinout
  //logger( Logger::LoggerLevel::DEBUG_FUNCTION, "setPinout() - set all pinout");
  pinMode( DHT_POWER_PIN, OUTPUT );
  digitalWrite( DHT_POWER_PIN, HIGH );

  dht.setup( DHT_PIN, DHTesp::DHT11 );
}

void readDHTAndCheckCorrect( void ){
  //Function read DHT after delay and check correct value
  //logger( Logger::LoggerLevel::DEBUG_FUNCTION, "readDHTAndCheckCorrect() - function read DHT ");
  dhtMeasurement = dht.getTempAndHumidity();

  // If cannot read DHT11
  if( isnan(dhtMeasurement.temperature) || isnan(dhtMeasurement.humidity) ){
    //logger( Logger::LoggerLevel::ERROR_SENSOR, "DHT11 - NAN value read");
    return;
  }

  // Return if value of temperature are not in range
  if( dhtMeasurement.temperature < (float) dht.getLowerBoundTemperature() ||
      dhtMeasurement.temperature > (float) dht.getUpperBoundTemperature() ){
    //logger( Logger::LoggerLevel::ERROR_SENSOR, "DHT11 - incorrect temperature value" );
    return;
  } 

  // Return if value of humidity are not in range
  if( dhtMeasurement.humidity < (float) dht.getLowerBoundHumidity() ||
      dhtMeasurement.humidity > (float) dht.getUpperBoundHumidity() ){
    //logger( Logger::LoggerLevel::ERROR_SENSOR, "DHT11 - incorrect humidity value" );
    return;
  }

  isCorrectMeasurement = true;
  tReadDHTAndCheckCorrect.disable();
  //logger( Logger::LoggerLevel::DEBUG, "DHT11 - correct measurements");
}

void sendMessageUnderCondition( void ){
  //logger( Logger::LoggerLevel::DEBUG_FUNCTION, "sendMessageUnderCondition() - check condition for send mesage");
  
  //If cannot connect to to DHT11
  if( !isCorrectMeasurement ){
    if( tReadDHTAndCheckCorrect.getIterations() == 0 ){
      //logger( Logger::LoggerLevel::ERROR_SENSOR, "Incorrect connection to DHT - check connection ");
      tDisconnectAndSleep.enableDelayed( DELAY_SHUT_DOWN );
    }
    return;
  }
  //If cannot found rootID
  if( rootID == 0 ){
    if( tSendMessageUnderCondition.isLastIteration() ){
      //logger( Logger::LoggerLevel::ERROR_MESH, "rootID - not found ...");
      tDisconnectAndSleep.enableDelayed( DELAY_SHUT_DOWN );
    }
    return;
  }

  //logger( Logger::LoggerLevel::DEBUG, "sendMessageUnderCondition() - correct condition ready to send!");
  tSendMessageUnderCondition.disable();
  sendMessageToRoot();
}

String createJSONMessage( void ){
  docJSON.clear();
  String messageString;
  docJSON[ FIELD_NODE_NAME ] = MQTT_USER_NAME;

  JsonArray measurements = docJSON.createNestedArray( FIELD_ARRAY );
  JsonObject measurement;
  measurement = measurements.createNestedObject();
  measurement[ FIELD_TOPIC ] = MQTT_TOPIC_TEMP;
  measurement[ FIELD_VALUE ] = dhtMeasurement.temperature;

  measurement = measurements.createNestedObject();
  measurement[ FIELD_TOPIC ] = MQTT_TOPIC_HUM;
  measurement[ FIELD_VALUE ] = dhtMeasurement.humidity;

  serializeJson( docJSON, messageString );
  return messageString;
}

void sendMessageToRoot( void ){
  //logger( Logger::LoggerLevel::DEBUG_FUNCTION, "sendMessageToRoot() - send data to Root");
  mesh.sendSingle( rootID, createJSONMessage() );
  tDisconnectAndSleep.enableDelayed( DELAY_SHUT_DOWN );
}

void disconnectAndSleep(){
  mesh.stop();
  digitalWrite( DHT_POWER_PIN, LOW );
  //logger( Logger::LoggerLevel::DEBUG_FUNCTION, "disconnectAndSleep() - Sleep for %u us",SLEEP_BETWEEN_MEASUREMENT );
  ESP.deepSleep( SLEEP_BETWEEN_MEASUREMENT );
}

void receivedCallback( uint32_t from, String &msg ) {
  //logger( Logger::LoggerLevel::DEBUG_MESH, "Receive data from nodeID: %u",from);
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

void newConnectionCallback(uint32_t nodeId) {
  rootID = getRootNodeID( mesh.asNodeTree() );
  //logger( Logger::LoggerLevel::DEBUG_MESH, "New connection to nodeID: %u", nodeId);
}

void changedConnectionCallback( void ){
  //logger( Logger::LoggerLevel::DEBUG_MESH, "MESH: Change connection");
  rootID = getRootNodeID( mesh.asNodeTree() );
};

void droppedConnectionCallback(uint32_t nodeId) {
  rootID = 0;
  //logger( Logger::LoggerLevel::DEBUG_MESH, "Droped connection to nodeID %u", nodeId);
}