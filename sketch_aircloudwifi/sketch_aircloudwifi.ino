/*
William Ferrell
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

const char* ssid = "Unit 504";
const char* password = "3.141504";

//Your Domain name with URL path or IP address with path
const char* serverName = "http://us-central1-aircloud-300705.cloudfunctions.net/hello_world?pqid=87575"; //69509

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

String sensorReadings;
float sensorReadingsArr[3];

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


void setup() {
  Serial.begin(115200);

  FastLED.addLeds<NEOPIXEL, SIGNAL_PIN>(leds, NUM_LEDS); // strip 1
  FastLED.addLeds<NEOPIXEL, PWR_PIN>(leds1, NUM_LEDS1); // strip 1
  FastLED.addLeds<NEOPIXEL, WIFI_PIN>(leds2, NUM_LEDS1); // strip 2
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds3, NUM_LEDS1); // strip 3
  FastLED.addLeds<NEOPIXEL, ERR_PIN>(leds4, NUM_LEDS1); // strip 4

  FastLED.setBrightness(MAX_BRIGHTNESS);
  // Turn the LED on, then pause
  leds1[0] = CRGB::Green;

  FastLED.show();

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    Serial.println("No Wifi");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  leds2[0] = CRGB::Green;
  FastLED.show();

  // Now iterate through the various colors.
  for (int i = 0; i < NUM_LEDS; i++) {
    // Turn the LED on, then pause
    leds[i] = CRGB::Green;
  }
  FastLED.show();
  delay(2000);
  // Now iterate through the various colors.
  for (int i = 0; i < NUM_LEDS; i++) {
    // Turn the LED on, then pause
    leds[i] = CRGB::YellowGreen;
  }
  FastLED.show();
  delay(2000);
  // Now iterate through the various colors.
  for (int i = 0; i < NUM_LEDS; i++) {
    // Turn the LED on, then pause
    leds[i] = CRGB::Orange;
  }
  FastLED.show();
  delay(2000);
  // Now iterate through the various colors.
  for (int i = 0; i < NUM_LEDS; i++) {
    // Turn the LED on, then pause
    leds[i] = CRGB::Red;
  }
  FastLED.show();
  delay(2000);
  // Now iterate through the various colors.
  for (int i = 0; i < NUM_LEDS; i++) {
    // Turn the LED on, then pause
    leds[i] = CRGB::Purple;
  }
  FastLED.show();
  delay(2000);
  // Now iterate through the various colors.
  for (int i = 0; i < NUM_LEDS; i++) {
    // Turn the LED on, then pause
    leds[i] = CRGB::Maroon;
  }
  FastLED.show();
  delay(2000);  
  // Now iterate through the various colors.
  for (int i = 0; i < NUM_LEDS; i++) {
    // Turn the LED on, then pause
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  delay(2000);  

  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
  timerDelay = 5000;
}

void loop() {
  //Send an HTTP POST request every 10 minutes
  Serial.println("millis,  lastTime, timerDelay | START");
  Serial.println(millis());
  Serial.println(lastTime);
  Serial.println(timerDelay);
  Serial.println("millis,  lastTime, timerDelay | END");
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED) {
      Serial.println(serverName);
      sensorReadings = httpGETRequest(serverName);
      Serial.println("sensorReadings");
      Serial.println(sensorReadings);
      JSONVar myObject = JSON.parse(sensorReadings);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        leds4[0] = CRGB::Black;
        FastLED.show();
        delay(2000);
        leds4[0] = CRGB::Red;
        FastLED.show();
        Serial.println("Show Red 1");
        delay(500);
        timerDelay = 5000;
        return;
      }
    
      Serial.print("JSON object = ");
      Serial.println(myObject);
    
      // myObject.keys() can be used to get an array of all the keys in the object
      JSONVar keys = myObject.keys();
    
      for (int i = 0; i < keys.length(); i++) {
        leds3[0] = CRGB::Green;
        FastLED.show();
        delay(100);
        leds3[0] = CRGB::Purple;
        FastLED.show();
        delay(200);
        int value = myObject[keys[i]];
        Serial.print(keys[i]);
        Serial.print(" = ");
        Serial.println(value);
        JSONVar matchingValue = "aircloud_aqi";
        if (keys[i] == matchingValue) {
          Serial.println("value");
          Serial.println(value);
          if (value >= 0) {
            // First we want to count up to show the units.
            for (int i = 0; i < NUM_LEDS; i++) {
              // Turn the LED on, then pause
              leds[i] = CRGB::Black;
            }
            FastLED.show();
            delay(100);
            // Now set a color and after each we show
            int count_remaining = value;
//            Serial.println("count_remaining");
//            Serial.println(count_remaining);

            int counting_up = 0;
            for (int i = NUM_LEDS; i > 0; i--) {
              Serial.println(i);
              // Turn the LED on, then pause
              if (count_remaining >= 0) {
                if (counting_up < 50) {
                  int mappedValue = map(counting_up,0,50,96,64);
                  Serial.println("counting_up mappedValue");
                  Serial.println(mappedValue);
                  leds[i].setHSV( mappedValue, 255, 255);
                }
                else if (value < 100) {
                  int mappedValue = map(counting_up,50,100,64,32);
                  Serial.println("counting_up mappedValue");
                  Serial.println(mappedValue);
                  leds[i].setHSV( mappedValue, 255, 255);
                }
                else if (value < 150) {
                  int mappedValue = map(counting_up,100,150,32,0);
                  Serial.println("counting_up mappedValue");
                  Serial.println(mappedValue);
                  leds[i].setHSV( mappedValue, 255, 255);
                }
                else if (value < 200) {
                  int mappedValue = map(counting_up,150,200,255,224);
                  Serial.println("counting_up mappedValue");
                  Serial.println(mappedValue);
                  leds[i].setHSV( mappedValue, 255, 255);
                }
                FastLED.show();
                delay(750);
                counting_up = counting_up + 1;
              }
              count_remaining = count_remaining - 1;
//              Serial.println("count_remaining");
//              Serial.println(count_remaining);
            }
            delay(10000);
            if (value < 50) {
              Serial.println("MAPPING");
              int mappedValue = map(value,0,50,96,64);
              Serial.println(mappedValue);
              for (int i = 0; i < NUM_LEDS; i++) {
                // Turn the LED on, then pause
//                leds[i] = CRGB::Green;
                  Serial.println("Set leds with setHSV");
                  leds[i].setHSV( mappedValue, 255, 255);
              }
            }
            else if (value < 100) {
              Serial.println("MAPPING");
              int mappedValue = map(value,50,100,64,32);
              Serial.println(mappedValue);
              for (int i = 0; i < NUM_LEDS; i++) {
                // Turn the LED on, then pause
                  Serial.println("Set leds with setHSV");
                  leds[i].setHSV( mappedValue, 255, 255);
              }
            }
            else if (value < 150) {
              Serial.println("MAPPING");
              int mappedValue = map(value,100,150,32,0);
              Serial.println(mappedValue);
              for (int i = 0; i < NUM_LEDS; i++) {
                // Turn the LED on, then pause
                  Serial.println("Set leds with setHSV");
                  leds[i].setHSV( mappedValue, 255, 255);
              }
            }
            else if (value < 200) {
              Serial.println("MAPPING");
              int mappedValue = map(value,150,200,255,224);
              Serial.println(mappedValue);
              for (int i = 0; i < NUM_LEDS; i++) {
                // Turn the LED on, then pause
                  Serial.println("Set leds with setHSV");
                  leds[i].setHSV( mappedValue, 255, 255);
              }
            }
            else if (value < 300) {
              Serial.println("MAPPING");
              int mappedValue = map(value,200,300,224,192);
              Serial.println(mappedValue);
              for (int i = 0; i < NUM_LEDS; i++) {
                // Turn the LED on, then pause
                  Serial.println("Set leds with setHSV");
                  leds[i].setHSV( mappedValue, 255, 255);
              }
            }
            else if (value >= 300) {
              for (int i = 0; i < NUM_LEDS; i++) {
                // Turn the LED on, then pause
                leds[i] = CRGB::Maroon;
              }
            }
            leds3[0] = CRGB::Green;
            leds4[0] = CRGB::Black;
            Serial.println("Setting isBadData to 0");
            isBadData = 0;
          }
          else {
            // Now I want to run the error flow so set the badData
            Serial.println("Setting isBadData to 1");
            isBadData = 1;
            leds3[0] = CRGB::Black;
            leds4[0] = CRGB::Red;
          }

        }
        FastLED.show();
        delay(100);
        timerDelay = 30000;
        Serial.println(timerDelay);
        Serial.println("Processed!");
      }
    }
    else {
      Serial.println("WiFi Disconnected");
      leds4[0] = CRGB::Black;
      FastLED.show();
      delay(2000);
      leds4[0] = CRGB::Red;
      FastLED.show();
      Serial.println("Show Red 2");
      delay(500);
      leds4[0] = CRGB::Black;
      FastLED.show();
      return;
    }
    lastTime = millis();
  }
  else {
    Serial.println("isBadData");
    Serial.println(isBadData);
    if (isBadData == 1) {
      Serial.println("Bad data");
      rainbow_wave(10, 10);                                      // Speed, delta hue values.
      FastLED.show();
//      for (int i = 0; i < NUM_LEDS; i++) {
//        // Turn the LED on, then pause
//        leds[i] = CRGB::Red;
//      }
//      leds4[0] = CRGB::Red;
//      FastLED.show();
//      delay(50);
//      leds4[0] = CRGB::Black;
//      for (int i = 0; i < NUM_LEDS; i++) {
//        // Turn the LED on, then pause
//        leds[i] = CRGB::Black;
//      }
//      FastLED.show();
//      delay(50);
//      for (int i = 0; i < NUM_LEDS; i++) {
//        // Turn the LED on, then pause
//        leds[i] = CRGB::Red;
//      }
//      leds4[0] = CRGB::Red;
//      FastLED.show();
//      delay(50);

    
    
    }
    else {
      float smoothness_pts = 500;//larger=slower change in brightness  
      
      float gamma = 0.14; // affects the width of peak (more or less darkness)
      float beta = 0.5; // shifts the gaussian to be symmetric
      Serial.println("Breathing");
      for (int ii=0;ii<smoothness_pts;ii++){
        float pwm_val = 70.0*(exp(-(pow(((ii/smoothness_pts)-beta)/gamma,2.0))/2.0));
        if (pwm_val < 8.0) {
          pwm_val = 8.0;
        }
        FastLED.setBrightness(pwm_val);
        FastLED.show();
        delay(5);
      }
      Serial.println("Breathing");
      for (int ii=0;ii<smoothness_pts;ii++){
        float pwm_val = 70.0*(exp(-(pow(((ii/smoothness_pts)-beta)/gamma,2.0))/2.0));
        if (pwm_val < 8.0) {
          pwm_val = 10.0;
        }        
        FastLED.setBrightness(pwm_val);
        FastLED.show();
        delay(5);
      }
      Serial.println("Breathing");
      for (int ii=0;ii<smoothness_pts;ii++){
        float pwm_val = 70.0*(exp(-(pow(((ii/smoothness_pts)-beta)/gamma,2.0))/2.0));
        if (pwm_val < 8.0) {
          pwm_val = 8.0;
        }        
        FastLED.setBrightness(pwm_val);
        FastLED.show();
        delay(5);
      }
      delay(timerDelay);
    }    
  }

}

void rainbow_wave(uint8_t thisSpeed, uint8_t deltaHue) {     // The fill_rainbow call doesn't support brightness levels.
 
// uint8_t thisHue = beatsin8(thisSpeed,0,255);                // A simple rainbow wave.
 uint8_t thisHue = beat8(thisSpeed,255);                     // A simple rainbow march.
  
 fill_rainbow(leds, NUM_LEDS, thisHue, deltaHue);            // Use FastLED's fill_rainbow routine.
 
} // rainbow_wave()


String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;

  Serial.println(serverName);

  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();

  Serial.println(httpResponseCode);

  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
