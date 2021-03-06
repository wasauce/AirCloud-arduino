/*
William Ferrell
*/


#if !( defined(ESP8266) ||  defined(ESP32) )
  #error This code is intended to run on the ESP8266 or ESP32 platform! Please check your Tools->Board setting.
#endif

#define ESP_ASYNC_WIFIMANAGER_VERSION_MIN_TARGET     "ESPAsync_WiFiManager v1.9.2"

// Use from 0 to 4. Higher number, more debugging messages and memory usage.
#define _ESPASYNC_WIFIMGR_LOGLEVEL_    1

#include <FS.h>

//Ported to ESP32
#ifdef ESP32
  #include <esp_wifi.h>
  #include <WiFi.h>
  #include <WiFiClient.h>

  // From v1.1.1
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;

  // LittleFS has higher priority than SPIFFS
  #if ( ARDUINO_ESP32C3_DEV )
    // Currently, ESP32-C3 only supporting SPIFFS and EEPROM. Will fix to support LittleFS
    #define USE_LITTLEFS          false
    #define USE_SPIFFS            true
  #else
    #define USE_LITTLEFS    true
    #define USE_SPIFFS      false
  #endif

  #if USE_LITTLEFS
    // Use LittleFS
    #include "FS.h"

    // The library has been merged into esp32 core release 1.0.6
    #include <LITTLEFS.h>             // https://github.com/lorol/LITTLEFS
    
    FS* filesystem =      &LITTLEFS;
    #define FileFS        LITTLEFS
    #define FS_Name       "LittleFS"
  #elif USE_SPIFFS
    #include <SPIFFS.h>
    FS* filesystem =      &SPIFFS;
    #define FileFS        SPIFFS
    #define FS_Name       "SPIFFS"
  #else
    // +Use FFat
    #include <FFat.h>
    FS* filesystem =      &FFat;
    #define FileFS        FFat
    #define FS_Name       "FFat"
  #endif
  //////

  #define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())

  #define LED_BUILTIN       2
  #define LED_ON            HIGH
  #define LED_OFF           LOW

#else

  #include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
  //needed for library
  #include <DNSServer.h>

  // From v1.1.1
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;

  #define USE_LITTLEFS      true
  
  #if USE_LITTLEFS
    #include <LittleFS.h>
    FS* filesystem =      &LittleFS;
    #define FileFS        LittleFS
    #define FS_Name       "LittleFS"
  #else
    FS* filesystem =      &SPIFFS;
    #define FileFS        SPIFFS
    #define FS_Name       "SPIFFS"
  #endif
  //////
  
  #define ESP_getChipId()   (ESP.getChipId())
  
  #define LED_ON      LOW
  #define LED_OFF     HIGH
#endif

// These defines must be put before #include <ESP_DoubleResetDetector.h>
// to select where to store DoubleResetDetector's variable.
// For ESP32, You must select one to be true (EEPROM or SPIFFS)
// For ESP8266, You must select one to be true (RTC, EEPROM, SPIFFS or LITTLEFS)
// Otherwise, library will use default EEPROM storage
#ifdef ESP32

  // These defines must be put before #include <ESP_DoubleResetDetector.h>
  // to select where to store DoubleResetDetector's variable.
  // For ESP32, You must select one to be true (EEPROM or SPIFFS)
  // Otherwise, library will use default EEPROM storage
  #if USE_LITTLEFS
    #define ESP_DRD_USE_LITTLEFS    true
    #define ESP_DRD_USE_SPIFFS      false
    #define ESP_DRD_USE_EEPROM      false
  #elif USE_SPIFFS
    #define ESP_DRD_USE_LITTLEFS    false
    #define ESP_DRD_USE_SPIFFS      true
    #define ESP_DRD_USE_EEPROM      false
  #else
    #define ESP_DRD_USE_LITTLEFS    false
    #define ESP_DRD_USE_SPIFFS      false
    #define ESP_DRD_USE_EEPROM      true
  #endif

#else //ESP8266

  // For DRD
  // These defines must be put before #include <ESP_DoubleResetDetector.h>
  // to select where to store DoubleResetDetector's variable.
  // For ESP8266, You must select one to be true (RTC, EEPROM, SPIFFS or LITTLEFS)
  // Otherwise, library will use default EEPROM storage
  #if USE_LITTLEFS
    #define ESP_DRD_USE_LITTLEFS    true
    #define ESP_DRD_USE_SPIFFS      false
  #else
    #define ESP_DRD_USE_LITTLEFS    false
    #define ESP_DRD_USE_SPIFFS      true
  #endif
  
  #define ESP_DRD_USE_EEPROM      false
  #define ESP8266_DRD_USE_RTC     false
#endif

#define DOUBLERESETDETECTOR_DEBUG       true  //false

#include <ESP_DoubleResetDetector.h>      //https://github.com/khoih-prog/ESP_DoubleResetDetector

// Number of seconds after reset during which a
// subseqent reset will be considered a double reset.
#define DRD_TIMEOUT 10

// RTC Memory Address for the DoubleResetDetector to use
#define DRD_ADDRESS 0

//DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);
DoubleResetDetector* drd;//////

// Onboard LED I/O pin on NodeMCU board
const int PIN_LED = 2; // D4 on NodeMCU and WeMos. GPIO2/ADC12 of ESP32. Controls the onboard LED.

// SSID and PW for Config Portal
String ssid = "AirCloudWIFI"; //"ESP_" + String(ESP_getChipId(), HEX);
String password;

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

// From v1.1.1
// You only need to format the filesystem once
//#define FORMAT_FILESYSTEM       true
#define FORMAT_FILESYSTEM         false

#define MIN_AP_PASSWORD_SIZE    8

#define SSID_MAX_LEN            32
//From v1.0.10, WPA2 passwords can be up to 63 characters long.
#define PASS_MAX_LEN            64

typedef struct
{
  char wifi_ssid[SSID_MAX_LEN];
  char wifi_pw  [PASS_MAX_LEN];
}  WiFi_Credentials;

typedef struct
{
  String wifi_ssid;
  String wifi_pw;
}  WiFi_Credentials_String;

#define NUM_WIFI_CREDENTIALS      2

// Assuming max 491 chars
#define TZNAME_MAX_LEN            50
#define TIMEZONE_MAX_LEN          50

typedef struct
{
  WiFi_Credentials  WiFi_Creds [NUM_WIFI_CREDENTIALS];
  char TZ_Name[TZNAME_MAX_LEN];     // "America/Toronto"
  char TZ[TIMEZONE_MAX_LEN];        // "EST5EDT,M3.2.0,M11.1.0"
  uint16_t checksum;
} WM_Config;

WM_Config         WM_config;

#define  CONFIG_FILENAME              F("/wifi_cred.dat")
//////

// Indicates whether ESP has WiFi credentials saved from previous session, or double reset detected
bool initialConfig = false;

// Use false if you don't like to display Available Pages in Information Page of Config Portal
// Comment out or use true to display Available Pages in Information Page of Config Portal
// Must be placed before #include <ESPAsync_WiFiManager.h>
#define USE_AVAILABLE_PAGES     true  //false

// From v1.0.10 to permit disable/enable StaticIP configuration in Config Portal from sketch. Valid only if DHCP is used.
// You'll loose the feature of dynamically changing from DHCP to static IP, or vice versa
// You have to explicitly specify false to disable the feature.
//#define USE_STATIC_IP_CONFIG_IN_CP          false
#define USE_STATIC_IP_CONFIG_IN_CP          false


// Use false to disable NTP config. Advisable when using Cellphone, Tablet to access Config Portal.
// See Issue 23: On Android phone ConfigPortal is unresponsive (https://github.com/khoih-prog/ESP_WiFiManager/issues/23)
#define USE_ESP_WIFIMANAGER_NTP     false

// Just use enough to save memory. On ESP8266, can cause blank ConfigPortal screen
// if using too much memory
#define USING_AFRICA        false
#define USING_AMERICA       true
#define USING_ANTARCTICA    false
#define USING_ASIA          false
#define USING_ATLANTIC      false
#define USING_AUSTRALIA     false
#define USING_EUROPE        false
#define USING_INDIAN        false
#define USING_PACIFIC       false
#define USING_ETC_GMT       false

// Use true to enable CloudFlare NTP service. System can hang if you don't have Internet access while accessing CloudFlare
// See Issue #21: CloudFlare link in the default portal (https://github.com/khoih-prog/ESP_WiFiManager/issues/21)
#define USE_CLOUDFLARE_NTP          false

// New in v1.0.11
#define USING_CORS_FEATURE          true
//////

// Use USE_DHCP_IP == true for dynamic DHCP IP, false to use static IP which you have to change accordingly to your network
#if (defined(USE_STATIC_IP_CONFIG_IN_CP) && !USE_STATIC_IP_CONFIG_IN_CP)
// Force DHCP to be true
#if defined(USE_DHCP_IP)
#undef USE_DHCP_IP
#endif
#define USE_DHCP_IP     true
#else
// You can select DHCP or Static IP here
//#define USE_DHCP_IP     true
#define USE_DHCP_IP     false
#endif

#if ( USE_DHCP_IP )
// Use DHCP
#warning Using DHCP IP
IPAddress stationIP   = IPAddress(0, 0, 0, 0);
IPAddress gatewayIP   = IPAddress(192, 168, 1, 1);
IPAddress netMask     = IPAddress(255, 255, 255, 0);
#else
// Use static IP
#warning Using static IP

#ifdef ESP32
IPAddress stationIP   = IPAddress(192, 168, 2, 232);
#else
IPAddress stationIP   = IPAddress(192, 168, 2, 186);
#endif

IPAddress gatewayIP   = IPAddress(192, 168, 2, 1);
IPAddress netMask     = IPAddress(255, 255, 255, 0);
#endif

#define USE_CONFIGURABLE_DNS      true

IPAddress dns1IP      = gatewayIP;
IPAddress dns2IP      = IPAddress(8, 8, 8, 8);

#define USE_CUSTOM_AP_IP          false

IPAddress APStaticIP  = IPAddress(192, 168, 100, 1);
IPAddress APStaticGW  = IPAddress(192, 168, 100, 1);
IPAddress APStaticSN  = IPAddress(255, 255, 255, 0);

#include <ESPAsync_WiFiManager.h>              //https://github.com/khoih-prog/ESPAsync_WiFiManager


#define HTTP_PORT     80

// AirCloud Imports and Prep


#include <HTTPClient.h>

//Your Domain name with URL path or IP address with path
const char* serverName = "http://us-central1-aircloud-300705.cloudfunctions.net/hello_world?pqid=35433"; //69509

char serverNameBase[] = "http://us-central1-aircloud-300705.cloudfunctions.net/hello_world?pqid="; //69509
String serverNameCombinedDone;
String purpleairPQIDDone;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
//unsigned long timerDelay = 5000;
// Set timer to 30 seconds (30000)
unsigned long timerDelay = 30000;
bool isBadData = 0;
bool isTimeOutMax = 0;


#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 183
#define NUM_LEDS1 1

#define SIGNAL_PIN 16 //16 SIGNAL
#define PWR_PIN 17 // PWR
#define WIFI_PIN 18 // WIFI
#define DATA_PIN 19 // DATA
#define ERR_PIN 21 // ERR

#define MAX_BRIGHTNESS 10

// Define the array of leds
CRGB leds[NUM_LEDS];

CRGB leds1[NUM_LEDS1];
CRGB leds2[NUM_LEDS1];
CRGB leds3[NUM_LEDS1];
CRGB leds4[NUM_LEDS1];



#include <ArduinoJson.h>

char configFileName[] = "/config.json";

//define your default values here, if there are different values in configFileName (config.json), they are overwritten.
#define PURPLEAIR_ID_LEN                6

char purpleair_id   [PURPLEAIR_ID_LEN]   = "XXXXX";


//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println(F("Should save config"));
  shouldSaveConfig = true;
}

bool loadFileFSConfigFile()
{
  //clean FS, for testing
  //FileFS.format();

  //read configuration from FS json
  Serial.println(F("Mounting FS..."));

  if (FileFS.begin())
  {
    Serial.println(F("Mounted file system"));

    if (FileFS.exists(configFileName))
    {
      //file exists, reading and loading
      Serial.println(F("Reading config file"));
      File configFile = FileFS.open(configFileName, "r");

      if (configFile)
      {
        Serial.print(F("Opened config file, size = "));
        size_t configFileSize = configFile.size();
        Serial.println(configFileSize);

        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[configFileSize + 1]);

        configFile.readBytes(buf.get(), configFileSize);

        Serial.print(F("\nJSON parseObject() result : "));

#if (ARDUINOJSON_VERSION_MAJOR >= 6)
        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get(), configFileSize);

        if ( deserializeError )
        {
          Serial.println(F("failed"));
          return false;
        }
        else
        {
          Serial.println(F("OK"));

          if (json["purpleair_id"])
            strncpy(purpleair_id, json["purpleair_id"], sizeof(purpleair_id));
         
        }

        //serializeJson(json, Serial);
        serializeJsonPretty(json, Serial);
#else
        DynamicJsonBuffer jsonBuffer;
        // Parse JSON string
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        // Test if parsing succeeds.

        if (json.success())
        {
          Serial.println("OK");

          if (json["purpleair_id"])
            strncpy(purpleair_id, json["purpleair_id"], sizeof(purpleair_id));

        }
        else
        {
          Serial.println(F("failed"));
          return false;
        }
        //json.printTo(Serial);
        json.prettyPrintTo(Serial);
#endif

        configFile.close();
      }
    }
  }
  else
  {
    Serial.println(F("failed to mount FS"));
    return false;
  }
  return true;
}

bool saveFileFSConfigFile()
{
  Serial.println(F("Saving config"));

#if (ARDUINOJSON_VERSION_MAJOR >= 6)
  DynamicJsonDocument json(1024);
#else
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
#endif

  json["purpleair_id"] = purpleair_id;

  File configFile = FileFS.open(configFileName, "w");

  if (!configFile)
  {
    Serial.println(F("Failed to open config file for writing"));

    return false;
  }

#if (ARDUINOJSON_VERSION_MAJOR >= 6)
  //serializeJson(json, Serial);
  serializeJsonPretty(json, Serial);
  // Write data to file and close it
  serializeJson(json, configFile);
#else
  //json.printTo(Serial);
  json.prettyPrintTo(Serial);
  // Write data to file and close it
  json.printTo(configFile);
#endif

  configFile.close();
  //end save

  return true;
}

///////////////////////////////////////////
// New in v1.4.0
/******************************************
   // Defined in ESPAsync_WiFiManager.h
  typedef struct
  {
    IPAddress _ap_static_ip;
    IPAddress _ap_static_gw;
    IPAddress _ap_static_sn;
  }  WiFi_AP_IPConfig;

  typedef struct
  {
    IPAddress _sta_static_ip;
    IPAddress _sta_static_gw;
    IPAddress _sta_static_sn;
    #if USE_CONFIGURABLE_DNS
    IPAddress _sta_static_dns1;
    IPAddress _sta_static_dns2;
    #endif
  }  WiFi_STA_IPConfig;
******************************************/

WiFi_AP_IPConfig  WM_AP_IPconfig;
WiFi_STA_IPConfig WM_STA_IPconfig;

void initAPIPConfigStruct(WiFi_AP_IPConfig &in_WM_AP_IPconfig)
{
  in_WM_AP_IPconfig._ap_static_ip   = APStaticIP;
  in_WM_AP_IPconfig._ap_static_gw   = APStaticGW;
  in_WM_AP_IPconfig._ap_static_sn   = APStaticSN;
}

void initSTAIPConfigStruct(WiFi_STA_IPConfig &in_WM_STA_IPconfig)
{
  in_WM_STA_IPconfig._sta_static_ip   = stationIP;
  in_WM_STA_IPconfig._sta_static_gw   = gatewayIP;
  in_WM_STA_IPconfig._sta_static_sn   = netMask;
#if USE_CONFIGURABLE_DNS
  in_WM_STA_IPconfig._sta_static_dns1 = dns1IP;
  in_WM_STA_IPconfig._sta_static_dns2 = dns2IP;
#endif
}

void displayIPConfigStruct(WiFi_STA_IPConfig in_WM_STA_IPconfig)
{
  LOGERROR3(F("stationIP ="), in_WM_STA_IPconfig._sta_static_ip, ", gatewayIP =", in_WM_STA_IPconfig._sta_static_gw);
  LOGERROR1(F("netMask ="), in_WM_STA_IPconfig._sta_static_sn);
#if USE_CONFIGURABLE_DNS
  LOGERROR3(F("dns1IP ="), in_WM_STA_IPconfig._sta_static_dns1, ", dns2IP =", in_WM_STA_IPconfig._sta_static_dns2);
#endif
}

void configWiFi(WiFi_STA_IPConfig in_WM_STA_IPconfig)
{
#if USE_CONFIGURABLE_DNS
  // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5
  WiFi.config(in_WM_STA_IPconfig._sta_static_ip, in_WM_STA_IPconfig._sta_static_gw, in_WM_STA_IPconfig._sta_static_sn, in_WM_STA_IPconfig._sta_static_dns1, in_WM_STA_IPconfig._sta_static_dns2);
#else
  // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
  WiFi.config(in_WM_STA_IPconfig._sta_static_ip, in_WM_STA_IPconfig._sta_static_gw, in_WM_STA_IPconfig._sta_static_sn);
#endif
}

///////////////////////////////////////////

uint8_t connectMultiWiFi()
{
#if ESP32
  // For ESP32, this better be 0 to shorten the connect time.
  // For ESP32-S2/C3, must be > 500
  #if ( USING_ESP32_S2 || USING_ESP32_C3 )
    #define WIFI_MULTI_1ST_CONNECT_WAITING_MS           500L
  #else
    // For ESP32 core v1.0.6, must be >= 500
    #define WIFI_MULTI_1ST_CONNECT_WAITING_MS           800L
  #endif
#else
  // For ESP8266, this better be 2200 to enable connect the 1st time
  #define WIFI_MULTI_1ST_CONNECT_WAITING_MS             2200L
#endif

#define WIFI_MULTI_CONNECT_WAITING_MS                   500L

  uint8_t status;

  //WiFi.mode(WIFI_STA);

  LOGERROR(F("ConnectMultiWiFi with :"));

  if ( (Router_SSID != "") && (Router_Pass != "") )
  {
    LOGERROR3(F("* Flash-stored Router_SSID = "), Router_SSID, F(", Router_Pass = "), Router_Pass );
    LOGERROR3(F("* Add SSID = "), Router_SSID, F(", PW = "), Router_Pass );
    wifiMulti.addAP(Router_SSID.c_str(), Router_Pass.c_str());
  }

  for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
  {
    // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
    if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
    {
      LOGERROR3(F("* Additional SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
    }
  }

  LOGERROR(F("Connecting MultiWifi..."));

  //WiFi.mode(WIFI_STA);

#if !USE_DHCP_IP
  // New in v1.4.0
  configWiFi(WM_STA_IPconfig);
  //////
#endif

  int i = 0;
  status = wifiMulti.run();
  delay(WIFI_MULTI_1ST_CONNECT_WAITING_MS);

  while ( ( i++ < 20 ) && ( status != WL_CONNECTED ) )
  {
    status = WiFi.status();

    if ( status == WL_CONNECTED )
      break;
    else
      delay(WIFI_MULTI_CONNECT_WAITING_MS);
  }

  if ( status == WL_CONNECTED )
  {
    LOGERROR1(F("WiFi connected after time: "), i);
    LOGERROR3(F("SSID:"), WiFi.SSID(), F(",RSSI="), WiFi.RSSI());
    LOGERROR3(F("Channel:"), WiFi.channel(), F(",IP address:"), WiFi.localIP() );
  }
  else
  {
    LOGERROR(F("WiFi not connected"));

    // To avoid unnecessary DRD
    drd->loop();
  
#if ESP8266      
    ESP.reset();
#else
    ESP.restart();
#endif  
  }

  return status;
}

#if USE_ESP_WIFIMANAGER_NTP

void printLocalTime()
{
#if ESP8266
  static time_t now;
  
  now = time(nullptr);
  
  if ( now > 1451602800 )
  {
    Serial.print("Local Date/Time: ");
    Serial.print(ctime(&now));
  }
#else
  struct tm timeinfo;

  getLocalTime( &timeinfo );

  // Valid only if year > 2000. 
  // You can get from timeinfo : tm_year, tm_mon, tm_mday, tm_hour, tm_min, tm_sec
  if (timeinfo.tm_year > 100 )
  {
    Serial.print("Local Date/Time: ");
    Serial.print( asctime( &timeinfo ) );
  }
#endif
}

#endif

void heartBeatPrint()
{
#if USE_ESP_WIFIMANAGER_NTP
  printLocalTime();
#else
  static int num = 1;

  if (WiFi.status() == WL_CONNECTED)
    Serial.print(F("H"));        // H means connected to WiFi
  else
    Serial.print(F("F"));        // F means not connected to WiFi

  if (num == 80)
  {
    Serial.println();
    num = 1;
  }
  else if (num++ % 10 == 0)
  {
    Serial.print(F(" "));
  }
#endif  
}

void check_WiFi()
{
  if ( (WiFi.status() != WL_CONNECTED) )
  {
    Serial.println(F("\nWiFi lost. Call connectMultiWiFi in loop"));
    connectMultiWiFi();
  }
}

void check_status()
{
  static ulong checkstatus_timeout  = 0;
  static ulong checkwifi_timeout    = 0;

  static ulong current_millis;

#define WIFICHECK_INTERVAL    1000L

#if USE_ESP_WIFIMANAGER_NTP
  #define HEARTBEAT_INTERVAL    60000L
#else
  #define HEARTBEAT_INTERVAL    10000L
#endif  

  current_millis = millis();

  // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
  if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0))
  {
    check_WiFi();
    checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;
  }

  // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
  if ((current_millis > checkstatus_timeout) || (checkstatus_timeout == 0))
  {
    heartBeatPrint();
    checkstatus_timeout = current_millis + HEARTBEAT_INTERVAL;
  }
}

int calcChecksum(uint8_t* address, uint16_t sizeToCalc)
{
  uint16_t checkSum = 0;
  
  for (uint16_t index = 0; index < sizeToCalc; index++)
  {
    checkSum += * ( ( (byte*) address ) + index);
  }

  return checkSum;
}

bool loadConfigData()
{
  File file = FileFS.open(CONFIG_FILENAME, "r");
  LOGERROR(F("LoadWiFiCfgFile "));

  memset((void *) &WM_config,       0, sizeof(WM_config));

  // New in v1.4.0
  memset((void *) &WM_STA_IPconfig, 0, sizeof(WM_STA_IPconfig));
  //////

  if (file)
  {
    file.readBytes((char *) &WM_config,   sizeof(WM_config));

    // New in v1.4.0
    file.readBytes((char *) &WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
    //////

    file.close();
    LOGERROR(F("OK"));

    if ( WM_config.checksum != calcChecksum( (uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum) ) )
    {
      LOGERROR(F("WM_config checksum wrong"));
      
      return false;
    }
    
    // New in v1.4.0
    displayIPConfigStruct(WM_STA_IPconfig);
    //////

    return true;
  }
  else
  {
    LOGERROR(F("failed"));

    return false;
  }
}

void saveConfigData()
{
  File file = FileFS.open(CONFIG_FILENAME, "w");
  LOGERROR(F("SaveWiFiCfgFile "));

  if (file)
  {
    WM_config.checksum = calcChecksum( (uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum) );
    
    file.write((uint8_t*) &WM_config, sizeof(WM_config));

    displayIPConfigStruct(WM_STA_IPconfig);

    // New in v1.4.0
    file.write((uint8_t*) &WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
    //////

    file.close();
    LOGERROR(F("OK"));
  }
  else
  {
    LOGERROR(F("failed"));
  }
}


String httpGETRequest(const char* serverName) {
  HTTPClient http;

  Serial.println(serverName);

  // Your Domain name with URL path or IP address with path
  http.begin(serverName);

  // set the timeout
  http.setTimeout(10000);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();

  Serial.println(httpResponseCode);

  Serial.println( http.errorToString(httpResponseCode) );
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    if (httpResponseCode == -11) {
      Serial.print("negative 11");
      Serial.print("negative 11");
      payload = "TIMEOUT";
    }
  }
  // Free resources
  http.end();

  return payload;
}


void breathe()
{
  float smoothness_pts = 500;//larger=slower change in brightness  
  
  float gamma = 0.14; // affects the width of peak (more or less darkness)
  float beta = 0.5; // shifts the gaussian to be symmetric
  float topPWM = 50.0; // this is the top value that could be hit.
  Serial.println("Breathing");
  for (int ii=0;ii<smoothness_pts;ii++){
    float pwm_val = topPWM*(exp(-(pow(((ii/smoothness_pts)-beta)/gamma,2.0))/2.0));
    if (pwm_val < 8.0) {
      pwm_val = 8.0;
    }
    FastLED.setBrightness(pwm_val);
    FastLED.show();
    delay(5);
    Serial.println("pwm_val");
    Serial.println(pwm_val);

  }
  Serial.println("Breathing");
  for (int ii=0;ii<smoothness_pts;ii++){
    float pwm_val = topPWM*(exp(-(pow(((ii/smoothness_pts)-beta)/gamma,2.0))/2.0));
    if (pwm_val < 8.0) {
      pwm_val = 10.0;
    }        
    FastLED.setBrightness(pwm_val);
    FastLED.show();
    delay(5);
  }
  Serial.println("Breathing");
  for (int ii=0;ii<smoothness_pts;ii++){
    float pwm_val = topPWM*(exp(-(pow(((ii/smoothness_pts)-beta)/gamma,2.0))/2.0));
    if (pwm_val < 8.0) {
      pwm_val = 8.0;
    }        
    FastLED.setBrightness(pwm_val);
    FastLED.show();
    delay(5);
  }
}

// Allocate the JSON document
//
// Inside the brackets, 200 is the capacity of the memory pool in bytes.
// Don't forget to change this value to match your JSON document.
// Use arduinojson.org/v6/assistant to compute the capacity.
StaticJsonDocument<275> doc;

void aircloudDisplay()
{
  Serial.println("aircloudDisplay - serverNameCombinedDone");
  Serial.println(serverNameCombinedDone);

  Serial.println(serverName);
  const char* serverNameCombinedDoneChar = serverNameCombinedDone.c_str();
  String sensorReadings = httpGETRequest(serverNameCombinedDoneChar);
  Serial.println("sensorReadings");
  Serial.println(sensorReadings);

  if (sensorReadings == "TIMEOUT") {
    if (isTimeOutMax) {
      sensorReadings == "{}";
    }
    else {
      isTimeOutMax = 1;
      Serial.println("SensorReadings are TIMEOUT sp we will attempt to re-fetch");
      Serial.println("TimeoutCount is acceptable");
      timerDelay = 5000;
      return;      
    }
  }

  if (sensorReadings == "{}") {
    Serial.println("SensorReadings are {} - so bad data.");
    leds4[0] = CRGB::Black;
    FastLED.show();
    delay(2000);
    leds4[0] = CRGB::Red;
    FastLED.show();
    Serial.println("Show Red 1");
    delay(500);
    Serial.println("Setting isBadData to 1");
    isBadData = 1;
    isTimeOutMax = 0;

    leds3[0] = CRGB::Black;
    leds4[0] = CRGB::Red;
    timerDelay = 5000;
    return;
  }


  // StaticJsonDocument<N> allocates memory on the stack, it can be
  // replaced by DynamicJsonDocument which allocates in the heap.
  //
  // DynamicJsonDocument doc(200);

  // JSON input string.
  //
  // Using a char[], as shown here, enables the "zero-copy" mode. This mode uses
  // the minimal amount of memory because the JsonDocument stores pointers to
  // the input buffer.
  // If you use another type of input, ArduinoJson must copy the strings from
  // the input to the JsonDocument, so you need to increase the capacity of the
  // JsonDocument.

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, sensorReadings);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    leds4[0] = CRGB::Black;
    FastLED.show();
    delay(2000);
    leds4[0] = CRGB::Red;
    FastLED.show();
    Serial.println("Show Red 1");
    delay(500);
    Serial.println("Setting isBadData to 1");
    isBadData = 1;
    isTimeOutMax = 0;
    leds3[0] = CRGB::Black;
    leds4[0] = CRGB::Red;
    timerDelay = 5000;
    return;
  }

  Serial.println("purpleairPQIDDone");
  Serial.println(purpleairPQIDDone);

  String aircloud_pqid = doc["id"];
  Serial.println("aircloud_pqid");
  Serial.println(aircloud_pqid);

  int aircloud_aqi = doc["aircloud_aqi"];
  Serial.println("aircloud_aqi");
  Serial.println(aircloud_aqi);
  leds3[0] = CRGB::Green;
  FastLED.show();
  delay(100);
  leds3[0] = CRGB::Purple;
  leds4[0] = CRGB::Black;
  FastLED.show();
  delay(200);


  if (aircloud_aqi >= 0) {
    // First we want to count up to show the units.
    // So set the leds to black
    for (int i = 0; i < NUM_LEDS; i++) {
      // Turn the LED on, then pause
      leds[i] = CRGB::Black;
    }
    FastLED.show();
    delay(100);
    // Now set a color and after each we show
    int count_remaining = aircloud_aqi;

    int counting_up = 0;
    for (int i = NUM_LEDS; i >= 0; i--) {
      Serial.println(i);
      // Turn the LED on, then pause
      if (count_remaining >= 0) {
        if (counting_up < 50) {
          int mappedValue = map(counting_up,0,50,96,64);
          Serial.println("counting_up mappedValue");
          Serial.println(mappedValue);
          leds[i].setHSV( mappedValue, 255, 255);
        }
        else if (counting_up < 100) {
          int mappedValue = map(counting_up,50,100,64,32);
          Serial.println("counting_up mappedValue");
          Serial.println(mappedValue);
          leds[i].setHSV( mappedValue, 255, 255);
        }
        else if (counting_up < 150) {
          int mappedValue = map(counting_up,100,150,32,0);
          Serial.println("counting_up mappedValue");
          Serial.println(mappedValue);
          leds[i].setHSV( mappedValue, 255, 255);
        }
        else if (counting_up < 200) {
          int mappedValue = map(counting_up,150,200,255,224);
          Serial.println("counting_up mappedValue");
          Serial.println(mappedValue);
          leds[i].setHSV( mappedValue, 255, 255);
        }
        else if (counting_up < 300) {
          int mappedValue = map(counting_up,200,300,224,192);
          Serial.println("counting_up mappedValue");
          Serial.println(mappedValue);
          leds[i].setHSV( mappedValue, 255, 255);
        }
        else if (counting_up >= 300) {
          leds[i] = CRGB::DarkViolet;
        }
        FastLED.show();
        delay(500);
        counting_up = counting_up + 1;
        if (i == 0) {
          i = NUM_LEDS;
        }
      }
      count_remaining = count_remaining - 1;
    }
    delay(10000);
    if (aircloud_aqi < 50) {
      Serial.println("MAPPING < 50");
      int mappedValue = map(aircloud_aqi,0,50,96,64);
      Serial.println(mappedValue);
      for (int i = 0; i < NUM_LEDS; i++) {
          leds[i].setHSV( mappedValue, 255, 255);
      }
    }
    else if (aircloud_aqi < 100) {
      Serial.println("MAPPING < 100");
      int mappedValue = map(aircloud_aqi,50,100,64,32);
      Serial.println(mappedValue);
      for (int i = 0; i < NUM_LEDS; i++) {
          leds[i].setHSV( mappedValue, 255, 255);
      }
    }
    else if (aircloud_aqi < 150) {
      Serial.println("MAPPING < 150");
      int mappedValue = map(aircloud_aqi,100,150,32,0);
      Serial.println(mappedValue);
      for (int i = 0; i < NUM_LEDS; i++) {
          leds[i].setHSV( mappedValue, 255, 255);
      }
    }
    else if (aircloud_aqi < 200) {
      Serial.println("MAPPING < 200");
      int mappedValue = map(aircloud_aqi,150,200,255,224);
      Serial.println(mappedValue);
      for (int i = 0; i < NUM_LEDS; i++) {
          leds[i].setHSV( mappedValue, 255, 255);
      }
    }
    else if (aircloud_aqi < 300) {
      Serial.println("MAPPING < 300");
      int mappedValue = map(aircloud_aqi,200,300,224,192);
      Serial.println(mappedValue);
      for (int i = 0; i < NUM_LEDS; i++) {
          leds[i].setHSV( mappedValue, 255, 255);
      }
    }
    else if (aircloud_aqi < 400) {
      Serial.println("MAPPING < 400");
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::DarkViolet;
      }
      // Set the bottom row, white
      for (int i = NUM_LEDS-14; i < NUM_LEDS; i++) {
        leds[i] = CRGB::WhiteSmoke;
      }
    }
    else if (aircloud_aqi >= 400) {
      Serial.println("MAPPING >= 400");
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::DarkViolet;
      }
      // Set the bottom row, white
      for (int i = NUM_LEDS-14; i < NUM_LEDS; i++) {
        leds[i] = CRGB::WhiteSmoke;
      }
      // Set the rest of the rim, white
      leds[0] = CRGB::WhiteSmoke;
      leds[1] = CRGB::WhiteSmoke;
      leds[2] = CRGB::WhiteSmoke;
      leds[5] = CRGB::WhiteSmoke;
      leds[6] = CRGB::WhiteSmoke;
      leds[11] = CRGB::WhiteSmoke;
      leds[12] = CRGB::WhiteSmoke;
      leds[19] = CRGB::WhiteSmoke;
      leds[20] = CRGB::WhiteSmoke;
      leds[21] = CRGB::WhiteSmoke;
      leds[22] = CRGB::WhiteSmoke;
      leds[30] = CRGB::WhiteSmoke;
      leds[31] = CRGB::WhiteSmoke;
      leds[43] = CRGB::WhiteSmoke;
      leds[44] = CRGB::WhiteSmoke;
      leds[45] = CRGB::WhiteSmoke;
      leds[46] = CRGB::WhiteSmoke;
      leds[62] = CRGB::WhiteSmoke;
      leds[63] = CRGB::WhiteSmoke;
      leds[80] = CRGB::WhiteSmoke;
      leds[81] = CRGB::WhiteSmoke;
      leds[98] = CRGB::WhiteSmoke;
      leds[99] = CRGB::WhiteSmoke;
      leds[116] = CRGB::WhiteSmoke;
      leds[117] = CRGB::WhiteSmoke;
      leds[134] = CRGB::WhiteSmoke;
      leds[135] = CRGB::WhiteSmoke;
      leds[152] = CRGB::WhiteSmoke;
      leds[153] = CRGB::WhiteSmoke;
      leds[168] = CRGB::WhiteSmoke;
    }

    leds3[0] = CRGB::Green;
    leds4[0] = CRGB::Black;
    Serial.println("Setting isBadData to 0");
    isBadData = 0;
    isTimeOutMax = 0;

    if (aircloud_pqid != purpleairPQIDDone) {
      Serial.println("They are NOT equal.");
      leds3[0] = CRGB::Yellow;
    }
    FastLED.show();
    delay(100);
    breathe();
    timerDelay = 60000*10; // This is where the refresh cycle is controlled.
    Serial.println("Processed!");
  }
  else {
    Serial.println("Setting isBadData to 1");
    isBadData = 1;
    isTimeOutMax = 0;
    leds3[0] = CRGB::Black;
    leds4[0] = CRGB::Red;
    timerDelay = 5000;    
  }

}

// Light up the main AirCloud LEDS and do a little warm up routine.
void setupDance()
{
    // Now iterate through the various colors.
    for (int i = 0; i < NUM_LEDS; i++) {
      // Turn the LED on, then pause
      leds[i] = CRGB::Green;
    }
    FastLED.show();
    delay(1000);
    // Now iterate through the various colors.
    for (int i = 0; i < NUM_LEDS; i++) {
      // Turn the LED on, then pause
      leds[i] = CRGB::YellowGreen;
    }
    FastLED.show();
    delay(1000);
    // Now iterate through the various colors.
    for (int i = 0; i < NUM_LEDS; i++) {
      // Turn the LED on, then pause
      leds[i] = CRGB::Red;
    }
    FastLED.show();
    delay(1000);
    // Now iterate through the various colors.
    for (int i = 0; i < NUM_LEDS; i++) {
      // Turn the LED on, then pause
      leds[i] = CRGB::Purple;
    }
    FastLED.show();
    delay(1000);
    // Now iterate through the various colors.
    for (int i = 0; i < NUM_LEDS; i++) {
      // Turn the LED on, then pause
      leds[i] = CRGB::Maroon;
    }
    FastLED.show();
    delay(1000);  
    // Now iterate through the various colors.
    for (int i = 0; i < NUM_LEDS; i++) {
      // Turn the LED on, then pause
      leds[i] = CRGB::Black;
    }
    FastLED.show();
    delay(1000);  
    // Now do the fast move up
    for (int j = 0; j < 256; j++) {
      Serial.println(j);
      for (int i = NUM_LEDS; i >= 0; i--) {
        // Turn the LED on, then pause
        leds[i].setHSV( j, 255, 255);
        FastLED.show();
        delay(10);
      }
      j = j + 25;
    }  
}


void setup()
{
  // put your setup code here, to run once:
  // initialize the LED digital pin as an output.
  Serial.begin(115200);
  while (!Serial);

  delay(200);

  FastLED.addLeds<NEOPIXEL, SIGNAL_PIN>(leds, NUM_LEDS); // strip 1
  FastLED.addLeds<NEOPIXEL, PWR_PIN>(leds1, NUM_LEDS1); // strip 1
  FastLED.addLeds<NEOPIXEL, WIFI_PIN>(leds2, NUM_LEDS1); // strip 2
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds3, NUM_LEDS1); // strip 3
  FastLED.addLeds<NEOPIXEL, ERR_PIN>(leds4, NUM_LEDS1); // strip 4

  FastLED.setBrightness(MAX_BRIGHTNESS);
  // Turn the LED on, then pause
  leds1[0] = CRGB::Purple;
  leds2[0] = CRGB::Purple;
  leds3[0] = CRGB::Purple;
  leds4[0] = CRGB::Purple;

  FastLED.show();

  Serial.print(F("\nStarting Async_ConfigOnDoubleReset using ")); Serial.print(FS_Name);
  Serial.print(F(" on ")); Serial.println(ARDUINO_BOARD);
  Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION);
  Serial.println(ESP_DOUBLE_RESET_DETECTOR_VERSION);

  if ( String(ESP_ASYNC_WIFIMANAGER_VERSION) < ESP_ASYNC_WIFIMANAGER_VERSION_MIN_TARGET )
  {
    Serial.print("Warning. Must use this example on Version later than : ");
    Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION_MIN_TARGET);
  }

  Serial.setDebugOutput(false);

  if (FORMAT_FILESYSTEM)
    FileFS.format();

  // Format FileFS if not yet
#ifdef ESP32
  if (!FileFS.begin(true))
#else
  if (!FileFS.begin())
#endif
  {
#ifdef ESP8266
    FileFS.format();
#endif

    Serial.println(F("SPIFFS/LittleFS failed! Already tried formatting."));
  
    if (!FileFS.begin())
    {     
      // prevents debug info from the library to hide err message.
      delay(100);
      
#if USE_LITTLEFS
      Serial.println(F("LittleFS failed!. Please use SPIFFS or EEPROM. Stay forever"));
#else
      Serial.println(F("SPIFFS failed!. Please use LittleFS or EEPROM. Stay forever"));
#endif

      while (true)
      {
        delay(1);
      }
    }
  }
  
  loadFileFSConfigFile();

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  ESPAsync_WMParameter custom_purpleair_id("purpleair_id", "purpleair_id", purpleair_id, PURPLEAIR_ID_LEN + 1);

  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);

  unsigned long startedAt = millis();

  // New in v1.4.0
  initAPIPConfigStruct(WM_AP_IPconfig);
  initSTAIPConfigStruct(WM_STA_IPconfig);
  //////

  //Local intialization. Once its business is done, there is no need to keep it around
  // Use this to default DHCP hostname to ESP8266-XXXXXX or ESP32-XXXXXX
  //ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer);
  // Use this to personalize DHCP hostname (RFC952 conformed)
  AsyncWebServer webServer(HTTP_PORT);

#if ( USING_ESP32_S2 || USING_ESP32_C3 )
  ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, NULL, "AirCloud");
#else
  DNSServer dnsServer;

  ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, "AirCloud");
#endif

  //set config save notify callback
  ESPAsync_wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  ESPAsync_wifiManager.addParameter(&custom_purpleair_id);

#if USE_CUSTOM_AP_IP
  //set custom ip for portal
  // New in v1.4.0
  ESPAsync_wifiManager.setAPStaticIPConfig(WM_AP_IPconfig);
  //////
#endif

  ESPAsync_wifiManager.setMinimumSignalQuality(-1);

  // From v1.0.10 only
  // Set config portal channel, default = 1. Use 0 => random channel from 1-11
  ESPAsync_wifiManager.setConfigPortalChannel(0);
  //////

#if !USE_DHCP_IP
  // Set (static IP, Gateway, Subnetmask, DNS1 and DNS2) or (IP, Gateway, Subnetmask). New in v1.0.5
  // New in v1.4.0
  ESPAsync_wifiManager.setSTAStaticIPConfig(WM_STA_IPconfig);
  //////
#endif

  // New from v1.1.1
#if USING_CORS_FEATURE
  ESPAsync_wifiManager.setCORSHeader("Your Access-Control-Allow-Origin");
#endif

  // We can't use WiFi.SSID() in ESP32 as it's only valid after connected.
  // SSID and Password stored in ESP32 wifi_ap_record_t and wifi_config_t are also cleared in reboot
  // Have to create a new function to store in EEPROM/SPIFFS for this purpose
  Router_SSID = ESPAsync_wifiManager.WiFi_SSID();
  Router_Pass = ESPAsync_wifiManager.WiFi_Pass();

  //Remove this line if you do not want to see WiFi password printed
  Serial.println("ESP Self-Stored: SSID = " + Router_SSID + ", Pass = " + Router_Pass);

  // SSID to uppercase
  ssid.toUpperCase();
  password  = "aircloud"; //"My" + ssid;

  bool configDataLoaded = false;

  // From v1.1.0, Don't permit NULL password
  if ( (Router_SSID != "") && (Router_Pass != "") )
  {
    LOGERROR3(F("* Add SSID = "), Router_SSID, F(", PW = "), Router_Pass);
    wifiMulti.addAP(Router_SSID.c_str(), Router_Pass.c_str());

    ESPAsync_wifiManager.setConfigPortalTimeout(120); //If no access point name has been previously entered disable timeout.
    Serial.println(F("Got ESP Self-Stored Credentials. Timeout 120s for Config Portal"));
  }
  
  if (loadConfigData())
  {
    configDataLoaded = true;

    ESPAsync_wifiManager.setConfigPortalTimeout(120); //If no access point name has been previously entered disable timeout.
    Serial.println(F("Got stored Credentials. Timeout 120s for Config Portal"));

#if USE_ESP_WIFIMANAGER_NTP      
    if ( strlen(WM_config.TZ_Name) > 0 )
    {
      LOGERROR3(F("Current TZ_Name ="), WM_config.TZ_Name, F(", TZ = "), WM_config.TZ);

  #if ESP8266
      configTime(WM_config.TZ, "pool.ntp.org"); 
  #else
      //configTzTime(WM_config.TZ, "pool.ntp.org" );
      configTzTime(WM_config.TZ, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
  #endif   
    }
    else
    {
      Serial.println(F("Current Timezone is not set. Enter Config Portal to set."));
    } 
#endif
  }
  else
  {
    // Enter CP only if no stored SSID on flash and file
    Serial.println(F("Open Config Portal without Timeout: No stored Credentials."));
    initialConfig = true;
  }

  if (drd->detectDoubleReset())
  {
    // DRD, disable timeout.
    ESPAsync_wifiManager.setConfigPortalTimeout(0);

    Serial.println(F("Open Config Portal without Timeout: Double Reset Detected"));
    initialConfig = true;
  }

  if (initialConfig)
  {
    Serial.print(F("Starting configuration portal @ "));
    
#if USE_CUSTOM_AP_IP    
    Serial.print(APStaticIP);
#else
    Serial.print(F("192.168.4.1"));
#endif

    Serial.print(F(", SSID = "));
    Serial.print(ssid);
    Serial.print(F(", PWD = "));
    Serial.println(password);

    digitalWrite(PIN_LED, LED_ON); // turn the LED on by making the voltage LOW to tell us we are in configuration mode.

    //sets timeout in seconds until configuration portal gets turned off.
    //If not specified device will remain in configuration mode until
    //switched off via webserver or device is restarted.
    //ESPAsync_wifiManager.setConfigPortalTimeout(600);

    // Starts an access point
    if (!ESPAsync_wifiManager.startConfigPortal((const char *) ssid.c_str(), password.c_str()))
      Serial.println(F("Not connected to WiFi but continuing anyway."));
    else
    {
      Serial.println(F("WiFi connected...yeey :)"));
    }

    // Stored  for later usage, from v1.1.0, but clear first
    memset(&WM_config, 0, sizeof(WM_config));

    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      String tempSSID = ESPAsync_wifiManager.getSSID(i);
      String tempPW   = ESPAsync_wifiManager.getPW(i);

      if (strlen(tempSSID.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1)
        strcpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str());
      else
        strncpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1);

      if (strlen(tempPW.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1)
        strcpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str());
      else
        strncpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1);

      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
      {
        LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
        wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
      }
    }

#if USE_ESP_WIFIMANAGER_NTP      
    String tempTZ   = ESPAsync_wifiManager.getTimezoneName();

    if (strlen(tempTZ.c_str()) < sizeof(WM_config.TZ_Name) - 1)
      strcpy(WM_config.TZ_Name, tempTZ.c_str());
    else
      strncpy(WM_config.TZ_Name, tempTZ.c_str(), sizeof(WM_config.TZ_Name) - 1);

    const char * TZ_Result = ESPAsync_wifiManager.getTZ(WM_config.TZ_Name);
    
    if (strlen(TZ_Result) < sizeof(WM_config.TZ) - 1)
      strcpy(WM_config.TZ, TZ_Result);
    else
      strncpy(WM_config.TZ, TZ_Result, sizeof(WM_config.TZ_Name) - 1);
         
    if ( strlen(WM_config.TZ_Name) > 0 )
    {
      LOGERROR3(F("Saving current TZ_Name ="), WM_config.TZ_Name, F(", TZ = "), WM_config.TZ);

#if ESP8266
      configTime(WM_config.TZ, "pool.ntp.org"); 
#else
      //configTzTime(WM_config.TZ, "pool.ntp.org" );
      configTzTime(WM_config.TZ, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
#endif
    }
    else
    {
      LOGERROR(F("Current Timezone Name is not set. Enter Config Portal to set."));
    }
#endif

    // New in v1.4.0
    ESPAsync_wifiManager.getSTAStaticIPConfig(WM_STA_IPconfig);
    //////

    saveConfigData();
  }

  digitalWrite(PIN_LED, LED_OFF); // Turn led off as we are not in configuration mode.

  startedAt = millis();

  if (!initialConfig)
  {
    // Load stored data, the addAP ready for MultiWiFi reconnection
    if (!configDataLoaded)
      loadConfigData();

    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
      {
        LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
        wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
      }
    }

    if ( WiFi.status() != WL_CONNECTED )
    {
      Serial.println(F("ConnectMultiWiFi in setup"));

      connectMultiWiFi();
    }
  }

  Serial.print(F("After waiting "));
  Serial.print((float) (millis() - startedAt) / 1000);
  Serial.print(F(" secs more in setup(), connection result is "));

  if (WiFi.status() == WL_CONNECTED)
  {

    Serial.print(F("connected. Local IP: "));
    Serial.println(WiFi.localIP());   

    leds1[0] = CRGB::Green;
    leds2[0] = CRGB::Green;
    leds3[0] = CRGB::Black;
    leds4[0] = CRGB::Black;
    FastLED.show();

    setupDance();

    Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
    timerDelay = 5000;  
  
  }
  else
    Serial.println(ESPAsync_wifiManager.getStatus(WiFi.status()));

  //read updated parameters
  strncpy(purpleair_id, custom_purpleair_id.getValue(), sizeof(purpleair_id));

  String serverNameCombined;
  serverNameCombined = serverNameBase;
  serverNameCombinedDone = serverNameCombined + custom_purpleair_id.getValue();
  Serial.println("serverNameCombinedDone");
  Serial.println(serverNameCombinedDone);

  purpleairPQIDDone = custom_purpleair_id.getValue();
  Serial.println("purpleairPQIDDone");
  Serial.println(purpleairPQIDDone);

  //save the custom parameters to FS
  if (shouldSaveConfig)
  {
    saveFileFSConfigFile();
  }

}

void loop()
{
  // Call the double reset detector loop method every so often,
  // so that it can recognise when the timeout expires.
  // You can also call drd.stop() when you wish to no longer
  // consider the next reset as a double reset.
  drd->loop();

  // put your main code here, to run repeatedly
  check_status();

  Serial.println("millis,  lastTime, timerDelay | START");
  Serial.println(millis());
  Serial.println(lastTime);
  Serial.println(timerDelay);
  Serial.println("millis,  lastTime, timerDelay | END");
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED) {
      Serial.println("Loop - WIFI connected -- getting ready to run.");
      aircloudDisplay();
      lastTime = millis();
    }
  }
  else {
    if (isBadData == 1) {
      Serial.println("Bad data");
      rainbow_wave(10, 10);                                      // Speed, delta hue values.
      FastLED.show();
    }
    else {
      // Flash the data light real quick to indicate that it is alive.
      leds3[0] = CRGB::Black;
      FastLED.show();
      delay(50);
      leds3[0] = CRGB::Green;
      FastLED.show();
      delay(50);
      leds3[0] = CRGB::Black;
      FastLED.show();
      delay(50);
      leds3[0] = CRGB::Green;
      FastLED.show();
      // We can take a delay for a minute as it is not bad data. Nothing to do.
      delay(60000);
    }
  }
}


void rainbow_wave(uint8_t thisSpeed, uint8_t deltaHue) {     // The fill_rainbow call doesn't support brightness levels.
 
// uint8_t thisHue = beatsin8(thisSpeed,0,255);                // A simple rainbow wave.
 uint8_t thisHue = beat8(thisSpeed,255);                     // A simple rainbow march.
  
 fill_rainbow(leds, NUM_LEDS, thisHue, deltaHue);            // Use FastLED's fill_rainbow routine.
 
} // rainbow_wave()
