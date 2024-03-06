#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <Arduino.h>

namespace Logger{
    
    typedef enum{
        ERROR_SENSOR    = 1 << 0,
        DEBUG_SENSOR    = 1 << 1,
        ERROR_MESH      = 1 << 2,
        DEBUG_MESH      = 1 << 3,
        ERROR_FUNCTION  = 1 << 4,
        DEBUG_FUNCTION  = 1 << 5,
        DEBUG           = 1 << 6
    }LoggerLevel;


    class Logger{
        uint16_t loggerLevel_ = 0;
        char msg_[200];

    public:
        Logger();
        Logger( uint16_t loggerLevel ) { setLoggerLevel( loggerLevel ); };

        void setLoggerLevel( uint16_t loggerLevel ){
            /**
             * @brief Set type of message you want to generate.
             * To choose type use Logger::Logger Level enumerate.
             * Function print to serial type of generated messages.
             */
            loggerLevel_ = loggerLevel;
            
            Serial.printf("\nLogger Active Type: ");

            if( loggerLevel_ & DEBUG ){
                //If debug active print all types message
                Serial.printf("DEBUG: ");
                loggerLevel_ = DEBUG * 2 - 1;
            }

            if( loggerLevel_ & ERROR_SENSOR ){
                Serial.printf(" ERROR_SENSOR |");
            }

            if( loggerLevel_ & DEBUG_SENSOR ){
                Serial.printf(" ERROR_SENSOR |");
            }

            if( loggerLevel_ & ERROR_MESH ){
                Serial.printf(" ERROR_SENSOR |");
            }

            if( loggerLevel_ & DEBUG_MESH ){
                Serial.printf(" ERROR_SENSOR |");
            }

            if( loggerLevel_ & ERROR_FUNCTION ){
                Serial.printf(" ERROR_SENSOR |");
            }

            if( loggerLevel_ & DEBUG_FUNCTION ){
                Serial.printf(" ERROR_SENSOR |");
            }

            Serial.println();
        }


        void operator()(LoggerLevel messageType, const char* format...){
            if( !(messageType & loggerLevel_) )
                return; // Return if type of messages are not set to print
  
            va_list args;
            va_start(args, format);
            vsnprintf(msg_, 200, format, args);

            switch ( messageType )
            {
            case ERROR_SENSOR:
                Serial.printf("ERROR_SENSOR: ");
                break;
            case DEBUG_SENSOR:
                Serial.printf("DEBUG_SENSOR: ");
                break;
            case ERROR_MESH:
                Serial.printf("ERROR_MESH: ");
                break;
            case DEBUG_MESH:
                Serial.printf("DEBUG_MESH: ");
                break;
            case ERROR_FUNCTION:
                Serial.printf("ERROR_FUNCTION: ");
                break;
            case DEBUG_FUNCTION:
                Serial.printf("DEBUG_FUNCTION: ");
                break;
            case DEBUG:
                Serial.printf("DEBUG: ");
                break;
            }

            Serial.println(msg_);
            va_end(args);
        }
    }; // class Logger
} // namespace Logger





#endif