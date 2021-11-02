# ESP32-AI-CAM
ESP32-CAM with Microsoft Azure AI Computer Vision and Storage Blobs

This propgram uses an ESP32-CAM module, with several sensors attached with I2C, to take pictures with the original ov2640 2MP camera, or an ov5640 5MP camera, and then send the photo first to Microsoft Cloud Azure for computer vision analysis, returning its analysis of the picture, and then sends the picture to Azure Blob Storage, along with sensors readings date, time, temperature, pressure, humidity, and light level, which can be further analysed by Azure cloud level functions and databases.  A backup copy of all the pictures and data is stored on the SD card of the ESP32-CAM.  The current sensors are a BME280 for temperature, pressure, and humidity, and a TSL2561 for light.  An ADXL345 is used to wake up the ESP32-CAM whenever there is a tap or motion of the camera.  

See https://github.com/jameszah/ESP32-AI-CAM/tree/main/samples for some sample photos.  The first bunch are ov2640, and the last is ov5640.  The filename of the picture contains the meta-data in a json like format for the temperature, humidity, etc recorded at the time of the picture.  It is designed for more complicated functions than taking pictures of your phone!  The Azure computer vision got Mozart wrong, apparently Marcia Davenport had something to do with that painting.

More details coming on hardware and software installation and design.

## Hardware - simple

## Hardware - better

## Hardware - nice

## Software Installation

## Compile Time Paramaters

## Azure Subscription and setting up Computer Vision, IOTHub, Blob Storage

...

https://github.com/Azure/azure-sdk-for-c/issues/1796

https://github.com/Azure/azure-iot-arduino/issues/136
