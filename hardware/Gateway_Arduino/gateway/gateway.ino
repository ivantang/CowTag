// Gateway Client 72point5
// Gateway connected to cc1310, waiting for a new sensor packet to be
// transmitted to our server
//
// JSON protocols gotten from https://github.com/bblanchon/ArduinoJson
// Copyright Benoit Blanchon 2014-2016
// MIT License
//
// Post web client method extrated from  http://playground.arduino.cc/Code/WebClient
// posted November 2012 by SurferTim
//
// SPI protocols extrated from http://www.gammon.com.au/spi
// Copyright Nick Gammon January 2011
// Common atribute License 3.0 https://creativecommons.org/licenses/by/3.0/au/legalcode
//
// Modified by Erik Dandanell
#include <ArduinoJson.h>
#include <SPI.h>
//#include <SPI_Anything.h>
#include <Ethernet2.h>
#include <time.h>

#define CC1310_SS_PIN 8
#define GATEWAY_MAX_NODES 7

EthernetClient client;
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };  // MAC address for the ethernet controller.
char serverName[] = "74.208.156.48";
char pageName[] = "/newRaw";  // http resource
int serverPort = 80; // change to your server's port
char json_string[] = "{\"body_temp\": 11.0, \"ext_temp\": 11.0, \"x\": 1.0, \"y\": 1.0, \"z\": 1.0, \"respire\": 100.0, \"cow_id\":1, \"timestamp\": 1455555555, \"error\": \"0\"}";
const unsigned long BAUD_RATE = 9600;                 // serial connection speed
const size_t MAX_CONTENT_SIZE = 512;       // max size of the HTTP response

typedef enum {OK, LOWBATTERY, SENSORERROR} errCode;

struct TemperatureData {
  char temp_l[16];
  char temp_h[16];
};

struct AccelerationData {
  char x[16];
  char y[16];
  char z[16];
};

struct HeartrateData {
  char temp_l[16];
  char temp_h[16];
  char rate_l[16];
  char rate_h[16];
};

/* will be concat of data from all sensors */
struct SampleData {
  struct TemperatureData tempData;
  struct AccelerationData accelerometerData;
  struct HeartrateData heartRateData;
  errCode errorCode;
  time_t timeStamp;
  char nodeAddress[8];
};

// Print the data extracted from the JSON
void printSampleData(const struct SampleData* data) {
  Serial.print("Temperature:");
  Serial.print(" body_temp=");
  //Serial.print(data->tempData.temp_h);
  //Serial.print(".");
  Serial.print(data->tempData.temp_l);

  Serial.print(" ext_Temp=");
  //Serial.print(data->heartRateData.temp_h);
  //Serial.print(".");
  Serial.println(data->heartRateData.temp_l);

  Serial.print("Accelerometer:");
  Serial.print(" x=");
  Serial.print(data->accelerometerData.x);
  Serial.print(" y=");
  Serial.print(data->accelerometerData.y);
  Serial.print(" z=");
  Serial.println(data->accelerometerData.z);

  Serial.print("Respiration rate= ");
  //Serial.print(data->heartRateData.rate_h);
  //Serial.print(".");
  Serial.println(data->heartRateData.rate_l);

  Serial.print("CowId= ");
  Serial.println(data->nodeAddress);

  Serial.print("Timestamp= ");
  Serial.println(data->timeStamp);

  Serial.print("Error code= ");
  Serial.println(data->errorCode);
}

void serializeSampleData(const struct SampleData* sampleData, char *data)
{
  sprintf(data, "{\"body_temp\": %s, \"ext_temp\": %s, \"x\": %s, \"y\": %s, \"z\": %s, \"respire\": %s, \"cow_id\": %s, \"timestamp\": %ld, \"error\": \" %i \"}\n",
          sampleData->tempData.temp_l,
          sampleData->heartRateData.temp_l,
          sampleData->accelerometerData.x,
          sampleData->accelerometerData.y,
          sampleData->accelerometerData.z,
          sampleData->heartRateData.rate_l,
          sampleData->nodeAddress,
          sampleData->timeStamp,
          sampleData->errorCode);
}

// Parse the JSON from the input string and extract the interesting values
// Here is the JSON we need to parse/send
//{
//  "body_temp": x.x,
//  "ext_temp": x.x,
//  "x": x,
//  "y": x,
//  "z": x,
//  "respire": x,
//  "cow_id": x,
//  "timestamp": x,
//  "error": "x"
//}
bool parseSampleData(char* content, struct SampleData* sampleData) {
  // Compute optimal size of the JSON buffer according to what we need to parse.
  // This is only required if you use StaticJsonBuffer.
  const size_t BUFFER_SIZE =
    JSON_OBJECT_SIZE(9);

  // Allocate a temporary memory pool on the stack
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  // If the memory pool is too big for the stack, use this instead:
  // DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(content);

  if (!root.success()) {
    Serial.println("JSON parsing failed!");
    return false;
  }

  // Here were copy the strings we're interested in
  strcpy(sampleData->tempData.temp_l, root["body_temp"]);;
  strcpy(sampleData->heartRateData.temp_l, root["ext_temp"]);
  strcpy(sampleData->accelerometerData.x, root["x"]);
  strcpy(sampleData->accelerometerData.y, root["y"]);
  strcpy(sampleData->accelerometerData.z, root["z"]);
  strcpy(sampleData->heartRateData.rate_l, root["respire"]);
  strcpy(sampleData->nodeAddress, root["cow_id"]);
  strcpy(sampleData->nodeAddress, root["cow_id"]);
  sampleData->timeStamp = root["timestamp"];
  int errorcode =  root["error"];
  if (errorcode == 0) {
    sampleData->errorCode = OK;
  } else if ( errorcode == 1 ) {
    sampleData->errorCode = LOWBATTERY;
  } else {
    sampleData->errorCode = SENSORERROR;
  }

  return true;
}

// Initialize Serial port
void initSerial() {
  Serial.begin(BAUD_RATE);
  while (!Serial) {
    ;  // wait for serial port to initialize
  }
  Serial.println("Serial ready");
}

// Initialize Ethernet library
void initEthernet() {
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
    if (!Ethernet.begin(mac)) {
      Serial.println("Failed to configure Ethernet");
      return;
    }
  Serial.println("Ethernet ready");
  delay(1000);
}

// Pause for 5 seconds
void wait() {
  Serial.println("Wait 5 seconds");
  delay(5000);
}

byte postPage(char* domainBuffer, int thisPort, char* page, char* thisData)
{
  int inChar;
  char outBuf[64];

  Serial.print(F("connecting..."));

  if (client.connect(domainBuffer, thisPort) == 1)
  {
    Serial.println(F("connected"));

    // send the header
    sprintf(outBuf, "POST %s HTTP/1.1", page);
    client.println(outBuf);
    sprintf(outBuf, "Host: %s", domainBuffer);
    client.println(outBuf);
    client.println(F("Connection: close\r\nContent-Type: application/json"));
    sprintf(outBuf, "Content-Length: %u\r\n", strlen(thisData));
    client.println(outBuf);

    // send the body (variables)
    client.print(thisData);
  }
  else
  {
    Serial.println(F("failed"));
    return 0;
  }

  int connectLoop = 0;

  while (client.connected())
  {
    while (client.available())
    {
      inChar = client.read();
      Serial.write(inChar);
      connectLoop = 0;
    }

    delay(1);
    connectLoop++;
    if (connectLoop > 10000)
    {
      Serial.println();
      Serial.println(F("Timeout"));
      client.stop();
    }
  }

  Serial.println();
  Serial.println(F("disconnecting."));
  client.stop();
  return 1;
}

// ARDUINO entry point #1: runs once when you press reset or power the board
void setup() {
  // disable SD SPI
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  initSerial();
  initEthernet();

  // disable cc1310 SPI
  // pinMode(8,OUTPUT);
  // digitalWrite(8,HIGH);

}

bool newTagDataRequest(struct SampleData* sampleData) {
  SPI.begin ();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  digitalWrite(CC1310_SS_PIN, LOW);
  //SPI_readAnything(sampleData);
  digitalWrite(CC1310_SS_PIN, HIGH);
  return true;
}

// ARDUINO entry point #2: runs over and over again forever
void loop() {
  SampleData sampleData;
  char data[MAX_CONTENT_SIZE];

  // //struc acrobatics
  Serial.println(json_string);
  strncpy(data,json_string,sizeof(json_string));
   Serial.println("parsing and printing sampledata");
   parseSampleData(data,&sampleData);
   printSampleData(&sampleData);
   Serial.println("serializing sampledata");
   serializeSampleData(&sampleData,data);
  Serial.println(data);

  //if(newTagDataRequest(&sampleData)){
  // printSampleData(&sampleData);
  // serializeSampleData(&sampleData,data);
  if (!postPage(serverName, serverPort, pageName, data)) {
    Serial.println(F("Fail "));
  }
  else {
    Serial.println(F("Pass "));
  }
  //}

  wait();
}
