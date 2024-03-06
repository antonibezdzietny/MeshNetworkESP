//Configuration MESH 
const char* MESH_PREFIX   = "TestingMeshNetwork";
const char* MESH_PASSWORD = "Qwerty123!@#";
const int   MESH_PORT     = 5555;
const int   MESH_CHANEL   = 9; //Must be the same as chanal AP

//Local config
const char* MQTT_USER_NAME = "ESP_WEMOS_BALCONY";
const char* FIELD_NODE_NAME  = "Node";
const char* FIELD_ARRAY = "Measurements";
const char* FIELD_TOPIC = "Topic";
const char* FIELD_VALUE = "Value";
const char* MQTT_TOPIC_TEMP = "balcony/temperature";
const char* MQTT_TOPIC_HUM = "balcony/humidity";
const char* MQTT_TOPIC_PRESSURE = "balcony/pressure";
const char* MQTT_TOPIC_LIGHT = "balcony/light";


//Timing
const unsigned long BETWEEN_MEASUREMENT= 60e6;