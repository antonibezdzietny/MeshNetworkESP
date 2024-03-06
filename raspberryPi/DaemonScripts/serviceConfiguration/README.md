# RaspberryPi - systemd - autorun services
MQTTLoger & RESTApi autorun with Pi user.

## Instalation
Put files to:
> /lib/systemd/system

Add services to autorun:
```bash
sudo systemctl enable mqttpythonloger.service
sudo systemctl enable restapi.service
```

## Configuration
Configure in `.service` files, line `ExecStart`.

For example:
```
ExecStart=/usr/bin/python3 /home/pi/ScriptsMeshNetwork/RESTApi.py
```
WHERE  
/usr/bin/python3 - path to python interpreter,  
/home/pi/ScriptsMeshNetwork/RESTApi.py - path to script.
