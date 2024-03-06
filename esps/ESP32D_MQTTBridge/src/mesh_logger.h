#ifndef __LOGGER_MESH_NETWORK
#define __LOGGER_MESH_NETWORK

namespace MeshNetworkLoger{
    #include <Arduino.h>

    typedef enum{
        MESH_SEND = 1 << 0,
        MESH_RECEIVE = 1 << 1 ,
        MESH_NEW_CON = 1 << 2,
        MESH_DROPPED_CON = 1 << 3,
        MESH_CHANGED_CON = 1 << 4,
        MESH_DEBUG = 1 << 5,
        SENSOR_READ = 1 << 6,
        SENSOR_ERROR = 1 << 7,
        STARTUP = 1 << 8,
        DEBUG = 1 << 9
    } LogLevel;


class LogClass {
 public:
  void setLogLevel(uint16_t newTypes) {
    // set the different kinds of debug messages you want to generate.
    types = newTypes;
    Serial.print(F("\nsetLogLevel:"));
    if (types & DEBUG) {
      //if DEBUG print all message
      Serial.print(F(" DEBUG : "));
      types = DEBUG * 2 -1;
    }

    if (types & MESH_SEND) {
      Serial.print(F(" MESH_SEND |"));
    }
    if (types & MESH_RECEIVE) {
      Serial.print(F(" ERROR |"));
    }
    if (types & MESH_NEW_CON) {
      Serial.print(F(" STARTUP |"));
    }
    if (types & MESH_DROPPED_CON) {
      Serial.print(F(" MESH_DROPPED_CONNECTION |"));
    }
    if (types & MESH_DROPPED_CON) {
      Serial.print(F(" MESH_CHANGED_CON |"));
    }
    if (types & MESH_DEBUG) {
      Serial.print(F(" MESH_DEBUG |"));
    }
    if (types & SENSOR_READ) {
      Serial.print(F(" CONNECTION |"));
    }
    if (types & SENSOR_ERROR) {
      Serial.print(F(" CONNECTION |"));
    }
    if (types & STARTUP) {
      Serial.print(F(" STARTUP |"));
    }
    Serial.println();
    return;
  }

  void operator()(LogLevel type, const char* format...) {
    if (type & types) {  // Print only the message types set for output
      va_list args;
      va_start(args, format);

      vsnprintf(str, 200, format, args);

      if (types) {
        switch (type) {
          case MESH_SEND:
            Serial.print(F("MESH_SEND: "));
            break;
          case MESH_RECEIVE:
            Serial.print(F("MESH_RECEIVE: "));
            break;
          case MESH_NEW_CON:
            Serial.print(F("MESH_NEW_CONNECTION: "));
            break;
          case MESH_DROPPED_CON:
            Serial.print(F("MESH_DROPPED_CONNECTION: "));
            break;
          case MESH_CHANGED_CON:
            Serial.print(F("MESH_CHANGED_CONNECTION: "));
            break;
          case MESH_DEBUG:
            Serial.print(F("MESH_DEBUG: "));
            break;
          case SENSOR_READ:
              Serial.print(F("SENSOR_READ: "));
              break;
          case SENSOR_ERROR:
              Serial.print(F("SENSOR_ERROR: "));
              break;
          case STARTUP:
              Serial.print(F("STARTUP: "));
              break;
          case DEBUG:
              Serial.print(F("DEBUG: "));
              break;
        }
      }
      Serial.println(str);

      va_end(args);
    }
  }

 private:
  uint16_t types = 0;
  char str[200];
};

}



#endif