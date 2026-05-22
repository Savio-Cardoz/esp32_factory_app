# ESP32 Factory App

Platform: ESP32S3
ESP-IDF version: 5.4.x

This project is meant to be a factory application thats sits alongside a single updateable application partition.
The factory app can read binaries from a SDcard and flash to the application partition.
The intention being to be able to use more of the available flash space of the ESP32S3 for application purposes.

TODO:
1. Check binary hash before flashing
2. Implement rollback 
