# DAS – Distributed Alarm System 
This project implements a number of sensor modules that together form a home alarm system.
The base idea is to completely avoid using a local alarm controller. All alarm state information is stored in a cloud server and the alarm logic operates towards the server, rather than towards the individual sensors and activators.
The alarm modules are based on the popular ESP8266 controllers that have a built in WiFi interface.
The modules connect over WiFi and talk to a cloud based MQTT server.
The alarm logic module is built using Python.

The purpose of the project has not been to implement a fully ifunctional alarm system, but rather to investigate if the tested technologies could be used in a real system.
