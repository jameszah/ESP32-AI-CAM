/*
  esp32-ai-cam

    by James Zahary    jamzah.plc@gmail.com

    https://github.com/jameszah/ESP32-AI-CAM

    jameszah/ESP32-AI_CAM is licensed under the
    GNU General Public License v3.0

*/

#define IOT_CONFIG_WIFI_SSID            "..."
#define IOT_CONFIG_WIFI_PASSWORD        "..."

/*String containing Hostname, Device Id & Device Key in the format:                         */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"                */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>"    */
//static const char* connectionString = "[device connection string]";
static const char* connectionString = "HostName=myhubname.azure-devices.net;DeviceId=jzLogger;SharedAccessKey=............................................";

const char* host = "myazureregion.api.cognitive.microsoft.com"; 
const char* Ocp_Apim_Subscription_Key = "................................";
const int Port = 443;

#define ms_between_pictures 10000
#define number_of_pictures 2
