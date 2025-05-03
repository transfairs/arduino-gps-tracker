///////////////////////////////////////////
// 1.1 Include necessary libraries
///////////////////////////////////////////

#include "AssetTracker/AssetTracker.h"
#include "HttpClient/HttpClient.h"
#include "TinyGPS/TinyGPS.h"

 STARTUP(cellular_credentials_set("internet", "", "", NULL));

///////////////////////////////////////////
// 1 Initiation
///////////////////////////////////////////

/******************************************
 * 1.1 General
******************************************/

int             sleepTime = 60;
AssetTracker    assetTracker = AssetTracker();
FuelGauge       batteryState;


/******************************************
 * 1.2 GPS
******************************************/

TinyGPS         tinyGPS;
struct data{
  float  altitude, course, latitude, longitude;
  int    satellites;
  bool   valid;
  String string;
}               gps_data;
bool            isValidGPS, validStart;
int             sentences;
int             serial1Available;

/******************************************
 * 1.3 HTTPS
******************************************/

#define         HOST "127.0.0.1"
#define         PATH "/server/electron"
#define         PORT 80
HttpClient      httpClient;

http_request_t  request;
http_response_t response;



///////////////////////////////////////////
// 2 Setup
///////////////////////////////////////////

void setup() {
  Time.setTime(Particle.syncTime());
  // Open a serial port to observe programm over USB serial monitor
  Serial.begin(9600);

  // Turn GPS module on, 1.5 seconds delay
  assetTracker.begin();
  assetTracker.gpsOn();

  // Initiate cloud functionality
  /* ToDo: add more
  *
  * get actual position
  * restart device
  * change to/from power saving mode
  *
  */
  Particle.function("batteryStats", getBatteryStatus);
}



///////////////////////////////////////////
// 3 Main loop
///////////////////////////////////////////

void loop() {

  /******************************************
   * 3.1 Get GPS data
  ******************************************/

  // Initiate validation of new GPS data
  isValidGPS          = false;
  gps_data.latitude   = 0.0;
  gps_data.longitude  = 0.0;
  gps_data.valid      = false;
  validStart          = false;
  sentences           = 0;

  // Check whether GPS shield is connected and working
  serial1Available = Serial1.available();
  if (serial1Available >= 63) {
    // Get GPS data
    for (int i=0; i<serial1Available; i++) {
      Serial1.read();
    }
  }
  // Check valid sentence
  if (Serial1.available()>0 && Serial1.peek() != '$') {
    while (Serial1.peek() != '$' && Serial1.peek() != -1)
    // empty the buffer up to a new $ or end of buffer
    Serial1.read();
  }

  // Collect five valid sentences and calculate an average position
  for (unsigned long start = millis(); millis()-start<1000 || (validStart && sentences <= 5);) {
    while (Serial1.available()) {
      char c = Serial1.read();
      if (!validStart && c == '$') {
        // Mark beginning of the sentence
        validStart = true;
      }

      // Incrementally handle and parse GPS data. Returns true after a valid sentence received
      if (validStart) {
        if (tinyGPS.encode(c)) {
          isValidGPS = true;
          validStart = false;
          sentences++;
        }
      }
    }
  }

  if (isValidGPS) {
    unsigned long age;
    tinyGPS.f_get_position(&gps_data.latitude, &gps_data.longitude, &age);
    Serial.println();
    Serial.println("GPS data struct:");
    Serial.println("================");
    Serial.print("latitude: ");
    Serial.println(gps_data.latitude);
    Serial.print("longitude: ");
    Serial.println(gps_data.longitude);
    Serial.print("age: ");
    Serial.println(age);

//    sprintf(szInfo, "%.4f,%.4f alt: %.1f course: %.1f sats: %d",
    gps_data.latitude   = (gps_data.latitude  == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : gps_data.latitude);
    gps_data.longitude  = (gps_data.longitude == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : gps_data.longitude);
    gps_data.altitude   = tinyGPS.f_altitude();
    gps_data.course     = tinyGPS.f_course();
    gps_data.satellites = tinyGPS.satellites();
    gps_data.valid      = true;

    Serial.print("altitude: ");
    Serial.println(gps_data.altitude);
    Serial.print("course: ");
    Serial.println(gps_data.altitude);
    Serial.print("satellites: ");
    Serial.println(gps_data.satellites);
    Serial.println();
    Serial.print("new latitude: ");
    Serial.println(gps_data.latitude);
    Serial.print("new longitude: ");
    Serial.println(gps_data.longitude);
    Serial.println("================");
    Serial.println();
  }


  /******************************************
   * 3.2 Send data over 3G
  ******************************************/

//  if (httpClient.connect(HOST, PORT)) {
  if (true) {
    request.hostname = HOST;
    request.path     = PATH;
    request.port     = PORT;

    gps_data.string = "<=>#" + getCoreID() + "#0#GPS:" + gps_data.latitude + ";" + gps_data.longitude + "#ALT:" + gps_data.altitude + "#COURSE_OG:" + gps_data.course + "#SATELLITES:" + gps_data.satellites + "#SPEED_OG:-1#TIME:" + Time.now() + "#";

    Serial.print("str = ");
    Serial.println(gps_data.string);
    String hex = str2hex(gps_data.string);

    request.body = "frame=" + hex;

    http_header_t headers[] = {
        {"Content-Type", "application/x-www-form-urlencoded"},
        { "User-agent", "Particle HttpClient"},
        { NULL, NULL }
    };

    httpClient.post(request, response, headers);
  }
  else {
    Serial.println("connection failed");
  }
  Serial.println();
  Serial.println();
  System.sleep(SLEEP_MODE_DEEP, 1770);
}


String getCoreID()
{
  String coreIdentifier = "";
  char id[12];
  memcpy(id, (char *)ID1, 12);
  char hex_digit;
  for (int i = 0; i < 12; ++i) {
    hex_digit = 48 + (id[i] >> 4);
    if (57 < hex_digit) hex_digit += 39;
    coreIdentifier = coreIdentifier + hex_digit;
    hex_digit = 48 + (id[i] & 0xf);
    if (57 < hex_digit) hex_digit += 39;
    coreIdentifier = coreIdentifier + hex_digit;
  }
  return coreIdentifier;
}


String str2hex(const String& input)
{
  static const char* const lut = "0123456789ABCDEF";
  size_t len = input.length();
  String output;

  output.reserve(2 * len);
  for (size_t i = 0; i < len; ++i) {
    const unsigned char c = input[i];
    output += lut[c >> 4];
    output += lut[c & 15];
  }
  return output;
}

int getBatteryStatus(String command) {
  Particle.publish("B", "v: " + String::format("%.2f", batteryState.getVCell()) +
              ", c: " + String::format("%.2f", batteryState.getSoC()), 60, PRIVATE);
  return 1;
}
