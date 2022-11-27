#include <SoftwareSerial.h>
#include <NMEAGPS.h>
#include <SH1106Wire.h>
#include <math.h>

SH1106Wire Display(0x3c, SDA, SCL);
NMEAGPS GPS;
SoftwareSerial SerialGPS(D3, D4);

struct GpsFormat {
  char* Latitude;
  char* Longitude;
};

void setup() {
  Serial.begin(9600);
  SerialGPS.begin(9600);
  Display.init();
  Display.flipScreenVertically();  
  Display.clear();  
  Display.display();
}

void drawGPSscreen(GpsFormat gpsFormat) {    
  Display.setTextAlignment(TEXT_ALIGN_LEFT);
  Display.setFont(Monospaced_plain_10);

  Display.drawHorizontalLine(0, 35, 128);
  //Display.drawString(0, 40, "lat:  50  19.34'");  
  Display.drawString(0, 40, "lat: " + String(gpsFormat.Latitude));  
  Display.drawCircle(52, 45, 2);
  //Display.drawString(0, 50, "lng: 019  12.34'");  
  Display.drawString(0, 50, "lng: " + String(gpsFormat.Longitude));  
  Display.drawCircle(52, 55, 2);
  Display.display();
}

void drawGPSdebug(gps_fix &GPSfix) {
  if (GPSfix.valid.location) {
		Serial.println("location float");	
		Serial.printf("%f\n", GPSfix.latitude());
    Serial.printf("%f\n", GPSfix.longitude());  			
    Serial.println("location int");		
    Serial.printf("%d\n", GPSfix.latitudeL());
		Serial.printf("%d\n", GPSfix.longitudeL());  			    
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

void logToSDcard(GpsFormat gpsFormat) {

}

//190391997
//502563175
GpsFormat formatFix(gps_fix &gps_fix) {  
  //char latitudeString[255] =  " --  --.--' -";
  //char longitudeString[255] = "---  --.--' -";  
  
  if (gps_fix.valid.location) {
    float latitudeFloat  = gps_fix.latitude();
    float longitudeFloat = gps_fix.longitude();

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
    
    Serial.printf(" %02d  %02d.%02d' %c", latDegrees, latMinutes, latMinutesDecimal, latHem);
    Serial.println();
    Serial.printf("%03d  %02d.%02d' %c", lngDegrees, lngMinutes, lngMinutesDecimal, lngHem);

    //sprintf(latitudeString, " %02d  %02d.%02d' %c", latDegrees, latMinutes, latMinutesDecimal, latHem);
    //sprintf(longitudeString, "%03d  %02d.%02d' %c", lngDegrees, lngMinutes, lngMinutesDecimal, lngHem);

    //Serial.println(latitudeString);
    //Serial.println(longitudeString);
  }

  GpsFormat result = {
    " --  --.--' -",
    "---  --.--' -"
    //latitudeString,
    //longitudeString
  };

  return result;
}

void GPSloop()
{
	while (GPS.available(SerialGPS)) {		
    gps_fix GPSfix = GPS.read();
    GpsFormat gpsFormat = formatFix(GPSfix);
    drawGPSdebug(GPSfix);		
    drawGPSscreen(gpsFormat);		
    logToSDcard(gpsFormat);
	}    
}

void loop() {    
  GPSloop();  
}