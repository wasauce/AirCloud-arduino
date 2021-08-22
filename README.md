# AirCloud-arduino



![AirCloudProduction](https://user-images.githubusercontent.com/41729/130364989-d0984486-b9fb-4146-8b6d-ec51d1f5bc67.png)



## Setup Instructions

1. **Gather your materials**

   Gather your AirCloud, MicroUSB cord, Smartphone, and WiFi credentials.

   Like most IoT devices, AirCloud needs a non-5G network.

   

2. **Find the right PurpleAir ID**

   On a laptop, go to https://www.purpleair.com/map and find the id of the PurpleAir air sensor you want the AirCloud to display.

   For my use, I found one in my neighborhood. You can see it here: https://www.purpleair.com/map?opt=1/i/mAQI/a10/cC0&select=35433#14.35/37.7596/-122.37854

   

   Click on the one you want, then copy the URL -- you need to extract the ID of the sensor.

   Using the URL above, the sensor ID is `35433` -- see it comes after the `select=` in the URL.

   Write this sensor ID number down. You will need it in step 6.

   If you have problems, call me -- we will figure it out together.

   

3. **Power Up**

   Connect the USB cable to the AirCloud and to a power source.

   The AirCloud should now light up with 4 purple lights.

   ![IMG_1376](https://user-images.githubusercontent.com/41729/130365055-9d876e0d-0a79-4687-b868-61872e68e190.jpeg)


   

4. **Connect to Wifi**

   Pull out your smartphone, go to the settings and go to where you select your WiFi network.

   You want to connect to the WiFi network called "AIRCLOUDWIFI". The password is: aircloud

   

   ![IMG_1377](https://user-images.githubusercontent.com/41729/130365071-d6f0535c-bbac-4220-8b2b-1c02771b647b.PNG)

   

   After connecting, a pop-up dialog should appear. This is where you will select what WiFi network you want to connect the PCB to, and set the which sensor ID.

   

   ![IMG_1378](https://user-images.githubusercontent.com/41729/130365089-8e4b590c-4c31-4d48-a6c1-2ad70a828a18.PNG)


   

   From the list of Wifi networks you see, select your WiFi network (again -- non-5G) and put in your WiFi password. To be clear you are giving this information to the AirCloud -- not to the AirCloud's servers.

   You will input this information twice. (Or you can put in the info for two different WiFi networks).

   

   Then take the PurpleAir sensor ID -- from step 2 -- and type it in in the `purpleair_id` field.

   Then hit `Save`

   
   ![IMG_1379](https://user-images.githubusercontent.com/41729/130365096-8f7badaf-5816-411b-83d1-556790dbcfe6.PNG)

   

   The pop-up dialog should now dismiss.

   The AirCloud will attempt to connect to WiFi and boot up.

   

   If it fails to connect to your WiFi:

   

   The 4 system lights will remain purple. Wait at least 30 seconds before starting over. Most likely, you mistyped your WiFi password. If you need to do a full reset, please see the FAQs below.

   

   If it succeeds, your should see the cloud light up:


https://user-images.githubusercontent.com/41729/130365437-9f895091-fe3f-44fe-8009-93f59bd6155e.mp4


   See a full video of the setup process on an iPhone here:


https://user-images.githubusercontent.com/41729/130365375-3e15a8ac-a207-44d5-80d4-a78e2277ef3e.mp4



5. **All Done!**

   The AirCloud will now have the top two lights (Power and Wifi) light up. The other 2 will change to black. The Cloud will now do it's Setup Dance and walk through some colors. Then, if all things are working, you should see it start to count up to the current AQI from your SensorID.



https://youtu.be/4pnXt1TztCo



-------------



# FAQs



## Update Interval

The AirCloud updates every 10 minutes.

You will see the Data light turn purple, then the cloud will go black, and then it will count up 1 LED per AQI PM2.5 value. So if the PM2.5 is 50, 50 lights will light up. If AQI is 500 -- the full cloud will light up. There are 183 LEDs in the cloud -- so if the AQI is > 183 -- it will count up to 183 and then change colors to the final value. If the AQI is > 183 you are already in trouble.



## Restarting Frequently

If your AirCloud is restarting frequently (i.e. goes black, and does the setupDance and then counts up) this means one of two things is happening:

1. There is a bug in the firmware and it is crashing. Yikes. Only solution is to tell me.
2. The power supplied to the AirCloud is not sufficient. I have seen this a few times when I have not properly connected the USB cord -- so some power is flowing but not a sufficient amount. This can also happen if the power supply is bad (e.g. you are powering it from a battery pack, or a laptop running out of power). Try re-connecting the power cord AND / OR switching power supplies.




## What it looks like (Explaining the different colors)

### 	Setup Dance



### 	Status Lights (left hand side)



### 	Counting up



### 	Breathing





### 	AQI Color Mapping

   ![AqiChart](https://user-images.githubusercontent.com/41729/130365106-8eba599b-d2b4-4b62-914f-67797c90bee9.png)

The > 301 AQI / Hazardous level is a VERY bright purple on the AirCloud. I hope you never see it.



### Rainbow Cloud / Moving Rainbow Cloud

<GIF>



If you see a flashing rainbow cloud it means there is an error! If it goes away after a few seconds or a couple of minutes -- it probably means the server was down and has come back online. You can email me and I can try to remotely debug.



## Turn on my AirCloud

1. Plug in your AirCloud

2. Hit the reset button ONCE. The reset button is on the back. If it still does not power on, hit the reset button again after waiting 10 seconds.

<Photo>




Are you changing locations and going to need to connect to a new Wifi network? If so, read about how to Reset your AirCloud.



## How to Reset your AirCloud / How to change Wifi Credentials / How to change PurpleAir Ids

1. Plug in your AirCloud
2. Hit the reset button multiple times -- I would say hit it every 5 seconds for 20 seconds (so 4 times).
3. Go follow the setup instructions -- starting with step XX

<Photo>




You know you have succeed in resetting if the 4 status lights on the left hand side are purple.

<Photo>



----

## Want to hack on the arduino code?

This repository has the arduino code running on the AirCloud. Please feel free to modify it as you wish! I'd love any and all PRs with improvements.



You will of course need Arudino and then you will need to install the following libraries done through the Library Manager:

Arduino_JSON - 0.1.0

Adafruit BusIO - 1.6.0 # I don't actually think this is needed but I do have it installed

Adafruit Circuit Playground 1.11.2

Adafruit DMA neopixel library 1.2.0

Adafruit GFX Library 1.10.3

Adafruit GPS Library 1.5.3

Adafruit ILI9341 1.5.6

Adafruit LED Backpack Library 1.1.8

Adafruit SleepyDog Library 1.3.2

Adafruit NeoPixel 1.2.0

Adafruit STMPE610 1.1.2

Adafruit TouchScreen 1.1.1

Adafruit Zero DMA Library 1.0.8

Adafruit Zero PDM Library 1.2.0

ArduinoJson 6.18.1

ESP_DoubleResetDetector 1.1.1

ESP_MultiResetDetector 1.1.1

ESPAsync_WiFiManager 1.9.2

FastLED 3.4.0

HttpClient 2.2.0

is31fl3733 1.0.1

LittleFS_esp32 1.0.6

RTClib 1.12.3

WiFiManager 2.0.3-alpha
