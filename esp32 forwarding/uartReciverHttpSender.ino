#include "stringToJson.h"
#include <WiFi.h>
#include <stdio.h>
#include <stdlib.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#define LED_WIFI_CONNECTION 2


WiFiMulti wifiMulti;
char* ssid="FASTWEB-E273A3-2.4GH";
char* pwd="1W3HEGTRGT";
char delimiter = ';';
const char* input;

//31/01/24:01:00:00;35.00;0;0;0;+11.112597;+46.137386;256.5;+11.112597;+46.137386;256.5;+11.112597;+46.137394;256.8;+11.112597;+46.137424;257.7;+11.112599;+46.137489;255.4;+11.112265;+46.137524;251.9;+11.111943;+46.137665;248.4;+11.111838;+46.138126;251.3;+11.111697;+46.138294;248.9;+11.110552;+46.138176;246.0;+11.109485;+46.138065;243.3;+11.109016;+46.138573;241.0;+11.108971;+46.138664;240.5;+11.108417;+46.139778;239.0;+11.107315;+46.140919;236.2;+11.105661;+46.141613;233.5;+11.104668;+46.142097;231.9;+11.101726;+46.143192;228.1;+11.100146;+46.144054;226.3;+11.099698;+46.144897;225.3;+11.099638;+46.146233;223.4;+11.099705;+46.148003;220.6;+11.100215;+46.149677;218.2;+11.100906;+46.150578;217.1;+11.101125;+46.151722;215.9;+11.101864;+46.153240;214.2;+11.102911;+46.154987;213.3;+11.104322;+46.156517;214.5;+11.105819;+46.158115;215.8;+11.107552;+46.159927;218.4;+11.109124;+46.161579;218.3;+11.114491;+46.167450;217.2;+11.116472;+46.168873;218.0;+11.117986;+46.170364;217.3;+11.118130;+46.170517;217.3;+11.119495;+46.172001;218.1;+11.120573;+46.173630;218.3;+11.121778;+46.174690;218.3;+11.122038;+46.174313;223.1;+11.122561;+46.174221;229.8;+11.123334;+46.174835;238.6;+11.123719;+46.174526;243.9;+11.123410;+46.173927;247.0;+11.123236;+46.173588;244.7;+11.123038;+46.173615;243.1;+11.123014;+46.173576;245.2;+11.123007;+46.173573;245.5;+11.122999;+46.173573;246.0;+11.122993;+46.173573;246.5;+11.122995;+46.173576;246.9;+11.122998;+46.173576;247.0;+11.122999;+46.173580;246.7;+11.123000;+46.173580;246.2;+11.123000;+46.173580;245.4;+11.123001;+46.173580;244.3;+11.123001;+46.173576;242.9;+11.123004;+46.173573;242.1;+11.123008;+46.173573;241.7;+11.123012;+46.173569;241.3;+11.123016;+46.173569;241.1;+11.123019;+46.173569;240.8;+11.123021;+46.173573;240.5;+11.123019;+46.173573;240.3;+11.123017;+46.173576;239.9;+11.123013;+46.173576;239.5;+11.123010;+46.173576;239.1;+11.123007;+46.173580;238.9;+11.123007;+46.173584;238.5;+11.123009;+46.173584;238.6;+11.123011;+46.173588;239.0;+11.123013;+46.173588;239.4;+11.123014;+46.173588;239.3;+11.123013;+46.173588;239.4;+11.123011;+46.173584;239.7;+11.123011;+46.173584;239.4;+11.123011;+46.173580;238.8;+11.123012;+46.173576;238.0;+11.123013;+46.173573;237.6;+11.123014;+46.173569;237.4;+11.123015;+46.173565;237.2;+11.123015;+46.173561;237.2;+11.123014;+46.173561;237.3;+11.123013;+46.173565;237.0;+11.123013;+46.173569;236.7;+11.123014;+46.173573;236.6;+11.123015;+46.173576;236.5;+11.123016;+46.173580;236.4;+11.123017;+46.173580;236.6;+11.123017;+46.173580;236.9;+11.123016;+46.173580;237.1;+11.123016;+46.173580;237.2;+11.123016;+46.173580;237.3;+11.123015;+46.173580;237.2;+11.123013;+46.173580;237.0;+11.123011;+46.173576;236.9;+11.123007;+46.173576;237.1;+11.123003;+46.173576;237.6;+11.122999;+46.173576;238.1;+11.122997;+46.173573;238.7;+11.122999;+46.173573;238.9;+11.123000;+46.173573;238.7;+11.123002;+46.173569;238.5;+11.123005;+46.173569;238.3;+11.123009;+46.173565;238.1;+11.123012;+46.173565;237.7;+11.123013;+46.173561;237.4;+11.123015;+46.173561;237.2;+11.123016;+46.173561;236.9;+11.123017;+46.173561;236.8;+11.123017;+46.173565;236.9;+11.123017;+46.173569;237.2;+11.123020;+46.173573;237.4;+11.123022;+46.173576;237.4;+11.123021;+46.173576;237.4;+11.123020;+46.173580;237.5;+11.123018;+46.173580;237.6;+11.123016;+46.173580;237.9;+11.123015;+46.173580;237.9;+11.123016;+46.173580;237.6;+11.123019;+46.173580;237.0;+11.123019;+46.173580;237.0;+11.123020;+46.173580;237.1;+11.123020;+46.173580;237.1;+11.123018;+46.173576;236.9;+11.123018;+46.173573;236.8;+11.123016;+46.173569;236.8;+11.123016;+46.173569;236.8;+11.123016;+46.173565;236.4;+11.123016;+46.173561;236.0;+11.123015;+46.173557;235.7;+11.123014;+46.173553;235.5;+11.123013;+46.173553;235.5;+11.123010;+46.173550;235.8;+11.123008;+46.173550;236.1;+11.123007;+46.173550;236.0;+11.123009;+46.173553;236.0;+11.123011;+46.173553;236.2;+11.123012;+46.173557;236.4;+11.123013;+46.173557;236.6;+11.123013;+46.173561;236.4;+11.123013;+46.173561;236.7;+11.123013;+46.173561;237.1;+11.123012;+46.173561;237.3;+11.123012;+46.173561;237.7;+11.123012;+46.173561;238.4;+11.123011;+46.173561;238.7;+11.123011;+46.173561;238.4;+11.123009;+46.173561;237.9;+11.123009;+46.173557;237.4;+11.123012;+46.173550;238.1;+11.123012;+46.173550;238.1;+11.123011;+46.173550;238.2;+11.123011;+46.173550;238.0;+11.123011;+46.173553;237.9;+11.123012;+46.173557;237.8;+11.123013;+46.173561;237.8;+11.123016;+46.173565;237.9;+11.123018;+46.173565;238.0;+11.123021;+46.173565;238.6;+11.123022;+46.173569;239.0;+11.123023;+46.173569;239.2;+11.123024;+46.173565;239.5;+11.123023;+46.173565;239.8;+11.123023;+46.173561;240.3;+11.123021;+46.173557;239.9;+11.123018;+46.173553;239.5;+11.123014;+46.173553;239.3;+11.123013;+46.173553;239.2;+11.123013;+46.173553;239.1;+11.123013;+46.173553;239.1;+11.123012;+46.173553;239.7;+11.123010;+46.173557;240.4;+11.123010;+46.173557;240.4;+11.123010;+46.173557;239.9;+11.123010;+46.173557;239.5;+11.123007;+46.173557;239.3;+11.123004;+46.173557;239.3;+11.123003;+46.173561;239.1;+11.123004;+46.173565;238.9;+11.123005;+46.173569;238.8;+11.123006;+46.173573;238.3;+11.123007;+46.173573;238.3;+11.123008;+46.173576;238.9;+11.123007;+46.173573;240.0;+11.123007;+46.173573;240.8;+11.123008;+46.173573;241.6;+11.123008;+46.173573;242.4;+11.123009;+46.173576;243.0;+11.123010;+46.173580;243.1;+11.123012;+46.173584;242.4;+11.123013;+46.173584;241.8;+11.123013;+46.173588;241.2;+11.123014;+46.173588;240.4;+11.123016;+46.173588;239.7;+11.123017;+46.173588;239.6;+11.123019;+46.173588;239.4;+11.123021;+46.173588;239.0;+11.123019;+46.173592;238.9;+11.123016;+46.173592;239.0;";




void setup() {
  
  Serial.begin(9600);
  Serial2.begin(9600);
  delay(3000);
  // Connessione al WiFi
  wifiMulti.addAP(ssid, pwd);
  pinMode(LED_WIFI_CONNECTION, OUTPUT);
  Serial.println("Connessione in corso");
  
  

}
void loop() {
   if(wifiMulti.run() == WL_CONNECTED){
    digitalWrite(LED_WIFI_CONNECTION, HIGH);
  }else{
    digitalWrite(LED_WIFI_CONNECTION, LOW);
  }
  
  String c="";
  while(Serial2.available()>0){
    c=Serial2.readStringUntil('\0');

  }
  
  if(c.length()>50){
    input=c.c_str();
    Serial.println(input);
    sendJson();
  }
  
  
  
  
}
void sendJson(){
  if(wifiMulti.run()!= WL_CONNECTED ){
    
    Serial.println(wifiMulti.run());
    
  }
  else{
    char* formattedResult = formatInputString(input, delimiter);
    
    HTTPClient  http;

    http.begin("http://13.60.21.191:3000/api");

    http.addHeader("Content-Type", "application/json");
    int httpCode=http.POST(formattedResult);
    
    if(httpCode>0){
      Serial.println("Done.");
      free(formattedResult);
    }
    else{
      Serial.println("HTTP POST: Failed");
      Serial.println(http.errorToString(httpCode).c_str());

    }

    http.end();

  }

}

