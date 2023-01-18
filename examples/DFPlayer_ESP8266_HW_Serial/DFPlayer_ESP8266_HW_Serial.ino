/***************************************************************************************************/
/*
   This is an Arduino sketch for DFPlayer Mini MP3 module

   written by : enjoyneering
   source code: https://github.com/enjoyneering/DFPlayer

   DFPlayer Mini features:
   - 3.2v..5.0v, typical 4.2v
   - 15mA without flash drive, typical 24mA
   - 24-bit DAC with 90dB output dynamic range and SNR over 85dB
   - micro SD-card, up to 32GB (FAT16, FAT32)
   - USB-Disk up to 32GB (FAT16, FAT32)
   - supports mp3 sampling rate 8KHz, 11.025KHz, 12KHz, 16KHz, 22.05KHz, 24KHz, 32KHz, 44.1KHz, 48KHz
   - supports up to 100 folders, each folder can be assigned to 001..255 songs
   - built-in 3W mono amplifier, NS8002 AB-Class with standby function
   - UART to communicate, 9600bps (parity:none, data bits:8, stop bits:1, flow control:none)

   NOTE:
   - if you hear a loud noise, add a 1K resistor in series with DFPlayer TX pin
   - move the jumper from right to left to automatically switch the amplifier to standby

   Frameworks & Libraries:
   ESP8266 Core      -  https://github.com/esp8266/Arduino
   EspSoftwareSerial -  https://github.com/plerup/espsoftwareserial


   GNU GPL license, all text above must be included in any redistribution,
   see link for details  - https://www.gnu.org/licenses/licenses.html
*/
/***************************************************************************************************/

#include <DFPlayer.h>


#define MP3_SERIAL_SPEED    9600 //DFPlayer Mini suport only 9600-baud
#define MP3_SERIAL_TIMEOUT  350  //average DFPlayer response timeout 100msec..400msec, (YX5200/AAxxxx)...(GD3200B/MH2024K)


DFPlayer mp3;                    //connect DFPlayer RX-pin to GPIO15(TX) & DFPlayer TX-pin to GPIO13(RX)


/**************************************************************************/
/*
    setup()

    Main setup

    NOTE:
    - moduleType:
      - DFPLAYER_MINI:
        - DFPlayer Mini module
        - MP3-TF-16P module
        - FN-M16P module
        - YX5200 chip
        - YX5300 chip
        - JL AAxxxx chip
      - DFPLAYER_FN_X10P:
        - FN-M10P module
        - FN-S10P module
        - FN6100 chip
      - DFPLAYER_HW_247A:
        - HW-247A module
        - GD3200B chip
      - DFPLAYER_NO_CHECKSUM:
        - no checksum calculation (not recomended for MCU without external
          crystal oscillator)
*/
/**************************************************************************/
void setup()
{
  Serial.begin(MP3_SERIAL_SPEED, SERIAL_8N1);                     //hardware serial on GPIO01(TX) & GPIO03(RX) after boot

  mp3.begin(Serial, MP3_SERIAL_TIMEOUT, DFPLAYER_HW_247A, false); //"DFPLAYER_HW_247A" see NOTE, false=no feedback from module after the command

  Serial.swap();     //now hardware serial on GPIO15(TX) & GPIO13(RX)
  Serial.flush();    //clear serial library buffer, ALWAYS CLEAN BUFFER AFTER SERIAL SWAP!!!

  mp3.stop();        //if player was runing during ESP8266 reboot
  mp3.reset();       //reset all setting to default
  
  mp3.setSource(2);  //1=USB-Disk, 2=TF-Card, 3=Aux, 4=Sleep, 5=NOR Flash
  
  mp3.setEQ(0);      //0=Off, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass
  mp3.setVolume(25); //0..30, module persists volume on power failure

  mp3.sleep();       //inter standby mode, 24mA

  Serial.swap();     //now hardware serial on GPIO01(TX) & GPIO03(RX)
  Serial.flush();    //clear serial library buffer, ALWAYS CLEAN BUFFER AFTER SERIAL SWAP!!!
}


/**************************************************************************/
/*
    loop()

    Main loop
*/
 /**************************************************************************/
void loop()
{
  Serial.swap();                      //now hardware serial on GPIO15(TX) & GPIO13(RX)
  Serial.flush();                     //clear serial library buffer, ALWAYS CLEAN BUFFER AFTER SERIAL SWAP!!!

  mp3.wakeup(2);                      //exit standby mode & initialize sourse 1=USB-Disk, 2=TF-Card, 3=Aux, 5=NOR Flash

  mp3.playTrack(1);                   //play track #1, don’t copy 0003.mp3 and then 0001.mp3, because 0003.mp3 will be played firts
//mp3.playMP3Folder(1);               //1=track, folder name must be "mp3" or "MP3" & files in folder must start with 4 decimal digits with leading zeros
//mp3.playFolder(1, 2);               //1=folder/2=track, folder name must be 01..99 & files in folder must start with 3 decimal digits with leading zeros

  mp3.setResponse(true);              //enable=request feedback to return not only errors but also OK statuses

  uint8_t response = mp3.getVolume(); //0..30

  Serial.swap();                      //now hardware serial on GPIO01(TX) & GPIO03(RX)
  Serial.flush();                     //clear serial library buffer, ALWAYS CLEAN BUFFER AFTER SERIAL SWAP!!!

  Serial.println(response);           //print volume to Arduino Serial Monitor

  Serial.swap();                      //now hardware serial on GPIO15(TX) & GPIO13(RX)
  Serial.flush();                     //clear serial library buffer, ALWAYS CLEAN BUFFER AFTER SERIAL SWAP!!!

  response = mp3.getCommandStatus();  //1=module busy, 2=module sleep, 3=request not fully received, 4=checksum not match
                                      //5=requested folder/track out of range, 6=requested folder/track not found
                                      //7=advert available while track is playing, 8=SD card not found, 9=???, 10=module sleep
                                      //11=OK command accepted, 12=OK playback completed, 13=OK module ready after reboot

  Serial.swap();                      //now hardware serial on GPIO01(TX) & GPIO03(RX)
  Serial.flush();                     //clear serial library buffer, ALWAYS CLEAN BUFFER AFTER SERIAL SWAP!!!

  Serial.println(response);           //print command status to Arduino Serial Monitor
}
