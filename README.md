# **MicroTrackAPI:** Real-Time Sensor Data Collection and Time Tracking - Through a Wireless Network

An esp32 microcontroller based system for collecting data from sensors which interact with the world.  
This data is sent through the network to a Flask API for interacting with a database.  
The API/database stores the current sensor reading, alongside a deviceID, and the time at which the reading was taken.  
Database and API tested with, and functions on, a Raspberry Pi 4.  
Current naming conventions, and personal use, is based on a simple photoresistor, but will work with any 5v resistance based analog sensor.  

<picture>
 <img alt="ESP32 in 3d printed case, next to a raspberry pi" src="database.png" width="40%">
</picture>



## Installation and Implementation

A MariaDB should be set up on a device, of your choosing, on your network.  
The same device, or another on the same network, should have the latest version of python installed. (3.9.2 working)  
This device will also need flask version 1.x. (higher version releases untested as of yet) https://flask.palletsprojects.com/en/stable/  
The device set up with python should receive the api.py file from this repository, and execute the file.  
(open the devices terminal, navigate to the directory containing api.py, and execute: "python api.py")  

The esp_32_photoresistor_server.ino file should be compiled and flashed to an esp32.  
(The arduino IDE contains some useful tools for this process, hence the filetype, but this can be done using your tool of choice with minimal change required)  
Connect your sensor to pin 34, and make any other neccesary connections.  
For my implementation of a light level sensor, I connected pin 34 to the 3.3v line through a photoresistor.  
I found it useful to also connect a 10k resistor between pin 34 and ground, as interference made readings inconsistent.  

You will need to input your network and database credentials in each file before execution.