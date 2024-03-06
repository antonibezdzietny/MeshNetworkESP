#include "sensors.hpp"

using namespace Sensors;


Logger::Logger logger( Logger::ERROR_FUNCTION | Logger::ERROR_SENSOR );
SensorStatus sensorsStatus [ SensorType::ALL ];
measurementsCallback callbackReadyMeasurements;
Scheduler *scheduler_;
TwoWire twoWire;

DHTesp dht;
Adafruit_BMP280 bmp(&twoWire);
BH1750 bh;



Task tMeasurementLoop( 0, TASK_FOREVER, &measurementLoop );
Task tPowerOnSensors( 0, TASK_ONCE, &powerOnSensors );
Task tConnectionToSensors( 0, TASK_ONCE, &connectionToSensors );
Task tGetMeasurements( 0, TASK_ONCE, &getMeasurements );
Task tPowerOffSensors( 0, TASK_ONCE, &powerOffSensors );

void Sensors::setLoggerLevel( uint16_t logger_level ){
    //Set Logger level with Logger::
    logger.setLoggerLevel(logger_level);
}

void Sensors::setScheduler( Scheduler* scheduler__ ){
    //Should be set before use Loop
    scheduler_ = scheduler__;
}

void Sensors::setMeasurementsReadyCallback( measurementsCallback measurementReadyCall ){ 
    //Function set callback which will be call when series of measurement finish
    callbackReadyMeasurements = measurementReadyCall;
}

void Sensors::setPinout( void ){
    //Function set default pinout
    pinMode( PIN_POWER_I2C, OUTPUT );
    pinMode( PIN_POWER_DHT, OUTPUT );
    digitalWrite( PIN_POWER_I2C, LOW );
    digitalWrite( PIN_POWER_DHT, LOW );
    twoWire.begin( PIN_SDA, PIN_SCL );
}

void Sensors::setDefaultStatusForSensors( void ){
    //Set to default value all sensors status
    for(int i=0; i<SensorType::ALL; ++i){
        sensorsStatus[i].isConnection  = false;
        sensorsStatus[i].isError       = false;
        sensorsStatus[i].isMeasurement = false;
        sensorsStatus[i].measurement   = 0.0;
    }
}

void Sensors::setMeasurementLoop( unsigned long interval ){
    /**
     * @brief Function run infinite loop with measurements
     * @param interval Time between measurements
     */
    logger(Logger::DEBUG_FUNCTION,"setMeasurementLoop()");
    if( interval < MINIMAL_MEASUREMENTS_INTERVAL ){
        interval = MINIMAL_MEASUREMENTS_INTERVAL;
        logger(Logger::ERROR_FUNCTION, 
        "In Sensors::setMeasurementLoop( unsigned long interval) - interval is too short, so set 5s");
    }

    scheduler_->addTask( tPowerOnSensors );
    scheduler_->addTask( tConnectionToSensors );
    scheduler_->addTask( tGetMeasurements );
    scheduler_->addTask( tPowerOffSensors );

    scheduler_->addTask( tMeasurementLoop );
    tMeasurementLoop.setInterval( interval );
    tMeasurementLoop.enable();
}

SensorStatus Sensors::getSensorStatus( SensorType sensorType ){
    /**
     * @brief Function for get sensor status
     * @param sensorType Sensors::SensorType - sensor type which value will be returned
     * @return Sensors::SensorStatus - structure
     */
    return sensorsStatus[ sensorType ];
}


/**
 *  ----- BELLOW FUNCTION -------
 *  Should not be use by user
 */

void Sensors::measurementLoop( void ){
    //Config infinite task set interval 
    logger(Logger::DEBUG_FUNCTION,"measurementLoop()");
    tPowerOnSensors.setIterations( TASK_ONCE );
    tConnectionToSensors.setIterations( TASK_ONCE );
    tGetMeasurements.setIterations( TASK_ONCE );
    tPowerOffSensors.setIterations( TASK_ONCE );

    tPowerOnSensors.enable();
    tConnectionToSensors.enableDelayed( DELAY_CONNECTION );
    tGetMeasurements.enableDelayed( DELAY_CONNECTION + DELAY_MEASUREMENTS );
    tPowerOffSensors.enableDelayed( 2 * DELAY_CONNECTION + DELAY_MEASUREMENTS );
}

void Sensors::powerOnSensors( void ){
    //Power On all sensors
    logger(Logger::DEBUG_FUNCTION,"powerOnSensors()");
    digitalWrite( PIN_POWER_DHT, HIGH );
    digitalWrite( PIN_POWER_I2C, HIGH );
}

void Sensors::connectionToSensors( void ){
    //Get connection to sensors
    logger(Logger::DEBUG_FUNCTION,"connectionToSensors()");

    //Connection to DHT11
    dht.setup( PIN_DHT, DHTesp::DHT11 );
    sensorsStatus[ SensorType::DHT_TEMP ].isConnection = true;
    sensorsStatus[ SensorType::DHT_HUM ].isConnection = true;
    logger( Logger::DEBUG_SENSOR, "Connected to DHT" );

    //Connection to BMP
    if( !bmp.begin( BMP_I2C_ADDRESS ) ){
        logger( Logger::ERROR_SENSOR, "No connection to BMP via I2C" );
        sensorsStatus[ SensorType::BMP_PRE ].isError = true;
    }else{
        bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     
                        Adafruit_BMP280::SAMPLING_X2,     
                        Adafruit_BMP280::SAMPLING_X16,    
                        Adafruit_BMP280::FILTER_X16,      
                        Adafruit_BMP280::STANDBY_MS_500); 

        sensorsStatus[ SensorType::BMP_PRE ].isConnection = true;
        logger( Logger::DEBUG_SENSOR, "Connected to BMP via I2C" );
    }

    //Connection to BH
    if( !bh.begin(BH1750::ONE_TIME_HIGH_RES_MODE, BH_I2C_ADDRESS, &twoWire) ){
        logger( Logger::ERROR_SENSOR, "No connection to BH via I2C" );
        sensorsStatus[ SensorType::BH_LIG ].isError = true;
    }else{
        sensorsStatus[ SensorType::BH_LIG ].isConnection = true;
        logger( Logger::DEBUG_SENSOR, "Connected to BH via I2C" );
    }

}


void Sensors::getMeasurements( void ){
    //Get measurements from sensors
    logger(Logger::DEBUG_FUNCTION,"getMeasurements()");

    //Read DHT
    if( sensorsStatus[ SensorType::DHT_TEMP ].isConnection ){
        TempAndHumidity valueDHT = dht.getTempAndHumidity();
        sensorsStatus[ SensorType::DHT_HUM ].measurement  = valueDHT.humidity;
        sensorsStatus[ SensorType::DHT_TEMP ].measurement = valueDHT.temperature;
    }

    //Read BMP280
    if( sensorsStatus[ SensorType::BMP_PRE ].isConnection ){
        sensorsStatus[ SensorType::BMP_PRE ].measurement = bmp.readPressure()/100; //For value in hPa
    }

    //Read BH1750
    if( sensorsStatus[ SensorType::BH_LIG ].isConnection ){
        sensorsStatus[ SensorType::BH_LIG ].measurement = bh.readLightLevel(); //For value in hPa
    }


    checkMeasurements();
}


void Sensors::checkMeasurements( void ){
    //Function check range of measurements
    for( int i=0; i<SensorType::ALL; ++i ){
        if( !sensorsStatus[i].isConnection ){
            sensorsStatus[i].isError = true;
            break;
        }

        switch ( i )
        {
        case SensorType::DHT_TEMP:
            if( isnan(sensorsStatus[SensorType::DHT_TEMP].measurement) ){
                sensorsStatus[SensorType::DHT_TEMP].isError = true;
            }
            if( sensorsStatus[SensorType::DHT_TEMP].measurement > dht.getUpperBoundTemperature() ||
                sensorsStatus[SensorType::DHT_TEMP].measurement < dht.getLowerBoundTemperature() ){
                    sensorsStatus[SensorType::DHT_TEMP].isError = true;
                }
            break;
        
        case SensorType::DHT_HUM:
            if( isnan(sensorsStatus[SensorType::DHT_HUM].measurement) ){
                sensorsStatus[SensorType::DHT_HUM].isError = true;
            }
            if( sensorsStatus[SensorType::DHT_HUM].measurement > dht.getUpperBoundHumidity() ||
                sensorsStatus[SensorType::DHT_HUM].measurement < dht.getLowerBoundHumidity() ){
                    sensorsStatus[SensorType::DHT_HUM].isError = true;
                }
            break;
        }
    }
    
    callCallback();
}

void Sensors::callCallback( void ){
    //Check if even one measuemnt is correct
    logger(Logger::DEBUG_FUNCTION, "callCallback()");
    for( int i=0; i<SensorType::ALL; ++i ){
        if( !sensorsStatus[i].isError ){
            callbackReadyMeasurements(true);
            return;
        }
    }
    callbackReadyMeasurements(false);
}

void Sensors::powerOffSensors( void ){
    //Power Off all sensors
    logger(Logger::DEBUG_FUNCTION,"powerOffSensors()");
    digitalWrite( PIN_POWER_DHT, LOW );
    digitalWrite( PIN_POWER_I2C, LOW );
}


