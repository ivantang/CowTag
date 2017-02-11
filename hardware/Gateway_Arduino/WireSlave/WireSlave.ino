#include <Wire.h>

void setup() {
  initSerial();
  Wire.begin(6);                // join i2c bus with address #8
  //Wire.setClock(400000L);
  Wire.onReceive(receiveEvent); // register event
  //Serial.begin(9600);           // start serial for output
}

void loop() {
  delay(100);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  Serial.println("Receiving");
  while (1 <= Wire.available()) { // loop through all but the last
    int c = Wire.read(); // receive byte as a character
    Serial.println(c);         // print the character
  }
  Serial.println("Done");

//  int x = Wire.read();    // receive byte as an integer
//  Serial.println(x);         // print the integer
//  Serial.print("Done\n");
}

// Initialize Serial port
void initSerial() {
  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to initialize
  }
  Serial.println("Serial ready");
}
