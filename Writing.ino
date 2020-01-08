#include <Wire.h>

#define EEPROM_ADR 0b1010000
#define MAX_I2C_READ_WRITE_BUFFER 16 
#define MAX_WRITE_ATTEMPTS 10
#define MAX_WAITING_TIME 100
#define VERBOSE true


unsigned char dataToWrite[] = {

// comma seperated bytes here
 
};


bool problems = false;
void setup() {

  Serial.begin(9600);

  Wire.begin();
  Wire.setClock(400000);

  int start = millis();
  Serial.println("Start write...");
  
  writeEEPROM(0, dataToWrite, sizeof(dataToWrite)/sizeof(dataToWrite[0]));
  Serial.println("Write complete!");
  if (problems) {
    Serial.print("Problem(s) encountered during execution.");
    if(!VERBOSE) {
      Serial.println("Enable verbose mode to learn more.");
    } else {
      Serial.println(); 
    }
  } 
  Serial.print(millis() - start); Serial.println("ms");

/*
  delay(500);
  int numBytesToRead = 50;
  int startAddress = 0;
  unsigned char readData[numBytesToRead];
  readEEPROM(startAddress, numBytesToRead, readData);
  for (int i = 0 ; i < sizeof(readData)/sizeof(readData[0]) ; i++) {
    Serial.println(readData[i], DEC);
  }
*/
}

void loop() {}

void writeEEPROM(int startAddress, unsigned char data[], int dataLength) {
  executeWrite(startAddress, data, dataLength);
  delay(100);
  validate(startAddress, data, dataLength);
  validate(startAddress, data, dataLength);
}

void executeWrite(int startAddress, unsigned char data[], int dataLength) {
  if(VERBOSE) {
    Serial.print("Write: ");  Serial.print(startAddress); Serial.print(" - ");  Serial.println(dataLength); 
  }
  int currentAddress = startAddress;
  int currentWriteIndex = 0;
  while (currentWriteIndex < dataLength) {
    if(VERBOSE) {
      Serial.print("Section: ");  Serial.println(currentAddress);
    }
    Wire.beginTransmission(EEPROM_ADR);
    Wire.write(currentAddress >> 8);
    Wire.write(currentAddress & 0xFF);
    for (int i = 0 ; i < MAX_I2C_READ_WRITE_BUFFER ; i++) {
      if (currentWriteIndex + i < dataLength) {
        Wire.write(data[currentWriteIndex + i]);
      }
    }
    currentAddress += MAX_I2C_READ_WRITE_BUFFER; 
    currentWriteIndex += MAX_I2C_READ_WRITE_BUFFER;
    Wire.endTransmission();
    delay(1);
  };
}

bool validate(int startAddress, unsigned char data[], int dataLength) {
  if(VERBOSE) {
    Serial.println("Validate: ");
  }
  int currentAddress = startAddress;
  int currentReadIndex = 0;
  int attempts = 0;
  while (currentReadIndex < dataLength) {
    Wire.beginTransmission(EEPROM_ADR);
    Wire.write(currentAddress >> 8);
    Wire.write(currentAddress & 0xFF);
    Wire.endTransmission();
    int readLength = MAX_I2C_READ_WRITE_BUFFER;
    if (dataLength - currentReadIndex < MAX_I2C_READ_WRITE_BUFFER) {
      readLength = dataLength - currentReadIndex; 
    }
    Wire.requestFrom(EEPROM_ADR, readLength);
    int waitingTime = 0;
    while(!Wire.available() && waitingTime <= MAX_WAITING_TIME) {
      delay(1);
      waitingTime++;
    }

    if(waitingTime != MAX_WAITING_TIME) {
      int i = 0;
      bool equal = true;
      while(Wire.available() && equal) {
        int value = Wire.read();
        if(VERBOSE) {
          Serial.print(value, DEC); Serial.print(" - ");
          Serial.println(data[currentReadIndex + i], DEC);
        }
        equal = value == data[currentReadIndex + i];
        i++;
      };
      if (equal|| attempts == MAX_WRITE_ATTEMPTS) {
        if(attempts == MAX_WRITE_ATTEMPTS && VERBOSE) {
          Serial.println("Max write attempts reached");
          problems = true;
        }
        attempts = 0;
        currentAddress += MAX_I2C_READ_WRITE_BUFFER; 
        currentReadIndex += MAX_I2C_READ_WRITE_BUFFER;
      } else {
        if(VERBOSE) {
          Serial.print("Attempt retry: ");  Serial.println(attempts);
        } 
        unsigned char testData[MAX_I2C_READ_WRITE_BUFFER];
        for (int i = 0; i < MAX_I2C_READ_WRITE_BUFFER; i++) {
          testData[i] = data[currentReadIndex + i];
        }
        executeWrite(currentAddress, testData, MAX_I2C_READ_WRITE_BUFFER);
        attempts++;
      }
    } else {
      if(VERBOSE) {
        Serial.println("Max waiting time reached");
      }
      problems = true;
      attempts = 0;
      currentAddress += MAX_I2C_READ_WRITE_BUFFER; 
      currentReadIndex += MAX_I2C_READ_WRITE_BUFFER;  
    }
  };
}

void readEEPROM(int startAddress, int dataLength, unsigned char *dataReceiver) {
  int currentAddress = startAddress;
  int currentReadIndex = 0;
  while (currentReadIndex < dataLength) {
    Wire.beginTransmission(EEPROM_ADR);
    Wire.write(currentAddress >> 8);
    Wire.write(currentAddress & 0xFF);
    Wire.endTransmission();
    int readLength = MAX_I2C_READ_WRITE_BUFFER;
    if (dataLength - currentReadIndex < MAX_I2C_READ_WRITE_BUFFER) {
      readLength = dataLength - currentReadIndex; 
    }
    Wire.requestFrom(EEPROM_ADR, readLength);
    int waitingTime = 0;
    while(!Wire.available() && waitingTime <= MAX_WAITING_TIME) {
      delay(1);
      waitingTime++;
    }

    if (waitingTime == MAX_WAITING_TIME) {
      problems = true;
    }

    int i = 0;
    while(Wire.available()) {
      dataReceiver[currentReadIndex + i] = Wire.read();
      i++;
    }
    currentAddress += MAX_I2C_READ_WRITE_BUFFER; 
    currentReadIndex += MAX_I2C_READ_WRITE_BUFFER;
  };
}
