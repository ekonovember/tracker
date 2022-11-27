#include <SoftwareSerial.h>
#include <NMEAGPS.h>
#include <SH1106Wire.h>
#include <math.h>
#include <SPI.h>
#include <SD.h>

struct GpsFormat {  
  String Latitude;
  String Longitude;
};

struct Timings {
  const unsigned long DisplayInterval = 3000;
  const unsigned long LoggingInterval = 10000;

  bool ShouldDisplay() {
    unsigned long currentMillis = millis();

    if (currentMillis - LastDisplay > DisplayInterval) {
      LastDisplay = currentMillis;
      return true;
    }

    return false;
  }

  bool ShouldLog() {
    unsigned long currentMillis = millis();

    if (currentMillis - LastLog > LoggingInterval) {
      LastLog = currentMillis;
      return true;
    }

    return false;
  }

  unsigned long LastDisplay = 0;
  unsigned long LastLog = 0;
  unsigned long LastPosition = 0;
};

SH1106Wire Display(0x3c, SDA, SCL);
NMEAGPS GPS;
SoftwareSerial SerialGPS(D3, D4);
//Timings ActionTimings = {0, 0, 0};

void setup() {
  Serial.begin(9600);
  SerialGPS.begin(9600);
  SD.begin(4);
  Display.init();
  Display.flipScreenVertically();  
  Display.clear();  
  Display.display();
}

void drawGPSscreen(GpsFormat GPSformat) {    
  Display.clear();  

  Display.setTextAlignment(TEXT_ALIGN_LEFT);
  Display.setFont(Monospaced_plain_10);

  Display.drawHorizontalLine(0, 35, 128);    
  Display.drawString(0, 40, "lat: " + GPSformat.Latitude);  
  Display.drawCircle(52, 45, 2);  
  Display.drawString(0, 50, "lng: " + GPSformat.Longitude);  
  Display.drawCircle(52, 55, 2);
  Display.display();
}

void GPSdebug(gps_fix &GPSfix) {
  if (GPSfix.valid.location) {
		Serial.println("location float");	
		Serial.printf("%f\n", GPSfix.latitude());
    Serial.printf("%f\n", GPSfix.longitude());  			    
	}

	if (GPSfix.valid.date) {
		Serial.println("date");	
		Serial.printf("%d %d %d\n", GPSfix.dateTime.year, GPSfix.dateTime.month, GPSfix.dateTime.date);
	}

	if (GPSfix.valid.time) {
		Serial.println("time");	
		Serial.printf("%d %d %d\n", GPSfix.dateTime.hours, GPSfix.dateTime.minutes, GPSfix.dateTime.seconds);
	}	

	if (GPSfix.valid.speed) {
		Serial.println("speed");	
		Serial.printf("%f KT\n", GPSfix.speed());    
	}

	if (GPSfix.valid.heading) {
		Serial.println("heading");	
		Serial.printf("%f DEG\n", GPSfix.heading());
    Serial.printf("%d DEG\n", GPSfix.heading_cd());    
	}
}

void logToSDcard(gps_fix &GPSfix) {
  File file = SD.open("log.csv");

  if (file && GPSfix.valid.date && GPSfix.valid.location) {    
    file.printf("%f,%f\n", GPSfix.latitude(), GPSfix.longitude());
    file.close();
  }
}

void drawLoadingScreen() {  
  Display.clear();

  unsigned long ms = millis();
  int positionOffset = 10;

  if (ms % 3000 == 0) positionOffset = 30;
  if (ms % 2000 == 0) positionOffset = 20;

  Display.fillCircle(positionOffset, 30, 6);

  Display.display();  
  delay(500);
}

GpsFormat formatFix(gps_fix &GPSfix) {  
  String latString = " --  --.--' -";
  String lngString = "---  --.--' -"; 
  
  if (GPSfix.valid.location) {
    char latitudeBuff[40];
    char longitudeBuff[40];  

    float latitudeFloat  = GPSfix.latitude();
    float longitudeFloat = GPSfix.longitude();

    char latHem = 'N';
    char lngHem = 'E';

    if (latitudeFloat  < 0) latHem = 'S';
    if (longitudeFloat < 0) lngHem = 'W';

    int latDegrees = abs(latitudeFloat);
    int lngDegrees = abs(longitudeFloat);

    float latMinutesFloat = (abs(latitudeFloat)  - latDegrees) * 60;
    float lngMinutesFloat = (abs(longitudeFloat) - lngDegrees) * 60;

    int latMinutes = (int)latMinutesFloat;
    int lngMinutes = (int)lngMinutesFloat;        

    int latMinutesDecimal =  (int)ceill((latMinutesFloat - latMinutes) * 100);
    int lngMinutesDecimal =  (int)ceill((lngMinutesFloat - lngMinutes) * 100);    

    sprintf(latitudeBuff, " %02d  %02d.%02d' %c", latDegrees, latMinutes, latMinutesDecimal, latHem);
    sprintf(longitudeBuff, "%03d  %02d.%02d' %c", lngDegrees, lngMinutes, lngMinutesDecimal, lngHem);

    latString = latitudeBuff;
    lngString = longitudeBuff;    
  }

  GpsFormat result = {    
    latString,
    lngString    
  };

  return result;
}

void GPSloop()
{
	while (GPS.available(SerialGPS)) {		
    gps_fix GPSfix = GPS.read();
    
    //if (ActionTimings.ShouldDisplay()) {
      GpsFormat GPSformat = formatFix(GPSfix);
      drawGPSscreen(GPSformat);		
    //}    
    
    //if (ActionTimings.ShouldLog()) {
      //logToSDcard(GPSfix);
    //}

    GPSdebug(GPSfix);		        
	}      
}

void loop() {    
  GPSloop();  
}