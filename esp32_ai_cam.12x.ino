/*

  esp32-ai-cam

    by James Zahary    jamzah.plc@gmail.com

    https://github.com/jameszah/ESP32-AI-CAM

    jameszah/ESP32-AI-CAM is licensed under the
    GNU General Public License v3.0

  The is Arduino code, with standard setup for ESP32-CAM
    - Board ESP32 Wrover Module
    - Partition Scheme Huge APP (3MB No OTA)

  Libraries from the Arduino Manage Libraries ...
   - AzureIoTHub by Microsoft 1.60
   - AzureIoTProtocol_HTTP by Microsoft 1.60
   - AzureIoTProtocol_MQTT by Microsoft 1.60
   - AzureIoTSocket_WiFi by Microsoft 1.02
   - AzureIoTUtility by Microsoft 1.61

   - SparkFun ADXL345 Arduino Library by SparkFun 1.0.0
   - SparkFun BME280 by SparkFun 2.0.9
   - SparkFun TSL2561 by Mike Grusin@SparkFun 1.1.0

   - ArduinoJSON by Benoit Blanchon 6.16.1

  Oct 24, 2021
  - start with iothub_upload_to_blob_8_56.8 (azure upload-to-blob via IOTHub)
  - add tsl i2c test program
  - add azure telemetry code for temp, humidity
  - add sd writer program
  - add computer vision code
  - switch adafruit to sparkfun libraries for adxl interupts
  - delete telemtery, make blob filename a json with telemetry
  - modify json for sd card file name


  Nov 1, 2021
  - ver 0.12


*/
/*

  Azure -> (Iot Hub) jzhub -> Iot Devices -> (Iot Device) jzLog -> Primary Connection String
  HostName=jzhub.azure-devices.net;DeviceId=jzLog;SharedAccessKey=............................................

  Azure -> (Iot Hub) jzhub -> File Upload -> Azure storage accont -> (Storage account) jzblob
                                     Azure storage container -> (Container) jzcont

  Filename is within the jzblob/jzcont, then 20211102/{=camera=_=cam1=,=date=_=2021-11-02=,=time=_=05-01-19=,=temp=_=21.7=,=humid=_=36.2=,=press=_=089.1=,=lux=_=477.0=,=reason=_=timer=,=seq=_=001=}.jpg

*/
/*
Using library AzureIoTHub at version 1.6.0 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\sketchbook\libraries\AzureIoTHub 
Using library AzureIoTUtility at version 1.6.1 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\sketchbook\libraries\AzureIoTUtility 
Using library AzureIoTProtocol_HTTP at version 1.6.0 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\sketchbook\libraries\AzureIoTProtocol_HTTP 
Using library AzureIoTSocket_WiFi at version 1.0.2 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\sketchbook\libraries\AzureIoTSocket_WiFi 
Using library WiFi at version 1.0 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\packages\esp32\hardware\esp32\1.0.6\libraries\WiFi 
Using library WiFiClientSecure at version 1.0 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\packages\esp32\hardware\esp32\1.0.6\libraries\WiFiClientSecure 
Using library ArduinoJson at version 6.16.1 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\sketchbook\libraries\ArduinoJson 
Using library FS at version 1.0 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\packages\esp32\hardware\esp32\1.0.6\libraries\FS 
Using library SD_MMC at version 1.0 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\packages\esp32\hardware\esp32\1.0.6\libraries\SD_MMC 
Using library Wire at version 1.0.1 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\packages\esp32\hardware\esp32\1.0.6\libraries\Wire 
Using library SparkFun_BME280 at version 2.0.9 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\sketchbook\libraries\SparkFun_BME280 
Using library SPI at version 1.0 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\packages\esp32\hardware\esp32\1.0.6\libraries\SPI 
Using library SparkFun_TSL2561 at version 1.1.0 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\sketchbook\libraries\SparkFun_TSL2561 
Using library Accelerometer_ADXL345 at version 1.0.0 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\sketchbook\libraries\Accelerometer_ADXL345 
"C:\\ArduinoPortable\\arduino-1.8.13\\portable\\packages\\esp32\\tools\\xtensa-esp32-elf-gcc\\1.22.0-97-gc752ad5-5.2.0/bin/xtensa-esp32-elf-size" -A "C:\\Users\\James\\AppData\\Local\\Temp\\arduino_build_223463/esp32_ai_cam.12.ino.elf"
Sketch uses 1244046 bytes (39%) of program storage space. Maximum is 3145728 bytes.
Global variables use 55612 bytes (16%) of dynamic memory, leaving 272068 bytes for local variables. Maximum is 327680 bytes.

*/


#ifdef SET_TRUSTED_CERT_IN_SAMPLES
#include "certs.h"
#endif // SET_TRUSTED_CERT_IN_SAMPLES

#include "settings.h"

#include <AzureIoTHub.h>
#include <stdio.h>
#include <stdlib.h>
#include "Esp.h"
#include "AzureIoTProtocol_HTTP.h"
#include "internal/blob.h"
#include "iothub.h"
#include "iothub_device_client.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "iothub_message.h"
#include "iothubtransporthttp.h"
#include "azure_c_shared_utility\httpapi.h"
#include <Arduino.h>
#include <time.h>
#include "AzureIoTSocket_WiFi.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>  // for vision
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_camera.h"

float get_bme_temperature();
float get_bme_pressure();
float get_bme_humidity();
float get_tsl_lux();

RTC_DATA_ATTR int bootCount = 0;
esp_sleep_wakeup_cause_t wakeup_reason; 

#define maxblock 8192
static char data_to_upload[maxblock];
static int block_count = 0;
static int fb_ptr;
static camera_fb_t * fb;
static int fblen;
unsigned char* fbbuf;
static int picnum = 0;

#define max_pics 5
int num_pics = 3;
uint8_t* framebuffer[max_pics];
int framebuffer_len[max_pics];
time_t framebuffer_now[max_pics];
float temperature[max_pics];
float pressure[max_pics];
float humidity[max_pics];
float lux[max_pics];

/*Optional string with http proxy host and integer for http proxy port (Linux only)         */
static const char* proxyHost = NULL;
static int proxyPort = 0;

// Times before 2010 (1970 + 40 years) are invalid
#define MIN_EPOCH (40 * 365 * 24 * 3600)


static const char ssid[] = IOT_CONFIG_WIFI_SSID;
static const char pass[] = IOT_CONFIG_WIFI_PASSWORD;

#include "soc/rtc_cntl_reg.h"
static void initWifi(const char* ssid, const char* pass) {
  uint32_t brown_reg_temp = READ_PERI_REG(RTC_CNTL_BROWN_OUT_REG);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // Attempt to connect to Wifi network:
  delay(500);
  Serial.print("\r\n\r\nAttempting to connect to SSID: ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);
  //WiFi.disconnect();
  delay(500);
  int fail = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    fail++;
    if (fail == 99) {
      ESP.restart();
    } else if (fail % 10 == 9) {
      WiFi.disconnect();
      WiFi.begin(ssid, pass);
    }
    delay(1000);
  }

  Serial.println("\r\nConnected to wifi");

  wifi_ps_type_t the_type;

  //esp_err_t get_ps = esp_wifi_get_ps(&the_type);
  //Serial.printf("The power save was: %d\n", the_type);

  esp_err_t set_ps = esp_wifi_set_ps(WIFI_PS_NONE);
  //esp_err_t new_ps = esp_wifi_get_ps(&the_type);
  //Serial.printf("The power save is : %d\n", the_type);

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, brown_reg_temp);
}

static void initTime() {
  time_t epochTime;

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  delay(2000);

  while (true) {
    epochTime = time(NULL);
    if (epochTime < MIN_EPOCH) {
      Serial.println("Fetching NTP epoch time failed! Waiting 2 seconds to retry.");
      delay(2000);
    } else {
      //Serial.print("Fetched NTP epoch time is: ");
      //Serial.println(epochTime);
      break;
    }
  }
}

static IOTHUB_CLIENT_FILE_UPLOAD_GET_DATA_RESULT getDataCallback(IOTHUB_CLIENT_FILE_UPLOAD_RESULT result, unsigned char const ** data, size_t* size, void* context) {
  (void)context;
  //Serial.printf("block %d, -- ",block_count);

  if (result == FILE_UPLOAD_OK) {
    if (data != NULL && size != NULL) {

      if (fb_ptr <  fblen) {
        int blk_size;
        if (fb_ptr + maxblock <= fblen) {
          blk_size = maxblock;
        } else {
          blk_size = fblen - fb_ptr;
        }

        if (block_count < 2 || block_count % 10 == 0) {
          Serial.printf("block %4d, fb_ptr %7d, blk_size %5d -- ", block_count, fb_ptr, blk_size);
          ram();
        }
        memcpy(&data_to_upload[0], (fbbuf + fb_ptr), blk_size);

        *data = (const unsigned char*)data_to_upload;
        *size = blk_size;
        block_count++;
        fb_ptr += blk_size;

      } else {
        *data = NULL;
        *size = 0;
        Serial.printf("Indicating upload is complete (%d blocks uploaded)\r\n", block_count);
      }
    }
    else {
      Serial.printf("Last call to getDataCallback (result for %dth block uploaded: %s)\r\n", block_count, MU_ENUM_TO_STRING(IOTHUB_CLIENT_FILE_UPLOAD_RESULT, result));
    }
  } else {
    Serial.printf("Received unexpected result %s\r\n", MU_ENUM_TO_STRING(IOTHUB_CLIENT_FILE_UPLOAD_RESULT, result));
  }
  return IOTHUB_CLIENT_FILE_UPLOAD_GET_DATA_OK;
}

void sd_jpeg_write(uint8_t * fbbuf, int fblen, time_t now, float termperature, float humidity, float lux, float pressure, int i);

void save_n_photo() {
  for (int i = 0; i < num_pics; i++ ) {
    sd_jpeg_write(framebuffer[i], framebuffer_len[i], framebuffer_now[i] , temperature[i], humidity[i], lux[i], pressure[i], i);
  }
}

void send_n_photo() {

  IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle;

  // Used to initialize IoTHub SDK subsystem
  (void)IoTHub_Init();
  Serial.printf("Starting the IoTHub client sample upload to blob...\r\n");

  device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(connectionString, HTTP_Protocol);
  if (device_ll_handle == NULL) {
    Serial.printf("Failure creating IotHub device. Hint: Check your connection string.\r\n");
  } else {

    IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_TRUSTED_CERT, certificates);

    HTTP_PROXY_OPTIONS http_proxy_options = { 0 };
    http_proxy_options.host_address = proxyHost;
    http_proxy_options.port = proxyPort;

    if (proxyHost != NULL && IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_HTTP_PROXY, &http_proxy_options) != IOTHUB_CLIENT_OK) {
      Serial.printf("failure to set proxy\n");
    } else {

      for (int i = 0; i < num_pics; i++ ) {
        fbbuf = framebuffer[i];
        fblen = framebuffer_len[i];

        fb_ptr = 0;
        block_count = 0 ;

        Serial.printf("Pic length is % d\n", fblen);

        char strfbuf_date[22];
        char strfbuf_time[22];

        char azure_filename[180];
        struct tm timeinfo;
        localtime_r(&framebuffer_now[i], &timeinfo); 
        char strfbuf_ymd[22];

        strftime(strfbuf_ymd, sizeof(strfbuf_ymd), "%Y%m%d", &timeinfo);  // azure does not need the leading slash !!!!
        //Serial.printf("strfbuf_ymd >>%s<<, len %d\n", strfbuf_ymd,sizeof(strfbuf_ymd));
        strftime(strfbuf_date, sizeof(strfbuf_date), "%F", &timeinfo);
        //Serial.printf("strfbuf_date >>%s<<, len %d\n", strfbuf_date,sizeof(strfbuf_date));
        strftime(strfbuf_time, sizeof(strfbuf_time), "%H-%M-%S", &timeinfo);
        //Serial.printf("strfbuf_time >>%s<<, len %d\n", strfbuf_time,sizeof(strfbuf_time));
        
        char reason[6];

        switch (wakeup_reason)
        {
          case ESP_SLEEP_WAKEUP_EXT0 :     sprintf(reason, "%s", "axdl3");  break;
          case ESP_SLEEP_WAKEUP_EXT1 :     sprintf(reason, "%s", "other");  break;
          case ESP_SLEEP_WAKEUP_TIMER :    sprintf(reason, "%s", "timer");  break;
          case ESP_SLEEP_WAKEUP_TOUCHPAD : sprintf(reason, "%s", "other");  break;
          case ESP_SLEEP_WAKEUP_ULP :      sprintf(reason, "%s", "other");  break;
          default :                        sprintf(reason, "%s", "other");  break;
        }

        char json_name[160];
        sprintf(json_name, "{=camera=_=%.4s=,=date=_=%s=,=time=_=%s=,=temp=_=%04.1f=,=humid=_=%04.1f=,=press=_=%05.1f=,=lux=_=%05.1f=,=reason=_=%.5s=,=seq=_=%03d=}",
                "cam1", strfbuf_date, strfbuf_time, temperature[i], humidity[i], pressure[i], lux[i], reason, i);

        //Serial.printf("The json_name >>%s<<, len %d\n", json_name, sizeof(json_name));

        sprintf(azure_filename, "%s/%s.jpg", strfbuf_ymd, json_name);
        Serial.printf("Filename >>%s<<, len %d\n", azure_filename, sizeof(azure_filename));

        long send_start = millis();

        if (IoTHubDeviceClient_LL_UploadMultipleBlocksToBlob(device_ll_handle, azure_filename, getDataCallback, NULL) != IOTHUB_CLIENT_OK) {
          Serial.printf("UploadMultipleBlocksToBlob failed\n");
        } else {
          Serial.printf("UploadMultipleBlocksToBlob success\n");
        }

        long send_done = millis();
        float kbps = 8.0 * fblen / (send_done - send_start) ;
        Serial.printf("Send %d bytes in %.1f seconds for %.1f kbps\n\n", fblen, (send_done - send_start) / 1000.0, kbps );
        Serial.printf("IP %s, Rssi %d\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());

        free(fbbuf);
        //ram();
      }
    }

    // Clean up the iothub sdk handle
    IoTHubDeviceClient_LL_Destroy(device_ll_handle);
  }
  // Free all the sdk subsystem
  IoTHub_Deinit();
}

void take_n_photo() {
  camera_fb_t * fb = NULL;

  for (int i = 0; i < num_pics; i++) {
    init_camera();
    Serial.println(" 3.    SMILE   "); delay(200);
    Serial.println(" 2.    SMILE   "); delay(200);
    Serial.println(" 1.    SMILE   "); delay(200);
    
    fb = get_good_jpeg();
    Serial.println("       DONE   ");
    if (!fb) {
      Serial.println("Camera capture failed");
      break;
    } else {
      framebuffer[i] = (uint8_t*)ps_malloc(512 * 1024);
      framebuffer_len[i] = fb->len;

      time_t now;
      time(&now);
      framebuffer_now[i] = now;

      memcpy(framebuffer[i], fb->buf, fb->len);
      //ram();
      Serial.printf("pic %d, size %d\n", i, fb->len);
      esp_camera_fb_return(fb);
      deinit_camera();

      temperature[i] = get_bme_temperature();
      pressure[i] = get_bme_pressure();
      humidity[i] = get_bme_humidity();
      lux[i] = get_tsl_lux();

    }
    if (i < num_pics - 1 ) {
      Serial.printf("Delay for %3.1f seconds until next picture\n",ms_between_pictures / 1000.0);
      delay(ms_between_pictures);
    } else {
      sendPhotoToServer(framebuffer[i], framebuffer_len[i]);
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  get_good_jpeg()  - take a picture and make sure it has a good jpeg
//
camera_fb_t * get_good_jpeg() {

  camera_fb_t * fb;
  int failures = 0;

  do {
    long start = millis();
    fb = esp_camera_fb_get();
    Serial.printf(" --%4d ms-- ", millis() - start);
    if (!fb) {
      Serial.printf("Camera Capture Failed %d ", failures);
      failures++;
      sensor_t * ss = esp_camera_sensor_get();
      int qual = ss->status.quality ;
      ss->set_quality(ss, qual + 2);
      Serial.printf("Lower the quality to %d\n", qual + 2);
      delay(100);
    } else {
      Serial.printf("%6d bytes\n", fb->len);
      break;
    }
  } while (failures < 5);   // normally leave the loop with a break()
  return fb;
}

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void ram() {
  Serial.printf(" Ram %7d / %7d  ",  ESP.getFreeHeap(), ESP.getHeapSize());
  Serial.printf(" psRam %7d / %7d\n", ESP.getFreePsram(), ESP.getPsramSize() );
}

void init_camera() {
  long millisstart = millis();
  uint32_t heapstart = ESP.getFreeHeap();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound())
  {
    //Serial.println("We have psram ...");
    config.frame_size = FRAMESIZE_QSXGA; // FRAMESIZE_UXGA; //FRAMESIZE_VGA; //FRAMESIZE_UXGA; // FRAMESIZE_+QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 7;
    config.fb_count = 1;
  } else {
    Serial.println("NO PSRAM !!!");
  }

  //ram();

  // Initialise Camera
  Serial.printf("Camera init \n");
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  } else {
    Serial.printf("Camera init success \n");
  }

  //ram();
  delay(500);

  fb = esp_camera_fb_get();  esp_camera_fb_return(fb);

  sensor_t * ss = esp_camera_sensor_get();

  Serial.printf("\nCamera started correctly, Type is % x (hex) of 9650, 7725, 2640, 3660, 5640\n\n", ss->id.PID);

  if (ss->id.PID == OV5640_PID ) {
    ss->set_hmirror(ss, 1);        // 0 = disable , 1 = enable
  } else {
    ss->set_hmirror(ss, 0);        // 0 = disable , 1 = enable
    ss->set_framesize(ss, FRAMESIZE_UXGA);
    Serial.printf("Framesize is %d\n", ss->status.framesize);
  }
  delay(500);

  //ram();

  //sensor_t * s = esp_camera_sensor_get();
  //s->set_framesize(s, FRAMESIZE_SXGA);
  //delay(500);

  for (int i = 0; i < 2; i++) {
    fb = get_good_jpeg();
    if (fb) {
      esp_camera_fb_return(fb);
    }
  }
  Serial.println(" ");

  //ram();

  Serial.printf("Camera init % d ms, costing % d bytes\n", millis() - millisstart, - ESP.getFreeHeap() + heapstart);
}

void deinit_camera() {
  long millisstart = millis();
  uint32_t heapstart = ESP.getFreeHeap();

  esp_err_t err2 = esp_camera_deinit();
  if (err2 != ESP_OK) {
    Serial.printf("Camera deinit failed with error 0x % x", err2);
    return;
  }
  Serial.printf("Camera deinit % d ms, saving % d bytes\n", millis() - millisstart, - heapstart + ESP.getFreeHeap() );
  Serial.printf("IP %s, Rssi %d\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
}


const char* boundry = "jz629zj";

void sendPhotoToServer(uint8_t* fbbuf, int fblen) {
  long millisstart = millis();
  uint32_t heapstart = ESP.getFreeHeap();
  //ram();

  StaticJsonDocument<768> doc;
  WiFiClientSecure client;
  client.setInsecure();
  Serial.printf("Connecting to %s:%d... ", host, Port);

  if (!client.connect(host, Port)) {
    Serial.println("Failure in connection with the server");
    return;
  }
  //ram();

  String start_request = "";
  String end_request = "";

  long send_start = millis();

  start_request = start_request + "--" + boundry + "\r\n";
  start_request = start_request + "Content-Disposition: form-data; name=\"file\"; filename=\"CAM.jpg\"\r\n";
  start_request = start_request + "Content-Type: image/jpg\r\n";
  start_request = start_request + "\r\n";

  end_request = end_request + "\r\n";
  end_request = end_request + "--" + boundry + "--" + "\r\n";

  int contentLength = (int)fblen + start_request.length() + end_request.length();

  String headers = "POST https://canadacentral.api.cognitive.microsoft.com/vision/v3.2/describe?maxCandidates=1&language=en HTTP/1.1\r\n"; //edit for your server
  headers = headers + "Host: " + host + "\r\n";
  headers = headers + "User-Agent: ESP32" + "\r\n";
  headers = headers + "Accept: */*\r\n";
  headers = headers + "Content-Type: multipart/form-data; boundary=" + boundry + "\r\n";
  headers = headers + "Ocp-Apim-Subscription-Key: " + Ocp_Apim_Subscription_Key + "\r\n";
  headers = headers + "Content-Length: " + contentLength + "\r\n";
  headers = headers + "\r\n";
  client.print(headers);
  //Serial.print(headers);
  client.flush();

  //Serial.println(start_request);
  client.print(start_request);
  client.flush();

  int iteration = fblen / 1024;
  for (int i = 0; i < iteration; i++) {
    client.write(fbbuf, 1024);
    fbbuf += 1024;
    client.flush();
  }
  size_t remain = fblen % 1024;
  client.write(fbbuf, remain);
  client.flush();
  client.print(end_request);

  long send_done = millis();
  float kbps = 8.0 * fblen / (send_done - send_start) ;
  Serial.printf("Send %d bytes in %.1f seconds for %.1f kbps\n\n", fblen, (send_done - send_start) / 1000.0, kbps );

  // header response
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    //Serial.println(line);
    if (line == "\r") {
      //Serial.println("headers received\n\n");
      break;
    }
  }
  // response body
  String description;
  while (client.available()) {
    char c = client.read();
    description = description + c;
    Serial.write(c);
  }

  client.flush();

  deserializeJson(doc, description);
  const char* description_captions_0_text = doc["description"]["captions"][0]["text"];
  char descriptionWithFullStop[100];   // array to hold the result.
  strcpy(descriptionWithFullStop, description_captions_0_text); // copy string one into the result.
  strcat(descriptionWithFullStop, ".");
  float description_captions_0_confidence = doc["description"]["captions"][0]["confidence"];
  Serial.printf("\n\nAzure Computer Vision says => ");
  Serial.print(descriptionWithFullStop);
  Serial.printf(", confidence=%5.1f\n\n", description_captions_0_confidence * 100);

  //ram();
}

void waitForKeyPress() {
  Serial.println("Hit Enter in the Serial Monitor for another picture");
  while (!Serial.available()) {
    delay(100);
  }
  while (Serial.available()) {
    Serial.read();
  }
}

void sf_setup();
void dis_inter();
void en_inter();

void ADXL_ISR();

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 900 /* Time ESP32 will go to sleep (in seconds) */

void setup() {

  Serial.begin(115200);
  Serial.printf("\n\nESP32-AZURE-AI-CAM v.12...\n\n");
  delay(500);

  //ram();

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }

  ++bootCount;
  Serial.println("\nBoot number: " + String(bootCount));

  sf_setup();
  //ram();
  dis_inter();

  initWifi(ssid, pass);
  initTime();

  //ram();

  num_pics = number_of_pictures;
  take_n_photo();
  save_n_photo();
  send_n_photo();

  //ram();
  for (int j = 0; j < 1; j++) {
    for (int i = 0; i < 10; i++) {
      ADXL_ISR();
    }
  }

  dis_inter();
  ADXL_ISR();

  Serial.printf("High interrupt from adxl to gpio 4\n");
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 1);

  time_t now;
  time(&now);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  int secs_to_hour =  (60 - timeinfo.tm_min) * 60 + (60 - timeinfo.tm_sec) ;
  Serial.printf("Sleeping interrupt until top of hour = %d seconds\n", secs_to_hour);

  esp_sleep_enable_timer_wakeup(1ULL * secs_to_hour * uS_TO_S_FACTOR);

  en_inter();
  ADXL_ISR();

  Serial.println("Going to deepsleep, waiting for gpio 4 interupt from adxl or timer");
  delay(1000);
  esp_deep_sleep_start();
}

void loop() {
  // we never get here, as we sleep at end of setup
  // add this code for fiddling with moving and taping adxl to refine paramters, and test take/save/send photos
 
  /*
    for (int j = 0; j < 10; j++) {
    for (int i = 0; i < 1000; i++) {
      ADXL_ISR();
    }
    Serial.println(j);
    }
    waitForKeyPress();
    sf_setup();
    take_n_photo();
    save_n_photo();
    send_n_photo();
  */
}
