@ECHO OFF

cd "C:\Program Files\mosquitto"
:x
mosquitto_sub.exe -h 192.168.1.23 -t rivaldm/test
::mosquitto_sub.exe -d -t rivaldm/test
goto x

PAUSE