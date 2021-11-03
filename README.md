# ESP32-AI-CAM
ESP32-CAM with Microsoft Azure AI Computer Vision and Storage Blobs

This propgram uses an ESP32-CAM module, with several sensors attached with I2C, to take pictures with the original ov2640 2MP camera, or an ov5640 5MP camera, and then send the photo first to Microsoft Cloud Azure for computer vision analysis, returning its analysis of the picture, and then sends the picture to Azure Blob Storage, along with sensors readings date, time, temperature, pressure, humidity, and light level, which can be further analysed by Azure cloud level functions and databases.  A backup copy of all the pictures and data is stored on the SD card of the ESP32-CAM.  The current sensors are a BME280 for temperature, pressure, and humidity, and a TSL2561 for light.  An ADXL345 is used to wake up the ESP32-CAM whenever there is a tap or motion of the camera.  

See https://github.com/jameszah/ESP32-AI-CAM/tree/main/samples for some sample photos.  The first bunch are ov2640, and the last is ov5640.  The filename of the picture contains the meta-data in a json like format for the temperature, humidity, etc recorded at the time of the picture.  It is designed for more complicated functions than taking pictures of your phone!  The Azure computer vision got Mozart wrong, apparently Marcia Davenport had something to do with that painting.

More details coming on hardware and software installation and design.

## Hardware - simple

## Hardware - better

## Hardware - nice

## Software Installation

As of Nov 3, 2021:

Start with a normal Arduino installation (https://www.arduino.cc/en/software) - I am using Windows 10 with version 1.8.13 -- a few months old I think, as the current 1.8.16.

Install the esp32 board library (Tools ... Board ... Board Manager) -- I am using 1.0.6 -- a few months old I think, as the current is 2.0.0.

Next install the Arduino Librares (Tools ... Manage Libraries).

You need the following Azure libraries from Microsoft (all current):
   - AzureIoTHub by Microsoft 1.60
   - AzureIoTProtocol_HTTP by Microsoft 1.60
   - AzureIoTProtocol_MQTT by Microsoft 1.60
   - AzureIoTSocket_WiFi by Microsoft 1.02
   - AzureIoTUtility by Microsoft 1.61

And the following libraries from Sprakfun for the I2C sensors (all current):
   - SparkFun ADXL345 Arduino Library by SparkFun 1.0.0
   - SparkFun BME280 by SparkFun 2.0.9
   - SparkFun TSL2561 by Mike Grusin@SparkFun 1.1.0

And the JSON Library to unpack the response from Azure Computer Vision:
   - ArduinoJSON by Benoit Blanchon 6.16.1

That is all easy, but the complex part comes next.  This system uses the Azure IOTHub system which is designed to connect small sensors Azure to report temperatures and pressues which are small chunks of data.  But in the past it seems to have supported larger chunks of data, such as Blobs - Binary Large Objects.  Support for this seems to have disappeared, so to add it back, do the following.  You have to grab these two files:

https://github.com/Azure/azure-iot-arduino/blob/jbobotek-patch-1/src/blob.c   
https://github.com/Azure/azure-iot-arduino/blob/jbobotek-patch-1/src/blob.h   

... and store them in these folders.  This is for a "portable" version of the Arduino compiler, which lets you have multiple versions of the compiler on your computer.  (I'll figure out the pathnames for the regular install later.)

C:\ArduinoPortable\arduino-1.8.13\portable\sketchbook\libraries\AzureIoTHub\src\internal\blob.h   
C:\ArduinoPortable\arduino-1.8.13\portable\sketchbook\libraries\AzureIoTHub\src\blob.c    

And then re-start your Arduino system is always good advice.

Read more about the disappearance of the Arduino IOTHub blob functionality here:

https://github.com/Azure/azure-iot-arduino/issues/136

Another alternative is to use another Azure library called azure-sdk-for-c https://github.com/Azure/azure-sdk-for-c, where they are just in the process of re-adding blob support into that system https://github.com/Azure/azure-sdk-for-c/issues/1796.  It remains to be seen if that will fit into the memory available on the esp32, and whether this libcurl issue still has to be addressed.  https://github.com/Azure/azure-iot-arduino/issues/135#issue-1024008479


## Compile Time Paramaters

## Azure Subscription and setting up Computer Vision, IOTHub, Blob Storage

...

https://github.com/Azure/azure-sdk-for-c/issues/1796

https://github.com/Azure/azure-iot-arduino/issues/136
