@ECHO OFF

cd "C:\Program Files\mosquitto"
set message="Test 1 2 3"

:a
::mosquitto_pub.exe -h test.mosquitto.org -t rivaldm/test -m %message%
mosquitto_pub.exe -d -t rivaldm/test -m %message%
echo Publish to MQTT: %message%
timeout /t 3 /nobreak
goto a

PAUSE