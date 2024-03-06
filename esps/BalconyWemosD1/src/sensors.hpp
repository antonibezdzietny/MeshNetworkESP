#ifndef SENSORS_HPP__
#define SENSORS_HPP__

#include <Arduino.h>
#include <painlessMesh.h>
#include <DHTesp.h>
#include <BH1750.h>
#include <Adafruit_BMP280.h>


#include "logger.hpp"

namespace Sensors{
    const uint8_t PIN_SDA = SDA;
    const uint8_t PIN_SCL = SCL;
    const uint8_t PIN_DHT = D3;
    const int BMP_I2C_ADDRESS = 0x76;
    const int BH_I2C_ADDRESS  = 0x23;
    const uint8_t PIN_POWER_I2C = D5;
    const uint8_t PIN_POWER_DHT = D6;
    const unsigned long DELAY_CONNECTION = 500u;
    const unsigned long DELAY_MEASUREMENTS = TASK_SECOND * 2;
    const unsigned long DAFAULT_MEASUREMENTS_INTERVAL = TASK_MINUTE * 5;
    const unsigned long MINIMAL_MEASUREMENTS_INTERVAL = TASK_SECOND * 5;

    enum SensorType{
        DHT_TEMP,
        DHT_HUM,
        BMP_PRE,
        BH_LIG,
        ALL
    };


    struct SensorStatus{
        bool isConnection;
        bool isError;
        bool isMeasurement;
        float measurement;
    };


    typedef std::function<void(bool isMeasurements)> measurementsCallback;
    void setLoggerLevel( uint16_t logger_level );
    void setPinout( void );
    void setScheduler( Scheduler* scheduler__ );
    void setMeasurementsReadyCallback( measurementsCallback measurementReadyCall );
    void setDefaultStatusForSensors( void );
    void setMeasurementLoop( unsigned long interval = DAFAULT_MEASUREMENTS_INTERVAL );
    SensorStatus getSensorStatus( SensorType sensorType );


    void measurementLoop( void );
    void powerOnSensors( void );
    void connectionToSensors( void );
    void getMeasurements( void );
    void checkMeasurements( void );
    void callCallback( void );
    void powerOffSensors( void );
} // namespace Sensors

#endif