#include <SoftwareSerial.h>
#include <NMEAGPS.h>
#include <Wire.h>
#include <SH1106Wire.h>
#include <math.h>
#include <SPI.h>
#include <SD.h>
#include <LinkedList.h>

bool VALID_FIX_ONLY_WITH_HEADING = false;
bool ENABLE_SERIAL_DEBUGGING     = true;

struct GpsFormat {  
  String Latitude;
  String Longitude;
};

class Timings {
  const unsigned long DisplayInterval    =  2 * 1000;
  const unsigned long LoggingInterval    =  5 * 1000;
  const unsigned long CardUpdateInterval = 60 * 1000;

  unsigned long LastDisplay = 0;
  unsigned long LastLog = 0;
  unsigned long LastPosition = 0;
  unsigned long LastCardUpdate = 0;

public:
  Timings() { }

  bool ShouldDisplay() {
    unsigned long currentMillis = millis();

    if ((currentMillis - LastDisplay) > DisplayInterval) {
      LastDisplay = currentMillis;
      return true;
    }

    return false;
  }

  bool ShouldLog() {
    unsigned long currentMillis = millis();        

    if ((currentMillis - LastLog) > LoggingInterval) {
      LastLog = currentMillis;
      return true;
    }

    return false;
  }

  bool ShouldUpdateCard() {
    unsigned long currentMillis = millis();

    if ((currentMillis - LastCardUpdate) > CardUpdateInterval) {
      LastCardUpdate = currentMillis;
      return true;
    }

    return false;
  }  
};

void DEBUG(String message) {
  if (!ENABLE_SERIAL_DEBUGGING) { return; }

  Serial.println(message);	
}

SH1106Wire Display(0x3c, SDA, SCL);
NMEAGPS GPS;
SoftwareSerial SerialGPS(D3, D4);
Timings ActionTimings = Timings();
LinkedList<gps_fix*> GPSDataList = LinkedList<gps_fix*>();

void setup() {
  if (ENABLE_SERIAL_DEBUGGING) {
    Serial.begin(9600);
    while(!Serial) delay(2000);
    DEBUG("Serial working");
  }  

  while(!Display.init()) delay(2000);
  DEBUG("Display working");

  SerialGPS.begin(9600);  
  while(!SerialGPS) delay(2000);
  DEBUG("GPS working");
  
  while(!SD.begin(D8)) delay(2000);  
  DEBUG("SD working");

  Display.flipScreenVertically();  
  Display.clear();  
  Display.display();
}

void drawGPSscreen(GpsFormat GPSformat) {    
  Display.clear();  
  
  if (ENABLE_SERIAL_DEBUGGING) { Display.setPixel(millis() % 128, 1); }  
  
  Display.setTextAlignment(TEXT_ALIGN_LEFT);
  Display.setFont(Monospaced_plain_10);  

  //position
  Display.drawHorizontalLine(0, 35, 128);    
  Display.drawString(0, 40, "lat: " + GPSformat.Latitude);  
  Display.drawCircle(52, 45, 2);  
  Display.drawString(0, 50, "lng: " + GPSformat.Longitude);  
  Display.drawCircle(52, 55, 2);
  
  Display.display();
}

void GPSdebug(gps_fix &GPSfix) {
  if (!ENABLE_SERIAL_DEBUGGING) { return; }

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

void logToSDcard(String fileName, LinkedList<gps_fix*> &positions) {
  File file = SD.open(fileName, FILE_WRITE);
  Serial.println(fileName);

  if (!file) { 
    positions.clear();  
    return; 
  }

  for (int i = 0; i < positions.size(); ++i) {
    gps_fix* current = positions.get(i);
    file.printf("%f,%f\n", current->latitude(), current->longitude());
    Serial.print(".");
  }

  Serial.println();
      
  file.close();
  positions.clear();  
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

bool isValidFix(gps_fix &GPSfix) {
  bool validHeadingAndSpeed = !VALID_FIX_ONLY_WITH_HEADING || (
    GPSfix.valid.heading && GPSfix.valid.speed
  );

  return GPSfix.valid.location &&
         GPSfix.valid.date &&
         validHeadingAndSpeed;
}

String formatFileName(gps_fix &GPSfix) {
  return String(GPSfix.dateTime.full_year()) + "-" 
       + String(GPSfix.dateTime.month) + "-"
       + String(GPSfix.dateTime.date) + ".csv";
}

void GPSloop() {
	while (GPS.available(SerialGPS)) {		
    gps_fix GPSfix = GPS.read();
    
    if (ActionTimings.ShouldDisplay()) {      
      DEBUG("printing to display");
      GpsFormat GPSformat = formatFix(GPSfix);      
      drawGPSscreen(GPSformat);		      
    }    
        
    if (ActionTimings.ShouldLog() && isValidFix(GPSfix)) {
      DEBUG("logging to memory");
      GPSDataList.add(&GPSfix);

      if (ActionTimings.ShouldUpdateCard()) {        
        DEBUG("logging SD card");
        logToSDcard(formatFileName(GPSfix), GPSDataList);
      }
    }
      
    GPSdebug(GPSfix);		        
	}      
}

void loop() {    
  GPSloop();  
}