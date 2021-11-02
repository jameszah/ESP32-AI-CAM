# ESP32-AI-CAM
ESP32-CAM with Microsoft Azure AI Computer Vision and Storage Blobs

This propgram uses an ESP32-CAM module, with several sensors attached with I2C, to take pictures with the original ov2640 2MP camera, or an ov5640 5MP camera, and then send the photo first to Microsoft Cloud Azure for computer vision analysis, returning its analysis of the picture, and then sends the picture to Azure Blob Storage, along with sensors readings date, time, temperature, pressure, humidity, and light level, which can be further analysed by Azure cloud level functions and databases.

See https://github.com/jameszah/ESP32-AI-CAM/tree/main/samples for some sample photos
