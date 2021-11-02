/*
  esp32-ai-cam

    by James Zahary    jamzah.plc@gmail.com

    https://github.com/jameszah/ESP32-AI_CAM

    jameszah/ESP32-AI_CAM is licensed under the
    GNU General Public License v3.0

*/

#include <stdio.h>
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "FS.h"
#include <SD_MMC.h>

void ram();

char avi_file_name[100];
File avifile;


extern esp_sleep_wakeup_cause_t wakeup_reason;

void sd_jpeg_write(uint8_t * fbbuf, int fblen, time_t now, float temperature, float humidity, float lux, float pressure, int i) {

  long millisstart = millis();
  uint32_t heapstart = ESP.getFreeHeap();

  int succ = SD_MMC.begin("/sdcard", true);
  if (succ) {
    Serial.printf("SD_MMC Begin: %d\n", succ);
    uint8_t cardType = SD_MMC.cardType();
    Serial.print("SD_MMC Card Type: ");
    if (cardType == CARD_MMC) {
      Serial.println("MMC");
    } else if (cardType == CARD_SD) {
      Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
      Serial.println("SDHC");
    } else {
      Serial.println("UNKNOWN");
    }

    Serial.printf("SD Mount % d ms, costing % d bytes\n", millis() - millisstart, - ESP.getFreeHeap() + heapstart);

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);

    char strfbuf_date[22];
    char strfbuf_time[22];

    char azure_filename[180];
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    char strfbuf_ymd[22];

    strftime(strfbuf_ymd, sizeof(strfbuf_ymd), "/%Y%m%d", &timeinfo);
    //Serial.printf("strfbuf_ymd >>%s<<, len %d\n", strfbuf_ymd, sizeof(strfbuf_ymd));
    strftime(strfbuf_date, sizeof(strfbuf_date), "%F", &timeinfo);
    //Serial.printf("strfbuf_date >>%s<<, len %d\n", strfbuf_date, sizeof(strfbuf_date));
    strftime(strfbuf_time, sizeof(strfbuf_time), "%H-%M-%S", &timeinfo);
    //Serial.printf("strfbuf_time >>%s<<, len %d\n", strfbuf_time, sizeof(strfbuf_time));

    SD_MMC.mkdir(strfbuf_ymd);

    char reason[8];

    switch (wakeup_reason)
    {
      case ESP_SLEEP_WAKEUP_EXT0 :     sprintf(reason, "%s", "axdl3");  break;
      case ESP_SLEEP_WAKEUP_EXT1 :     sprintf(reason, "%s", "other");  break;
      case ESP_SLEEP_WAKEUP_TIMER :    sprintf(reason, "%s", "timer");  break;
      case ESP_SLEEP_WAKEUP_TOUCHPAD : sprintf(reason, "%s", "other");  break;
      case ESP_SLEEP_WAKEUP_ULP :      sprintf(reason, "%s", "other");  break;
      default :                        sprintf(reason, "%s", "other");  break;
    }

    //strftime(strfbuf, sizeof(strfbuf), "%F_%H-%M-%S", &timeinfo);
    //Serial.printf("The date >>%s<<, len %d\n", strfbuf, sizeof(strfbuf));
    char json_name[160];
    sprintf(json_name, "{=camera=_=%.4s=,=date=_=%s=,=time=_=%s=,=temp=_=%04.1f=,=humid=_=%04.1f=,=press=_=%05.1f=,=lux=_=%05.1f=,=reason=_=%.5s=,=seq=_=%03d=}",
            "cam1", strfbuf_date, strfbuf_time, temperature, humidity, pressure, lux, reason, i);

    //Serial.printf("The json_name >>%s<<, len %d\n", json_name, sizeof(json_name));
    sprintf(azure_filename, "%s/%s.jpg", strfbuf_ymd, json_name);
        
    //Serial.printf("Filename >>%s<<, len %d\n", azure_filename, sizeof(azure_filename));
    long send_start = millis();

    avifile = SD_MMC.open(azure_filename, "w");
    if (avifile) {
      int err = avifile.write(fbbuf,  fblen);
      if (err != fblen) {
        Serial.print("Error on avi write: err = "); Serial.print(err);
        Serial.print(" len = "); Serial.println(fblen);
      }
      ram();
      avifile.close();
    } else {
      Serial.printf("Couldn't open the file %d\n", avifile);
    }
    millisstart = millis();
    heapstart = ESP.getFreeHeap();
    SD_MMC.end();
    Serial.printf("SD Dis-Mount % d ms, saving % d bytes\n", millis() - millisstart, + ESP.getFreeHeap() - heapstart);

  } else {
    Serial.printf("Failed to mount SD card VFAT filesystem. \n");
    Serial.println("Do you have an SD Card installed?");
    Serial.println("Check pin 12 and 13, not grounded, or grounded with 10k resistors!\n\n");
  }
}
