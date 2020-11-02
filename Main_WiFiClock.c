//###########################################################################################################""
//###                                                                                                    ####""
//###          CUBE CLOCK WIREFRAME + WiFi + RGB LED ...                                                 ####""
//###          1.0 - 2020 ROMAIN DUROCHER                                                                ####""
//###                                                                                                    ####""
//###########################################################################################################""

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Arduino_JSON.h>
#include <WifiUDP.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

const char* ssid     = "xxxxx";	// Type your network name here
const char* password = "xxxxx"; // Type your network password here

// Define NTP properties
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "ca.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)

// Set up the NTP UDP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

String TimeNow;
String DateNow;
String Temperature;
String Weather;

// RGB LED Pins
#define Blue  15
#define Green 13
#define Red   12

const int motionSensor = 14;

const int numReadings = 100;		//nombre de mesure dans la moyenne du capteur de luminosité
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 1000;                  // the running total - initialisée a une grosse valeur pour ne pas que l'affichage reste bloqué dans le noir le temps que la moyenne se fasse
int average = 0;                // the average
int lux = 0;
float dimValue = 0;							// 0 to 1 range
int threshold = 8;
int interruptHandler = 0;
bool ledOn = 0;
int ledUp = 1;  
int i = 0;

Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

//RGB Led 0 = full Bright, 1023 = off
int RedTemp = 1023;
int GreenTemp = 1023;
int BlueTemp = 1023;

int val;
int WeatherRefreshCount;

int Dim14SegDim = 0xE0;

int anim1[] = {0x0001,0x0020,0x0010,0x0008};

int anim2[] = {0x0008,0x0004,0x0002};

String openWeatherMapApiKey = "xxxx";  // Type here your API key
String _lat = "xxx"; // get lat & long values from https://www.latlong.net/
String _long = "xxx";

unsigned long lastConnectionTime = 0;     // last time you connected to the server, in milliseconds
const unsigned long intervalTime = 5 * 1000;    
unsigned long lastConnectionWeather = 0;     // last time you connected to the server, in milliseconds
const unsigned long intervalWeather = 30 * 60 * 1000;    
unsigned long lastConnectionTemperature = 0;     // last time you connected to the server, in milliseconds
const unsigned long intervalTemperature = 10 * 60 * 1000;    
unsigned long interruptCounter = 0;     // last time you connected to the server, in milliseconds
const unsigned long interruptInterval = 5 * 1000;     
unsigned long timingLED = 0;     // last time you connected to the server, in milliseconds
const unsigned long intervalLED = 50;     

unsigned long timeCounter = 0;     // last time you connected to the server, in milliseconds
const unsigned long Interval20s = 20 * 1000;    
const unsigned long Interval25s = 25 * 1000;    
const unsigned long Interval30s = 30 * 1000;  

static int ledValueGreen[61] = {		// table of value to make the led glow from dark to teal
  1006, 1006, 1005, 1003, 1001, 998, 994, 990, 985, 979, 
  973, 966, 959, 951, 943, 934, 924, 914, 904, 894, 882, 
  871, 859, 848, 835, 823, 810, 798, 785, 772, 759, 746, 
  733, 720, 708, 695, 683, 670, 659, 647, 635, 624, 614, 
  604, 594, 584, 575, 567, 559, 552, 545, 539, 533, 528, 
  524, 520, 517, 515, 513, 512, 512
};

static int ledValueBlue[61] = {		// table of value to make the led glow from dark to teal
  1018, 1018, 1018, 1017, 1016, 1015, 1014, 1013, 1012, 
  1010, 1008, 1006, 1004, 1002, 999, 996, 994, 991, 987, 
  984, 981, 978, 974, 971, 967, 963, 959, 956, 952, 948, 
  944, 940, 936, 932, 929, 925, 921, 917, 914, 910, 907, 
  904, 901, 897, 894, 892, 889, 886, 884, 882, 880, 878, 
  	876, 875, 874, 873, 872, 871, 870, 870, 870
};  

//####################################################################################################################################
//##################### INTERRUPTIONS ################################################################################################
//####################################################################################################################################

ICACHE_RAM_ATTR void detectsMovement() {
  interruptHandler = interruptHandler + 1;
  interruptCounter = millis();
}

//####################################################################################################################################

void setup() {
  pinMode(Blue, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(Red, OUTPUT);

  pinMode(motionSensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, FALLING);

  analogWrite(Red, RedTemp);
  analogWrite(Green, GreenTemp);
  analogWrite(Blue, BlueTemp);

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
 		readings[thisReading] = 0;	
  }

  alpha4.begin(0x70);

  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    	animation1();
      	if (millis() > start + 15000) {	// if no connexion after 15s			      
		whoops();
	}
	  
  }

  alpha4.clear();
  alpha4.writeDisplay();

  TimeNow = GetTime();
  Temperature = getTemperature();
  Weather = getWeather();
}

void loop() {
	
  HowBrightIsIt();

	if (millis() - lastConnectionTime > intervalTime) {
    lastConnectionTime = millis();    
    TimeNow = GetTime();
  }

  if (millis() - lastConnectionTemperature > intervalTemperature){
  	lastConnectionTemperature = millis();    
    	Temperature = getTemperature();
	  
	if(WiFi.status() != WL_CONNECTED){ // check every 5s (intervalTemperature) if wifi is still connected
    		whoops();
	}
  }

  if (millis() - lastConnectionWeather > intervalWeather){
  	lastConnectionWeather = millis();    
    Weather = getWeather();
  }

  if (millis() - timeCounter <= Interval20s){
  	PrintTime(TimeNow);
  }
  else if(millis() - timeCounter > Interval20s && millis() - timeCounter <= Interval25s){
  	PrintTemp(Temperature);
  }
  else if(millis() - timeCounter > Interval25s && millis() - timeCounter <= Interval30s &&  Weather != ""){ // Display Weather only if different from sun
  	PrintForecast(Weather);
  }
  else {
    timeCounter = millis();
  }

  if (interruptHandler != 0){
  	interruptVoid();
  }

  if (ledOn == 1){
  	ledVoid();
  }	 
}

//####################################################################################################################################
void animation1(){

	for(int i = 3; i >= 1; i--){
		alpha4.writeDigitRaw(0, 0);
		alpha4.writeDigitRaw(1, 0);
		alpha4.writeDigitRaw(2, 0);
		alpha4.writeDigitRaw(3, 0);
		alpha4.writeDigitRaw(i, 0x0001);
		alpha4.writeDisplay();

	  delay(45);
	}

  for (int i = 0 ; i <= 3 ; i++){
		 
		alpha4.writeDigitRaw(1, 0);
		alpha4.writeDigitRaw(0, anim1[i]);
		alpha4.writeDisplay();
    
    delay(45);
  } 

	for(int i = 1; i <= 2; i++){
		alpha4.writeDigitRaw(0, 0);
		alpha4.writeDigitRaw(1, 0);
		alpha4.writeDigitRaw(2, 0);
		alpha4.writeDigitRaw(3, 0);
		alpha4.writeDigitRaw(i, 0x0008);
		alpha4.writeDisplay();

	  delay(45);
	}
  
  for (int i = 0 ; i <= 2 ; i++){		 
		alpha4.writeDigitRaw(2, 0);
		alpha4.writeDigitRaw(3, anim2[i]);
	  alpha4.writeDisplay();
    
    delay(45);
  }
}

//####################################################################################################################################
void animation(int animChoice){
	// to display a smiley on the 14 seg display
	switch (animChoice){
		case 1:
			alpha4.writeDigitRaw(0, 0x00DD);
			alpha4.writeDigitRaw(1, 0x0000);
			alpha4.writeDigitRaw(2, 0x0000);
			alpha4.writeDigitRaw(3, 0x00DD);
			alpha4.writeDisplay();
			break;
		case 2:
			alpha4.writeDigitRaw(0, 0x001D);
			alpha4.writeDigitRaw(1, 0x0000);
			alpha4.writeDigitRaw(2, 0x0000);
			alpha4.writeDigitRaw(3, 0x001D);
			alpha4.writeDisplay();		
			break;
		case 3:
			alpha4.writeDigitRaw(0, 0x00EB);
			alpha4.writeDigitRaw(1, 0x0000);
			alpha4.writeDigitRaw(2, 0x0000);
			alpha4.writeDigitRaw(3, 0x00EB);
			alpha4.writeDisplay();
			break;
		case 4:
			alpha4.writeDigitRaw(0, 0x003E);
			alpha4.writeDigitRaw(1, 0x0000);
			alpha4.writeDigitRaw(2, 0x0000);
			alpha4.writeDigitRaw(3, 0x003E);
			alpha4.writeDisplay();
			break;
		default:
			alpha4.writeDigitRaw(0, 0x003F);
			alpha4.writeDigitRaw(1, 0x0000);
			alpha4.writeDigitRaw(2, 0x0000);
			alpha4.writeDigitRaw(3, 0x003F);
			alpha4.writeDisplay();
			break;
	}	
}
//####################################################################################################################################
void ledVoid(){
	// to make the RGB led glow teal, with a 2s period to simulate a calm breathing
	if (millis() - timingLED > intervalLED) {
    timingLED = millis();
    
    if (ledUp == 1){
      GreenTemp = ledValueGreen[i];
      BlueTemp = ledValueBlue[i];
      i = i + 1;
    }
    else if (ledUp == 0){
      GreenTemp = ledValueGreen[i];
      BlueTemp = ledValueBlue[i];
      i = i - 1;    
    }
  }

  if(i == 0){
    ledUp = 1;
  }

  if(i == 60){		// variable depend of the number of entry in ledValuexx table
    ledUp = 0;  
  }

  analogWrite(Red, RedTemp);
  analogWrite(Green, GreenTemp);
  analogWrite(Blue, BlueTemp);

	return;
}

//####################################################################################################################################
String getTemperature(){

	int tempTEMP;
	String tempExport= "";
  	String jsonBuffer; // not stable when global, not sure to understand why ..

	String serverPath = "http://api.openweathermap.org/data/2.5/onecall?lat=" + _lat + "&lon=" + _long + "&exclude=minutely,hourly,daily,alert&appid=" + _apiKey + "&units=metric";
	
	jsonBuffer = httpGETRequest(serverPath.c_str());
	JSONVar myObject = JSON.parse(jsonBuffer);

	// JSON.typeof(jsonVar) can be used to get the type of the var
	if (JSON.typeof(myObject) == "undefined") {
	  	Serial.println("ERROR");
		//Serial.println(ret);
        	delay(1000);
	  	return "azerty";
	}

  	tempTEMP = round(myObject["current"]["temp"]);

	if(tempTEMP <= -10 ){
    		tempExport = String(tempTEMP);
	}
	else if( tempTEMP > -10 && tempTEMP < 0 ){
		tempExport += " ";	// fill variable if not enough digit
		tempExport += String(tempTEMP);
	}
	else if(tempTEMP >= 0 && tempTEMP < 10 ){
		tempExport += " ";	// fill variable if not enough digit
		tempExport += " ";	// fill variable if not enough digit
		tempExport += String(tempTEMP);
	}
	else{
		tempExport += " ";	// fill variable if not enough digit
		tempExport += String(tempTEMP);
	}

	return tempExport;  
}

//####################################################################################################################################
String getWeather(){
	String tempWEATHER;
  	String jsonBuffer; // not stable when global, not sure to understand why ..
  	int temp = 0;
	
	String serverPath = "http://api.openweathermap.org/data/2.5/onecall?lat=" + _lat + "&lon=" + _long + "&exclude=current,minutely,daily,alert&appid=" + _apiKey + "&units=metric";
  
 	jsonBuffer = httpGETRequest(serverPath.c_str());
	// because the answer is wayyy to long on a terminal, I just want the current/next hour forecast
  	temp = jsonBuffer.indexOf("}]");
  	jsonBuffer = jsonBuffer.substring(0,temp+2);  
  	jsonBuffer = jsonBuffer + "}]}";
	
	JSONVar myObject = JSON.parse(jsonBuffer);

	// JSON.typeof(jsonVar) can be used to get the type of the var
	if (JSON.typeof(myObject) == "undefined") {
	  	//Serial.println("ERROR");
        	delay(1000);
	  	return "azerty";
	}

  	tempWEATHER = myObject["hourly"][0]["weather"][0]["icon"];
	// see OpenWeather documentation for code details
  	if ((tempWEATHER == "09d") || (tempWEATHER == "09n") || (tempWEATHER == "10d") || (tempWEATHER == "10n") || (tempWEATHER == "11d") || (tempWEATHER == "11n")){
  		tempWEATHER = "RAIN";
  	}
  	else if((tempWEATHER == "13d") || (tempWEATHER == "13n")){
  		tempWEATHER = "SNOW";
  	}
  	else if((tempWEATHER == "50d") || (tempWEATHER == "50n")){
  		tempWEATHER = "MIST";
  	}
	else{
		tempWEATHER ="";
	}

	return tempWEATHER;  
}

//####################################################################################################################################
String GetTime(){
	String actualtime;

	actualtime = "";

	timeClient.update();
  unsigned long epochTime =  timeClient.getEpochTime();

  // convert received time stamp to time_t object
  time_t local, utc;
  utc = epochTime;

  // Then convert the UTC UNIX timestamp to local time
  TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -300};  //UTC - 5 hours - change this as needed
  TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -360};   //UTC - 6 hours - change this as needed
  Timezone usEastern(usEDT, usEST);
  local = usEastern.toLocal(utc);

  if(hour(local) < 10){ // add a zero if minute is under 10
  	actualtime += "0";
  } 
  actualtime += hour(local);
  actualtime += ":";	// I realise this add is useless ...
  if(minute(local) < 10){ // add a zero if minute is under 10
  	actualtime += "0";
  } 
  actualtime += minute(local);

  return(actualtime);
}

//####################################################################################################################################
String GetDate(){
	String date;

	date = "";

	timeClient.update();
  unsigned long epochTime =  timeClient.getEpochTime();

  // convert received time stamp to time_t object
  time_t local, utc;
  utc = epochTime;

  // Then convert the UTC UNIX timestamp to local time
  TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -300};  //UTC - 5 hours - change this as needed
  TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -360};   //UTC - 6 hours - change this as needed
  Timezone usEastern(usEDT, usEST);
  local = usEastern.toLocal(utc);

  if(day(local) < 10){
  	date += "0";
  }
  date += day(local);
  date += " ";
  if(month(local) < 10){
  	date += "0";
  }
  date += month(local);

  return(date);
}

//####################################################################################################################################
void PrintTime(String actualtimePrint){

	alpha4.writeDigitAscii(0, actualtimePrint[0]);
	alpha4.writeDigitAscii(1, actualtimePrint[1]);
	alpha4.writeDigitAscii(2, actualtimePrint[3]);
	alpha4.writeDigitAscii(3, actualtimePrint[4]);
	alpha4.writeDisplay();	
}

//####################################################################################################################################
void PrintTemp(String TempPrint){

	alpha4.writeDigitAscii(0, TempPrint[0]);
	alpha4.writeDigitAscii(1, TempPrint[1]);
	alpha4.writeDigitAscii(2, TempPrint[2]);
	alpha4.writeDigitRaw(3, 0x00E3);
	alpha4.writeDisplay();	
}

//####################################################################################################################################
void PrintForecast(String ForecastPrint){

	alpha4.writeDigitAscii(0, ForecastPrint[0]);
	alpha4.writeDigitAscii(1, ForecastPrint[1]);
	alpha4.writeDigitAscii(2, ForecastPrint[2]);
	alpha4.writeDigitAscii(3, ForecastPrint[3]);
	alpha4.writeDisplay();	
}

//####################################################################################################################################
void HowBrightIsIt(){

	int temp = 0xE0;

	total = total - readings[readIndex];
  readings[readIndex] = analogRead(A0);
  total = total + readings[readIndex];

  readIndex = readIndex + 1;

  if (readIndex >= numReadings) {
    readIndex = 0;
  }

  average = total / numReadings;

  lux = average * 2;
  
  Serial.print("Lux : ");
  Serial.println(lux);

  if (lux  < threshold ){
  	dimValue = 0;
  	threshold = 14;		// to make an hysteresis
	  alpha4.clear();
	  alpha4.writeDisplay();

	  while( lux < threshold ){	  

	  	delay(1000);

			total = total - readings[readIndex];
		  readings[readIndex] = analogRead(A0);
		  total = total + readings[readIndex];
		  readIndex = readIndex + 1;

		  if (readIndex >= numReadings) {
		    readIndex = 0;
		  }

		  average = total / numReadings;
		  delay(10);    

		  lux = average;
	  }

  	return;
  }
  else{
  	dimValue = ((float)lux) / 500; // convert into %
  	threshold = 12; 	// to make an hysteresis
  }

  if( dimValue  > 1){
  	dimValue = 1;	// because dim should be between 0 and 1.
  }

  temp = temp + int(dimValue * 16) - 1;

	Wire.beginTransmission(0x70); // transmit to device 
	Wire.write(temp);        // sends bytes
	Wire.endTransmission();    // stop transmitting

  delay(10);    

}

//####################################################################################################################################
String httpGETRequest(const char* serverName) {
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
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

//####################################################################################################################################
void interruptVoid(){	
	int tempAnimation = 0;
	DateNow = GetDate();
	tempAnimation = random(4);

	while (interruptHandler > 0){
		
		delay(20);
  	
		if (millis() - interruptCounter >= interruptInterval){
	  	delay(200);
			interruptHandler = 0;	  	
  	}

    switch (interruptHandler) {
      case 1:
  			PrintTime(DateNow);
        break;
      case 2:
        PrintTemp(Temperature);
        break;
      case 3:
      	if (Weather != ""){	// Display Weather only if different from sun
        	PrintForecast(Weather);
      	}
      	else{			      		
			    alpha4.writeDigitAscii(0, 'S');
			    alpha4.writeDigitAscii(1, 'U');
			    alpha4.writeDigitAscii(2, 'N');
			    alpha4.writeDigitAscii(3, ' ');
					alpha4.writeDisplay();
      	}
        break;
      case 4:			      	
        animation(tempAnimation);
        break;
      case 5:        

				RedTemp = 1023;
				GreenTemp = 983;
				BlueTemp = 1013;					
			  analogWrite(Red, RedTemp);
			  analogWrite(Green, GreenTemp);
			  analogWrite(Blue, BlueTemp);

        ledOn = !ledOn;
        if (ledOn == 1){
        	ledVoid();
        }
        else{
					RedTemp = 1023;
					BlueTemp = 1023;
					GreenTemp = 1023;					
				  analogWrite(Red, RedTemp);
				  analogWrite(Green, GreenTemp);
				  analogWrite(Blue, BlueTemp);
        }
      	interruptHandler = 0;

        break;
      case 6:
        interruptHandler = 0;
        break;
      default:
        interruptHandler = 0;      	
        break;
    }
	}
}

//####################################################################################################################################
void whoops(){

  alpha4.clear();
  alpha4.writeDisplay();

  while(1){
          
    alpha4.writeDigitAscii(1, 'N');
    alpha4.writeDigitAscii(2, 'C');
    alpha4.writeDisplay();  

  }
}
