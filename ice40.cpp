#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "ice40.h"

using namespace std;

// Instatiates a new class and sets up the pins
ICE40::ICE40(const uint8_t CS_PIN, const uint8_t DONE_PIN, const uint8_t RST_PIN, const uint8_t SPI_CHANNEL){
  _CS_PIN = CS_PIN;
  _DONE_PIN = DONE_PIN;
  _RST_PIN = RST_PIN;
  _SPI_CHANNEL = SPI_CHANNEL;
  setup(_SPI_CHANNEL, 1000000);
}

void ICE40::setup(const uint8_t SPI_CHANNEL, const uint32_t clkSpeed){
    // Initilize wiring pi libray
  wiringPiSetup () ;
  // SPI Mode 3, CPOL=1 CPHA=1, Clock Idle High, Sample on rising edge
  wiringPiSPISetupMode(_SPI_CHANNEL, clkSpeed, 3);

  // Set pin modes
  pinMode (_CS_PIN, OUTPUT);
  pinMode (_RST_PIN, OUTPUT);
  pinMode (_DONE_PIN, INPUT);
  // Enable pullup resistor
  pullUpDnControl (_DONE_PIN, PUD_UP);
}

void ICE40::configure(const char filename[]){
  writeFile(filename);
}


void ICE40::writeFile(const char filename[]){
  // Import file
  ifstream bitstream;
  bitstream.open (filename, ios::binary);


  // Determine file size 
  uint16_t bitstreamSize = (int)bitstream.tellg();
  bitstream.seekg (0, ios::end);
  bitstreamSize = (int)bitstream.tellg() - bitstreamSize;
  bitstream.seekg (0);

  unsigned char *data = new unsigned char[bitstreamSize];
  for(int i = 0 ; i < bitstreamSize ; i++ ){
    char readBuffer[1];
    bitstream.read(readBuffer, 1);
    data[i] = readBuffer[0];
  }

  //Print out file
  hexDump(data, bitstreamSize);


  bitstream.close();

  burnData(data, bitstreamSize);

  delete [] data;

}


void ICE40::burnData(unsigned char *data, uint16_t length){
  clear();
  // Set CS Pin high
  digitalWrite (_CS_PIN, HIGH);
  // 8 Dummy Clocks
  unsigned char eightClocks[1] = {0xAA};
  wiringPiSPIDataRW (_SPI_CHANNEL, eightClocks, sizeof(eightClocks));
  // Set CS Pin low
  digitalWrite (_CS_PIN, LOW);
  // Write BIN file over SPI, MSB First
  // for(i = 0 ; i < sizeof(data)/0x1000 ; i++ ){
    wiringPiSPIDataRW (_SPI_CHANNEL, data, 0x1000);
    wiringPiSPIDataRW (_SPI_CHANNEL, &data[0x1000], (length - 0x1000)+1);
  // }
  
  // Set CS Pin high
  digitalWrite (_CS_PIN,  HIGH);
  // Clock dummy pulses
  unsigned char dummyPulses[20];
  wiringPiSPIDataRW (_SPI_CHANNEL, dummyPulses , sizeof(dummyPulses));
  // Wait until done is high
  while(!digitalRead(_DONE_PIN)){}
}

void ICE40::clear(){
  // Set CS Pin low
  digitalWrite (_CS_PIN,  LOW);
  // Set RESET Pin low
  digitalWrite (_RST_PIN,  LOW);
  // Delay Fig 13.2 200us+
  delayMicroseconds (200);
  // Set RESET Pin high
  digitalWrite (_RST_PIN,  HIGH);  
  // Wait 1200us for internal memor clear 
  delayMicroseconds (1200);
}

void ICE40::hexDump(unsigned char *data, uint16_t length){
    //Print out file
  const uint8_t lineWidth = 16;
  for(int i = 0 ; i < (length+1) ; i+=lineWidth ){
    printf("%04x  ", i);
    for(int j = 0 ; j < lineWidth ; j++ ){
      if(i+j < length){
        printf("%02x", data[i+j]);
      }
    }
  printf("\n");
  }
}