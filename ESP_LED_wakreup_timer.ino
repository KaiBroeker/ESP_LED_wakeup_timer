#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>

//wifi settings
const char* ssid = "<wifi name";
const char* pass = "<wifi password";


// wake up time settings
// UTC time
int wakeup_hour = 4;
int wakeup_minute = 55;
int start_low_brightness_min_for_wakeup = 5;
int led_on = 0;

// LED Strip infromations:
#define NUM_LEDS 33
#define DATA_PIN 3
#define CLOCK_PIN 2

// NTP Server name
const char* ntpServerName = "time.nist.gov";


CRGB leds[NUM_LEDS];
unsigned int localPort = 2390; 
IPAddress timeServerIP; // time.nist.gov NTP server address
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; 
WiFiUDP udp;
ESP8266WebServer server(80);

void updateColor(uint8_t r,uint8_t g,uint8_t b){
  for(uint8_t i = 0 ; i < NUM_LEDS; i++ ){
    leds[i].setRGB(r,g,b);
  }
}

void setup() { 
   FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);

   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid, pass);

   while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  udp.begin(localPort);

  //disable leds (0 = black)
  updateColor(0,0,0);
  FastLED.show();

  server.on("/", handleBody); //Associate the handler function to the path
  server.begin(); //Start the server

  //get ntp server ip
  WiFi.hostByName(ntpServerName, timeServerIP);
  
  delay(2000);
}

void loop() {   
  server.handleClient();//handle http requests
  if (led_on == 1){
        updateColor(255,128,0);
        FastLED.setBrightness(  100 );
        FastLED.show();
  }else{
    //send request for current time
    sendNTPpacket(timeServerIP);
    int cb = udp.parsePacket();

    if (cb) {
      udp.read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      const unsigned long seventyYears = 2208988800UL;
      unsigned long epoch = secsSince1900 - seventyYears;
  
      // checks the time and change the lightning
      // utc time
      if ((epoch  % 86400L) / 3600 >= wakeup_hour && (epoch  % 86400L) / 3600 <= (wakeup_hour +1) ) {
        // would the lower bightness start in the previous hour, we set the start to hour begin.
        if ((wakeup_minute - start_low_brightness_min_for_wakeup) <0){
          int tmp = wakeup_minute - start_low_brightness_min_for_wakeup;
          start_low_brightness_min_for_wakeup = start_low_brightness_min_for_wakeup - tmp;
        }
        
        //set the color and brightness if its wake up time :)
        if ((epoch  % 3600) / 60 >= (wakeup_minute - start_low_brightness_min_for_wakeup)) {
          start_low_brightness_min_for_wakeup -= 1;
          updateColor(255, 128, 0);
          if (start_low_brightness_min_for_wakeup >0){
            FastLED.setBrightness(  100 / (start_low_brightness_min_for_wakeup + 1) );
          }else{
            FastLED.setBrightness(  100 );
          }
        }
        FastLED.show();
      }
    }
  }
  delay(1000);
}



// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

//http body
void handleBody() { //Handler for the body path

      if (server.hasArg("hour")== true){
        wakeup_hour = server.arg("hour").toInt();
      }
      if (server.hasArg("min")== true){
        wakeup_minute = server.arg("min").toInt();
      }      
      if (server.hasArg("on")== true){
        led_on = server.arg("on").toInt();
      }
 
      String message = "wake : ";
             message += String(wakeup_hour) + ":";
             message += String(wakeup_minute) + "\n";
             message += "LED always on:  " + String(led_on);
             message += "\n";
             
 
      server.send(200, "text/plain", message);
}
