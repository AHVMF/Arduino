// ArduCAM demo (C)2015 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with a supported camera modules, and can run on any Arduino platform.
//
// This demo was made for ArduCAM Mini 2MP camera module.
// It will run 4 ArduCAM Mini 2MP moduels and take time elapse images every 5 seconds.
// The demo sketch will do the following tasks:
// 1. Set the sensor to JPEG mode.
// 2. Capture 4 image from 4 cameras simultaneously and buffer the image to FIFO every 5 seconds 
// 3. Store the image to Micro SD/TF card with JPEG format in sequential.
// 4. Resolution can be changed by myCAM.set_JPEG_size() function.
// This program requires the ArduCAM V3.4.0 (or later) library and ArduCAM Mini 2MP shield
// and use Arduino IDE 1.5.2 compiler or above
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>///////////
#include "memorysaver.h"
/////////////////////////////////
#define SD_CS 9
#if defined(__arm__)
#include <itoa.h>
#endif
/////////////////////////////////
const int CS1 = 4;
const int CS2 = 5;
const int CS3 = 6;
const int CS4 = 7;
bool cam1 = true, cam2 = true, cam3 = true, cam4 = true;

/////////////////////////////////
ArduCAM myCAM1(OV2640, CS1);
ArduCAM myCAM2(OV2640, CS2);
ArduCAM myCAM3(OV2640, CS3);
ArduCAM myCAM4(OV2640, CS4);

void setup() {
  // put your setup code here, to run once:
  uint8_t vid, pid;
  uint8_t temp;
#if defined (__AVR__)
  Wire.begin();
#endif
#if defined(__arm__)
  Wire1.begin();
#endif
  Serial.begin(115200);
  Serial.println("ArduCAM Start!");

  // set the SPI_CS as an output:
  pinMode(CS1, OUTPUT);
  pinMode(CS2, OUTPUT);
  pinMode(CS3, OUTPUT);
  pinMode(CS4, OUTPUT);

  // initialize SPI:
  SPI.begin();
  //Check if the ArduCAM SPI bus is OK
  myCAM1.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM1.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55)
  {
    Serial.println("SPI1 interface Error!");
    cam1 = false;
    //while(1);
  }

  myCAM2.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM2.read_reg(ARDUCHIP_TEST1);
  //Serial.println(temp);
  if (temp != 0x55)
  {
    Serial.println("SPI2 interface Error!");
    cam2 = false;
    //while(1);
  }

  myCAM3.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM3.read_reg(ARDUCHIP_TEST1);
  //Serial.println(temp);
  if (temp != 0x55)
  {
    Serial.println("SPI3 interface Error!");
    cam3 = false;
    //while(1);
  }

  myCAM4.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM4.read_reg(ARDUCHIP_TEST1);
  //Serial.println(temp);
  if (temp != 0x55)
  {
    Serial.println("SPI4 interface Error!");
    cam4 = false;
    //while(1);
  }

  //Check if the camera module type is OV2640
  myCAM1.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM1.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);

  if ((vid != 0x26) || (pid != 0x42))
    Serial.println("Can't find OV2640 module!");
  else
    Serial.println("OV2640 detected");

  //Change to JPEG capture mode and initialize the OV2640 module
  myCAM1.set_format(JPEG);
  myCAM1.InitCAM();
  myCAM1.OV2640_set_JPEG_size(OV2640_1600x1200);
  myCAM1.clear_fifo_flag();
  myCAM1.write_reg(ARDUCHIP_FRAMES, 0x00);
  ////////////////////////////////////////////////////
  if (!SD.begin(SD_CS))
  {
    //while (1);		//If failed, stop here
    Serial.println("SD Card Error");
  }
  else
    Serial.println("SD Card detected!");
  ////////////////////////////////////////////////////
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t temp, temp_last;
  uint8_t start_capture = 0;

  delay(5000);

  myCAM1.flush_fifo();
  myCAM2.flush_fifo();
  myCAM3.flush_fifo();
  myCAM4.flush_fifo();
  //Clear the capture done flag
  myCAM1.clear_fifo_flag();
  //Start capture
  myCAM1.start_capture();
  //Clear the capture done flag
  myCAM2.clear_fifo_flag();
  //Start capture
  myCAM2.start_capture();
  //Clear the capture done flag
  myCAM3.clear_fifo_flag();
  //Start capture
  myCAM3.start_capture();
  //Clear the capture done flag
  myCAM4.clear_fifo_flag();
  //Start capture
  myCAM4.start_capture();


  while (!myCAM1.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK) && cam1);
  if (cam1 == true)
  {
    Serial.println("CAM1 Capture Done!");
    read_fifo_burst(myCAM1);
    //Clear the capture done flag
    myCAM1.clear_fifo_flag();
  }

  while (!myCAM2.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK) && cam2);
  if (cam2 == true)
  {
    Serial.println("CAM2 Capture Done!");
    read_fifo_burst(myCAM2);
    //Clear the capture done flag
    myCAM2.clear_fifo_flag();
  }
  
  while (!myCAM3.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK) && cam3);
  if (cam3 == true)
  {
    Serial.println("CAM3 Capture Done!");
    read_fifo_burst(myCAM3);
    //Clear the capture done flag
    myCAM3.clear_fifo_flag();
  }
  
  while (!myCAM4.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK) && cam4);
  if (cam4 == true)
  {
    Serial.println("CAM4 Capture Done!");
    read_fifo_burst(myCAM4);
    //Clear the capture done flag
    myCAM4.clear_fifo_flag();
  }
}

uint8_t read_fifo_burst(ArduCAM myCAM)
{
  uint8_t temp, temp_last;
  uint32_t length = 0;
  //////////////////////////////////////////
  char str[8];
  File outFile;
  byte buf[256];
  static int k = 0;
  static int i = 0;
  //////////////////////////////////////////
  ////////////////////////////////////////////////////
  //Construct a file name
  k = k + 1;
  itoa(k, str, 10);
  strcat(str, ".jpg");
  //Open the new file
  outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
  if (! outFile)
  {
    Serial.println("open file failed");
    //return;
  }

  //////////////////////////////////////////////////
  length = myCAM.read_fifo_length();
  if (length >= 393216) //384 kb
  {
    Serial.println("Not found the end.");
    return 0;
  }
  //Serial.println(length);
  i = 0;
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  SPI.transfer(0x00);//First byte is 0xC0 ,not 0xff

  ////////////////////////////////////////////////////////
  while ( (temp != 0xD9) | (temp_last != 0xFF))
  {
    temp_last = temp;
    temp = SPI.transfer(0x00);

    //Write image data to buffer if not full
    if (i < 256)
      buf[i++] = temp;
    else
    {
      //Write 256 bytes image data to file
      myCAM.CS_HIGH();
      outFile.write(buf, 256);
      i = 0;
      buf[i++] = temp;
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();
    }
  }
  //Write the remain bytes in the buffer
  if (i > 0)
  {
    myCAM.CS_HIGH();
    outFile.write(buf, i);
  }
  //Close the file
  outFile.close();
  ///////////////////////////////////////////////////////
}
