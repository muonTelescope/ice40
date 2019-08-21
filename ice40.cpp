#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>

#include <wiringPi.h>
#include <wiringPiSPI.h>

using namespace std;

// Makefile needed
// -lwiringPi 

// ICE40 chip select GPIO22, Header Pin 15, Wiring pi Pin 3
// ICE40 reset       GPIO25, Header Pin 22, Wiring pi Pin 6
// ICE40 done        GPIO24, Header Pin 18, Wiring pi Pin 5
// SPI Channel 0

#define CS_PIN   3
#define RST_PIN  6
#define DONE_PIN 5
#define SPI_CHANNEL 0

// Argv 1 file that is being burned

int main (int argc, char** argv){
  wiringPiSetup () ;
  // SPI Mode 3, CPOL=1 CPHA=1, Clock Idle High, Sample on rising edge
  wiringPiSPISetupMode(SPI_CHANNEL, 5000000, 3);

  // Import file
  ifstream firmware;
  firmware.open (argv[1], ios::binary); 

  // int *readBuffer = malloc(sizeof(char));
  // (sizeof(readBuffer)/sizeof(readBuffer[0]))
  unsigned char data[0x1CA6+1];
  char readBuffer[16];
  for(int i = 0 ; i < 0x1CA6/16 + 1 ; i++ ){
    firmware.read(readBuffer, 16);
    for(int j = 0 ; j < 8*2 ; j+=2 ){
      data[i*16 + j] = readBuffer[j];
      data[i*16 + j+1] = readBuffer[j+1];
    }
    // printf("\n");
  }

  uint8_t lineWidth = 16;
  for(int i = 0 ; i < sizeof(data) ; i+=lineWidth ){
    printf("%04x  ", i);
    for(int j = 0 ; j < lineWidth ; j++ ){
      printf("%02x", data[i+j]);
    }
  printf("\n");
  }

  firmware.close();

  // Pin setup
  pinMode (CS_PIN, OUTPUT);
  pinMode (RST_PIN, OUTPUT);
  pinMode (DONE_PIN, INPUT);


  // Enable pullup resistor
  pullUpDnControl (DONE_PIN, PUD_UP);
  // Set CS Pin low
  digitalWrite (CS_PIN,  LOW);
  // Set RESET Pin low
  digitalWrite (RST_PIN,  LOW);
  // Delay Fig 13.2 200us+
  delayMicroseconds (200);
  // Set RESET Pin high
  digitalWrite (RST_PIN,  HIGH);  
  // Wait 1200us for internal memor clear 
  delayMicroseconds (1200);
  // Set CS Pin high
  digitalWrite (CS_PIN, HIGH);
  // 8 Dummy Clocks
  unsigned char eightClocks[1] = {0xAA};
  wiringPiSPIDataRW (SPI_CHANNEL, eightClocks, sizeof(eightClocks));
  // Set CS Pin low
  digitalWrite (CS_PIN, LOW);
  // Write BIN file over SPI
  // MSB First
  // for(i = 0 ; i < sizeof(data)/0x1000 ; i++ ){
    wiringPiSPIDataRW (SPI_CHANNEL, data, 0x1000);
    wiringPiSPIDataRW (SPI_CHANNEL, &data[0x1000], (0x1CA6 - 0x1000)+1);
  // }
  
  // Set CS Pin high
  digitalWrite (CS_PIN,  HIGH);
  // Clock dummy pulses
  unsigned char dummyPulses[20];
  wiringPiSPIDataRW (SPI_CHANNEL, dummyPulses , sizeof(dummyPulses));
  // Wait until done is high
  while(!digitalRead(DONE_PIN)){}

  return 0 ;
}

