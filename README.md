# ESP LED wakeup timer
With this code I wanted to try out if I can get up better when a light slowly gets brighter at wake up time.  
The leds start to light up some minutes before the wake-up time with a lower level, so that you are slowly woken up.  
And it helped me to get up better!

## How to use:
The ESP has no hardware clock, so I pull the current time.
Because of that we have to connect to our local wifi:
```
const char* ssid = "<wifi name>";
const char* pass = "<wifi password>";
```
A few line below the can set the default wake up time as UTC time.
```
int wakeup_hour = 4;
int wakeup_minute = 55;
```
you can also set how many minutes before the alarm the lower brightness should start.
the brightness then increases slowly.
```
int start_low_brightness_min_for_wakeup = 5;
```
If you want to use a different ntp server, feel free to change it.

## Webinterface
The wake up time can also be changed over a web interface.
To change the time you have to use the following url template:
```
http://<ip of the esp>/?hour=<wakeup hour>&min=<wakreup minute>
```

# Have Fun!
