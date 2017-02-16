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
#include <Wire.h>
//#include <SPI_Anything.h>
#include <Ethernet2.h>

#define CC1310_SS_PIN 8
#define GATEWAY_MAX_NODES 7
#define RADIO_PACKET_TYPE_SENSOR_PACKET 3
#define RADIO_PACKET_TYPE_ACCEL_PACKET 4

EthernetClient client;
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };  // MAC address for the ethernet controller.
char serverName[] = "74.208.156.48";
char pageName[] = "/newRaw";  // http resource
int serverPort = 80; // change to your server's port
const unsigned long BAUD_RATE = 9600;                 // serial connection speed
const size_t MAX_CONTENT_SIZE = 512;       // max size of the HTTP response
const unsigned PACKET_SIZE = 13;

int buffer[PACKET_SIZE];

struct TemperatureData {
  unsigned char objtemp_l;
  unsigned char objtemp_h;
};

struct AccelerationData {
  unsigned char x;
  unsigned char y;
  unsigned char z;
};

struct HeartrateData {
  unsigned char ambtemp_l;
  unsigned char ambtemp_h;
  unsigned char rate_l;
  unsigned char rate_h;
};

/* will be concat of data from all sensors */
struct SampleData {
  unsigned char cowID;
  unsigned char packetType;
  struct TemperatureData tempData;
  struct AccelerationData accelerometerData;
  struct HeartrateData heartRateData;
  unsigned long timestamp;

  unsigned char errorCode;
};

// Print the data extracted from the JSON
void printSampleData(const struct SampleData* sampleData) {
  Serial.println("Sample Data");
  Serial.print("Cow ID ");
  Serial.println(sampleData->cowID);
  Serial.print("Packet Type ");
  Serial.println(sampleData->packetType);
  Serial.print("Timestamp: ");
  Serial.println(sampleData->timestamp);
  Serial.print("Error Code ");
  Serial.println(sampleData->errorCode);
  if(sampleData->packetType == RADIO_PACKET_TYPE_SENSOR_PACKET){
    Serial.print("Object Temperature High ");
    Serial.println(sampleData->tempData.objtemp_h);
    Serial.print("Object Temperature Low ");
    Serial.println(sampleData->tempData.objtemp_l);
    Serial.print("Heart Rate High ");
    Serial.println(sampleData->heartRateData.rate_h);
    Serial.print("Heart Rate Low ");
    Serial.println(sampleData->heartRateData.rate_l);
    Serial.print("Ambient Temperature High ");
    Serial.println(sampleData->heartRateData.ambtemp_h);
    Serial.print("Ambient Temperature Low ");
    Serial.println(sampleData->heartRateData.ambtemp_l);
  } else if(sampleData->packetType == RADIO_PACKET_TYPE_ACCEL_PACKET){
    Serial.print("Acceleration x-axis ");
    Serial.println(sampleData->accelerometerData.x);
    Serial.print("Acceleration y-axis ");
    Serial.println(sampleData->accelerometerData.y);
    Serial.print("Acceleration z-axis ");
    Serial.println(sampleData->accelerometerData.z);
  }
}

// Parse the JSON from the input string and extract the interesting values
// Here is the JSON we need to parse/send
//{
//  "cowID": x,
//  "packetType": x,
//  "timestamp": x,
//  "objtemp_h": x,
//  "objtemp_l": x,
//  "hrate_high": x,
//  "hrate_low": x,
//  "ambtemp_h": x,
//  "ambtemp_l": x,
//  "errcode": x
//}
//{
//  "cowID": x,
//  "packetType": x,
//  "timestamp": x,
//  "xaxis": x,
//  "yaxis": x,
//  "zaxis": x,
//  "errcode": x
//}
 
void serializeSampleData(const struct SampleData* sampleData, char *data)
{
  if(sampleData->packetType == RADIO_PACKET_TYPE_SENSOR_PACKET){ 
    sprintf(data, "{\"cowID\": %i, \"packetType\": %i, \"timestamp\": %lu, \"objtemp_h\": %i, \"objtemp_l\": %i, \"hrate_high\": %i, \"hrate_low\": %i, \"ambtemp_h\": %i, \"ambtemp_l\": %i, \"errcode\": %i}\n",
              sampleData->cowID,
              sampleData->packetType,
              sampleData->timestamp,
              sampleData->tempData.objtemp_h,
              sampleData->tempData.objtemp_l,
              sampleData->heartRateData.rate_h,
              sampleData->heartRateData.rate_l,
              sampleData->heartRateData.ambtemp_h,
              sampleData->heartRateData.ambtemp_l,
              sampleData->errorCode);
  }else if(sampleData->packetType == RADIO_PACKET_TYPE_ACCEL_PACKET){
    sprintf(data, "{\"cowID\": %i, \"packetType\": %i, \"timestamp\": %lu, \"xaxis\": %i, \"yaxis\": %i, \"zaxis\": %i, \"errcode\": %i}\n",
              sampleData->cowID,
              sampleData->packetType,
              sampleData->timestamp,
              sampleData->accelerometerData.x,
              sampleData->accelerometerData.y,
              sampleData->accelerometerData.z,
              sampleData->errorCode);
  }
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

  Wire.begin(6);
  Wire.onReceive(receiveEvent);

}

// ARDUINO entry point #2: runs over and over again forever
void loop() {
  wait();
}

void receiveEvent(int howMany) {
  Serial.println("Receiving");
  int i = 0;
  
  SampleData sampleData;
   char data[MAX_CONTENT_SIZE];

  //Get Data via I2C
  while (1 <= Wire.available()) { // loop through all
    unsigned char c = Wire.read(); // receive byte as a int
    //Serial.println(c);         // print the character
    buffer[i] = c;
    i++;
  }

  //Store in JSON object and post to gateway
  sampleData.cowID = buffer[0];
  sampleData.packetType = buffer[1];
  sampleData.timestamp = ((unsigned long)buffer[2]<<24 | (unsigned long)buffer[3]<<16 | (unsigned long)buffer[4]<<8 | (unsigned long)buffer[5]);
  
  if(sampleData.packetType == RADIO_PACKET_TYPE_SENSOR_PACKET){
    sampleData.tempData.objtemp_h = buffer[6];
    sampleData.tempData.objtemp_l = buffer[7];
    sampleData.heartRateData.rate_h = buffer[8];
    sampleData.heartRateData.rate_l = buffer[9];
    sampleData.heartRateData.ambtemp_h = buffer[10];
    sampleData.heartRateData.ambtemp_l = buffer[11];
    sampleData.errorCode = buffer[12];
  }else if(sampleData.packetType == RADIO_PACKET_TYPE_ACCEL_PACKET){
    sampleData.accelerometerData.x = buffer[6];
    sampleData.accelerometerData.y = buffer[7];
    sampleData.accelerometerData.z = buffer[8];
    sampleData.errorCode = buffer[9];
  }

  //printSampleData(&sampleData);

  //Serial.println("Creating JSON Object");
  serializeSampleData(&sampleData,data);
  
  //Serial.print("\n\r");
  Serial.println(data);

  if (!postPage(serverName, serverPort, pageName, data)) {
    Serial.println(F("Fail "));
  }
  else {
    Serial.println(F("Pass "));
  }
  Serial.println("Done");
}
