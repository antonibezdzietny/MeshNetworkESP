# ESP Mesh Network via WiFi

### Mesh Network

This project is an implementation mesh network over WiFi built on painlessMesh library. The hardware mesh network is built on ESP modules and RaspberryPi. Messages are sent via the lightweight MQTT protocol.

![Network idea schema](/img/networkSchemat.png)


### RaspberryPi - Server MQTT / DB / REST 

RaspberryPi has the following role:

- MQTT Broker / Server - *Eclipse Mosquitto* - server responsible for communication with the nodes in mesh network
- DataBase Server - *MariaDB* - server responsible for data storage
- REST API - *Flask* - REST server responsible for communication with other outside devices

![DB](/img/dataBase.png)

![Example of Publish](/img/examplePublish.png)

### Android application

The project also provides a simple mobile application to observe the data collected.

![Application](/img/application.png)


