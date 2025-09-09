#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// RX, TX pair setup
// didnt use default pins cuz might interfere with serial outputs
SoftwareSerial gpsSerial(3, 4); 
TinyGPSPlus gps;

// machine states
enum State {
  IDLE,
  ASCENT,
  APOGEE,
  DESCENT,
  PAYLOAD,
  LANDED
};

State currentState = IDLE;
float maxAlt = 0;
float prevAlt = 0;
bool payload = false;

// moving average setup
#define ALT_WINDOW 5   // number of samples for moving average
float altBuffer[ALT_WINDOW];
int altIndex = 0;
bool bufferFilled = false;

void setup() {
  Serial.begin(9600);  
  // default freq    
  gpsSerial.begin(9600);     
  delay(1000); // delay for power up time

  // couldnt rly find exact syntax for the commands so had to consult ai; gave conflicting answers
  gpsSerial.print("$QGPSCFG=\"nmeaout\",0,0,0,0,0,0,0,0*20\r\n"); // disable all
  delay(100);
  gpsSerial.print("$QGPSCFG=\"nmeaout\",1,1,0,0,0,0,0,0*27\r\n"); // enable RMC and GGA
  delay(100);
  // delays in middle to just give some breathing room

  Serial.println("GPS configured: only RMC & GGA enabled.");
}

void loop() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Only process when new data is available(saves pain of async running)
  if (gps.location.isUpdated() || gps.altitude.isUpdated() || gps.time.isUpdated()) {
    float rawAlt = gps.altitude.meters();
    float filtAlt = getFilteredAltitude(rawAlt); // moving avg filter

    printData(filtAlt);
    updateStateMachine(filtAlt);
  }
}

// moving avg fn
float getFilteredAltitude(float newAlt) {
  altBuffer[altIndex] = newAlt;
  altIndex = (altIndex + 1) % ALT_WINDOW;

  if (altIndex == 0) bufferFilled = true;

  int count = bufferFilled ? ALT_WINDOW : altIndex;
  float sum = 0;
  for (int i = 0; i < count; i++) {
    sum += altBuffer[i];
  }
  return sum / count;
}

void printData(float altitude) {
  Serial.print("Time: ");
  if (gps.time.isValid()) {
    // found out serial.printf wont work on all arduino boards, uno esp, so used ai to generate a recursive printing algorithm
    char buf[9];
    sprintf(buf, "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
    Serial.print(buf);
  } else {
    Serial.print("INVALID");
  }

  Serial.print(" | Lat: "); Serial.print(gps.location.lat(), 6);
  Serial.print(" | Lon: "); Serial.print(gps.location.lng(), 6);
  Serial.print(" | Alt: "); Serial.print(altitude);
  Serial.print("m | State: ");

  switch (currentState) {
    case IDLE: Serial.print("IDLE"); break;
    case ASCENT: Serial.print("ASCENT"); break;
    case APOGEE: Serial.print("APOGEE"); break;
    case DESCENT: Serial.print("DESCENT"); break;
    case PAYLOAD: Serial.print("PAYLOAD"); break;
    case LANDED: Serial.print("LANDED"); break;
  }
  Serial.println();
}

void updateStateMachine(float currAlt) {
  // errpr lim
  const float altTolerance = 5.0; 

  switch (currentState) {
    case IDLE:
      if (currAlt > prevAlt + altTolerance) {
        currentState = ASCENT;
        maxAlt = currAlt;
      }
      break;

    case ASCENT:
      if (currAlt > maxAlt) maxAlt = currAlt;
      if (currAlt < maxAlt - altTolerance) currentState = APOGEE;
      break;

    case APOGEE:
      if (currAlt < maxAlt - altTolerance) currentState = DESCENT;
      break;

    case DESCENT:
      if (!payload && currAlt <= maxAlt*0.75 + altTolerance) {
        currentState = PAYLOAD;
        payload = true;
      }
      if (currAlt < 5 && abs(currAlt - prevAlt) < altTolerance) currentState = LANDED;
      break;

    case PAYLOAD:
      if (currAlt < 5 && abs(currAlt - prevAlt) < altTolerance) currentState = LANDED;
      break;

    case LANDED:
      // Remain in landed state
      break;
  }

  prevAlt = currAlt;
}

