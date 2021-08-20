# AirCloud-arduino

Install the following arduino libraries:

Arduino_JSON - 0.1.0
WiFi - 1.2.7


## Setup Instructions

1. Gather your materials

   Gather your AirCloud, MicroUSB cord, Smartphone, and WiFi credentials (Like most IoT devices, AirCloud needs a non-5G network.)

2. Find the right PurpleAir ID

   On a laptop, go to https://www.purpleair.com/map and find the id of the PurpleAir air sensor you want the AirCloud to display.

   For my use, I found one in my neighborhood. You can see it here: https://www.purpleair.com/map?opt=1/i/mAQI/a10/cC0&select=35433#14.35/37.7596/-122.37854

   Click on the one you want, then copy the URL -- you need to extract the ID of the sensor.

   Using the URL above, the sensor ID is 35433 -- see it comes after the `select=` in the URL.

   Write this sensor ID number down. You will need it in step 6.

   If you have problems, call me -- we will figure it out together.

3. Power Up

   Connect the USB cable to the AirCloud and to a power source.

   The AirCloud should now light up with 4 purple lights.

   <Photo>

4. Connect to Wifi

   Pull out your smartphone, go to the settings and go to where you select your WiFi network.

   You want to connect to the WiFi network called "AIRCLOUDWIFI". The password is: aircloud

   After connecting, a pop-up dialog should appear. This is where you will select what WiFi network you want to connect the PCB to, and set the which sensor ID.

   From the list of Wifi networks you see, select your WIFi network (again -- non-5G) and put in your WiFi password. To be clear you are giving this information to the AirCloud -- not to the AirCloud's servers.

   You will input this information twice.

   Then take the PurpleAir sensor ID -- from step 2 -- and type it in in the XYZ field.

   Then hit submit.

   <Screenshot>

   The pop-up dialog should now dismiss.

   The AirCloud will attempt to connect to Wifi and boot up.

   If it fails to connect:

   XXX

   If it succeeds, your should see the cloud light up

   <GIF>


   See a full video of the setup process here:

   << IDEO OF FULL SETUP>

5. All Done!

   The AirCloud will now have the top two lights (Power and Wifi) light up. The other 2 will change to black. The Cloud will now do it's Setup Dance and walk through some colors. Then, if all things are working, you should see it start to count up to the current AQI from your SensorID.





-------------



-------------



# FAQs



## Update Interval

The AirCloud updates every 10 minutes.

You will see the Data light turn purple, then the cloud will go black, and then it will count up 1 LED per AQI PM2.5 value. So if the PM2.5 is 50, 50 lights will light up. If AQI is 500 -- the full cloud will light up. There are 183 LEDs in the cloud -- so if the AQI is > 183 -- it will count up to 183 and then change colors to the final value. If the AQI is > 183 you are already in trouble.




## What it looks like (Explain the different colors)

### 	High Level:



### 	Setup Dance



### 	Status Lights (left hand side)



### 	Counting up



### 	Breathing



### Different colors of the cloud

AQI  color mapping.



### Rainbow Cloud / Moving Rainbow Cloud

<GIF>

1. If you see a flashing rainbow cloud it means there is an error! If it goes away after a few seconds or a couple of minutes -- it probably means the server was down and has come back online. You can email me and I can try to remotely debug.



## I want to change the location of my AirCloud / AirCloud not turning on?!

1. Plug in your AirCloud

2. Hit the reset button ONCE. The reset button is on the back. If it still does not power on, hit the reset button again after waiting 10 seconds.

<Photo>

<< ideo of turning it on>


Are you changing locations and going to need to connect to a new Wifi network? If so, read about how to Reset your AirCloud.



## How to Reset your AirCloud / How to change Wifi Credentials / How to change PurpleAir Ids

1. Plug in your AirCloud
2. Hit the reset button multiple times -- I would say hit it every 5 seconds for 20 seconds (so 4 times).
3. Go follow the setup instructions -- starting with step XX

<Photo>

<< ideo of a reset>


You know you have succeed in resetting if the 4 status lights on the left hand side are purple.

