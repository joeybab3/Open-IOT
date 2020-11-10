///////////////////
// Open IOT      //
///////////////////
// Models        //
///////////////////
// Bird Zero     //
// Okai ES100    //
// Okai ES200B   //
///////////////////
// https://joeybabcock.me/blog/electric_scooters/open-iot/
///////////////////

#include <Arduino.h>
#include <FastCRC.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <strings_en.h>
#include <WiFiManager.h>

#define HEARTBEAT_INTERVAL 500
#define PACKET_SIZE 5

FastCRC8 CRC8;
ESP8266WebServer server(80);
WiFiManager wm;

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

uint32_t crc;
int loopCount = 0;
int model = 0;
const int enablePin = 15;

bool isRunning = false;
bool headlightStatus = false;
bool flashStatus = false;
bool locked = false;
bool isMetric = false;
bool debug = false;
bool isBit4 = false; // "Fast Acceleration"
bool isBit7 = false;
bool isBit8 = false;

const char* ssid = "vehicle";
const char* password = "SPINB0001";

// The packet structure, 00 is a placeholder for the command, a speed variable, and the Checksum
uint8_t okaiPacket[] = {0xA6, 0x12, 0x02, 0x00, 0x00, 0x00};

byte okaiMaxSpeed      = 0xFF;
byte okaiDefaultConfig = 0xE1;

int chksm = 0;

void reCalculateCommand()
{
    crc = CRC8.maxim(okaiPacket, PACKET_SIZE);
    okaiPacket[5] = crc;
}

void sendCommand()
{
    if(debug)
    {
        Serial.print(okaiPacket[0], HEX);
        Serial.print(" ");
        Serial.print(okaiPacket[1], HEX);
        Serial.print(" ");
        Serial.print(okaiPacket[2], HEX);
        Serial.print(" ");
        Serial.print(okaiPacket[3], HEX);
        Serial.print(" ");
        Serial.print(okaiPacket[4], HEX);
        Serial.print(" ");
        Serial.print(okaiPacket[5], HEX);
        Serial.println();
    }
    else
    {
        Serial.write(okaiPacket, sizeof(okaiPacket));
    }
}

void restartEsp()
{
    server.send(200, "text/html", SendRestart());
    delay(500);
    ESP.restart();
}

void setup()
{
    Serial.begin(115200);
    delay(10);
    
    WiFi.mode(WIFI_AP_STA);

    wm.setConfigPortalBlocking(false);

    if(wm.autoConnect(ssid,password)){
        Serial.println("Connected");
    }
    else {
        Serial.println("No");
    }
    
    delay(100);
    
    pinMode(enablePin, OUTPUT);
    
    MDNS.begin("joeybabcock.me");
    MDNS.addService("http", "tcp", 80);
    
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_FS
        type = "filesystem";
      }
  
      // NOTE: if updating FS this would be the place to unmount FS using FS.end()
      Serial.println("Start updating " + type);
    });
    
    ArduinoOTA.onEnd([]() {
        //Serial.println("\nEnd");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        //Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
           //Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            //Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            //Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            //Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            //Serial.println("End Failed");
        }
    });
    
    //ArduinoOTA.setPassword("8266!");
    ArduinoOTA.begin();

    server.on("/", handle_OnConnect);
    server.onNotFound(handle_NotFound);
    server.on("/default", setDefaultConfig);
    server.on("/power", power);
    server.on("/headlight", headlight);
    server.on("/headlightOn", enableHeadlight);
    server.on("/headlightOff", disableHeadlight);
    server.on("/flash", flash);
    server.on("/units", setUnitSystem);
    server.on("/restart", restartEsp);
    server.on("/ping", ping);
    server.on("/toggle4", toggleBit4);
    server.on("/toggle7", toggleBit7);
    server.on("/toggle8", toggleBit8);

    server.begin();
    setDefaultConfig(); 
    Serial.begin(9600);
    pinMode(enablePin, OUTPUT);
    digitalWrite(enablePin, HIGH);
}

void loop()
{
    if(loopCount > 1000)
    {
        restartEsp();
    }
    else if(loopCount < 5 && WiFi.status() != WL_CONNECTED) {
        wm.process();
        delay(1000);
    }
    else
    {
      MDNS.update();
      ArduinoOTA.handle();
      server.handleClient();
      sendCommand();
      yield();
      delay(HEARTBEAT_INTERVAL);
    }
    loopCount++;
}

void handle_OnConnect() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", SendHTML()); 
}

void handle_NotFound(){
    server.sendHeader("Location", "/", true); //Redirect to our html web page 
    server.send(302, "text/plane","");
}


void handle_okai() {
  Serial.begin(9600);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);
  enableFastAcceleration();
  server.send(200, "text/html", SendHTML());
}

void goHome()
{
    server.sendHeader("Location", "/", true); //Redirect to our html web page 
    server.send(302, "text/plane","");
}

String SendHTML(){
  String page = "<!DOCTYPE html> <html>\n";
  page +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  page +="<title>Scooter Control</title>\n";
  page +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  page +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}#status {font-size: 36px;}\n";
  page += ".button {display: block;width: 120px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;";
  page += "text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  page += ".button2 {display: inline;width: 100px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;";
  page += "text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  page +=".button-on {background-color: #1abc9c;}\n";
  page +=".button-on:active {background-color: #16a085;}\n";
  page +=".button-off {background-color: #34495e;}\n";
  page +=".button-off:active {background-color: #2c3e50;}\n";
  page +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  page +="</style>\n";
  page +="</head>\n";
  page +="<body>\n";
  page +="<h1>Scooter Config</h1>\n";
  page +="<a class=\"button ";
  if(!isRunning){page += "button-off";}
  page += "\" href=\"/power\">POWER</a>\n";
  page +="<a class=\"button ";
  if(!headlightStatus){page += "button-off";}
  page += "\" href=\"/headlight\">HEADLIGHT</a>\n";
  page += "<a class=\"button ";
  if(!flashStatus){page += "button-off";}
  page += "\" href=\"/flash\">LIGHT FLASH</a>\n";
  page += "<a class=\"button button-off\" href=\"/units\">";
  if(!isMetric)
  {page += "SET KMH";}
  else
  {page += "SET MPH";}
  page += "</a>\n";
  page +="<a class=\"button button-off\" href=\"/restart\">RESTART</a>\n";
  page += "<span id='status'>&bull; 0x"+String(okaiPacket[3], HEX)+" </span>\n";
  page +="<small style='vertical-align: super;'>"+WiFi.localIP().toString()+" | "+WiFi.softAPIP().toString()+"</small>\n";
  page +="</body>\n";
  page +="</html>\n";
  return page;
}

String SendRestart(){
  String page = "<!DOCTYPE html> <html>\n";
  page +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  page +="<title>Restarting...</title>\n";
  page +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  page +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}#status {font-size: 36px;}\n";
  page +="</style>\n";
  page +="</head>\n";
  page +="<body>\n";
  page +="<h1>Restarting...</h1>\n";
  page +="<h1>Reloading in <span id='timer'>10</span> seconds</h1>\n";
  page += "<span id='status'>&bull;</span>\n";
  page +="<small style='vertical-align: super;'>"+WiFi.localIP().toString()+" | "+WiFi.softAPIP().toString()+"</small>\n";
  page += "<script>\n";
  page += "var numFailures = 0;\n";
  page +="function checkStatus()\n";
  page +="{\n";
  page +="var xhr = new XMLHttpRequest();\n";
  page +="xhr.open('GET', '/ping', true);\n";
  page +="xhr.timeout = 500; // time in milliseconds\n";
  page +="xhr.ontimeout = function (e) {\n";
  page +="  document.getElementById('status').style.color = 'red';\n";
  page +="};\n";
  page +="xhr.onload = function () {\n";
  page +="    if (xhr.readyState === xhr.DONE) {\n";
  page +="        if (xhr.status === 200) {\n";
  page +="            document.getElementById('status').style.color = 'green';\n";
  page += "           window.location.href = '/';\n";
  page +="        }\n";
  page +="        else\n";
  page +="        {\n";
  page +="            document.getElementById('status').style.color = 'red';\n";
  page +="        }\n";
  page +="    }\n";
  page +="    else\n";
  page +="    {\n";
  page +="        document.getElementById('status').style.color = 'red';\n";
  page +="    }\n";
  page +="};\n";
  page +="    xhr.send();\n";
  page +="}\n";
  page += "\tvar seconds = 10;\n";
  page += "\tfunction asecondhaspassed()\n";
  page += "\t{\n";
  page += "\t\tif(seconds < 1)\n";
  page += "\t\t{";
  page += "\t\t\twindow.location.href = '/';\n";
  page += "\t\t}\n";
  page += "\t\telse\n";
  page += "\t\t{\n";
  page += "\t\t\tseconds--;\n";
  page += "\t\t\tdocument.getElementById('timer').innerHTML = seconds;\n";
  page += "\t\tcheckStatus();\n";
  page += "\t\t}\n";
  page += "\t}\n";
  page += "\tvar timer = setInterval(asecondhaspassed, 1000);\n";
  page += "</script>\n";
  page +="</body>\n";
  
  page +="</html>\n";
  return page;
}
