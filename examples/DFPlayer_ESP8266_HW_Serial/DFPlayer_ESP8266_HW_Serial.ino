/***************************************************************************************************/
/*
   This is an Arduino sketch for DFPlayer Mini MP3 module

   written by : enjoyneering
   source code: https://github.com/enjoyneering/DFPlayer

   DFPlayer Mini features:
   - 3.2v..5.0v, typical 4.2v
   - average curren 24mA
   - 24-bit DAC
   - 90dB output dynamic range
   - SNR over 85dB
   - micro SD card up to 32GB (FAT16, FAT32)
   - USB-Disk up to 32GB (FAT16, FAT32)
   - supports up to 100 folders, each folder can be assigned to 001..255 songs
   - built-in 3W mono amplifier, MD8002 AB-Class
   - UART to communicate

   NOTE:
   - if you hear a loud noise, add a 1K resistor in series with DFPlayer TX pin
   - move the jumper from right to left to automatically switch the amplifier to standby

   Framework:
   ESP8266 Core - https://github.com/esp8266/Arduino


   GNU GPL license, all text above must be included in any redistribution,
   see link for details  - https://www.gnu.org/licenses/licenses.html
*/
/***************************************************************************************************/

#include <DFPlayer.h>


#define MP3_SERIAL_SPEED    9600 //DFPlayer Mini suport only 9600-baud
#define MP3_SERIAL_TIMEOUT  100  //average DFPlayer response timeout 100msec..200msec


DFPlayer mp3;                    //connect DFPlayer RX-pin to GPIO15(TX) & DFPlayer TX-pin to GPIO13(RX)

uint8_t response = 0;


/**************************************************************************/
/*
    setup()

    Main setup

    NOTE:
    - moduleType:
      - DFPLAYER_MINI: DFPlayer Mini, MP3-TF-16P, FN-M16P (YX5200, YX5300,
        JL AAxxxx)
      - DFPLAYER_FN_X10P: FN-M10P, FN-S10P (FN6100)
      - DFPLAYER_HW_247A: DFPlayer Mini HW-247A (GD3200B)
      - DFPLAYER_NO_CHECKSUM: no checksum calculation (not recomended for
        MCU without external crystal oscillator)
*/
 /**************************************************************************/
void setup()
{
  Serial.begin(MP3_SERIAL_SPEED, SERIAL_8N1);                  //hardware serial on GPIO01(TX) & GPIO03(RX) after boot

  mp3.begin(Serial, MP3_SERIAL_TIMEOUT, DFPLAYER_MINI, false); //DFPLAYER_MINI see NOTE, false=no response from module after the command

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
  Serial.swap();                     //now hardware serial on GPIO15(TX) & GPIO13(RX)
  Serial.flush();                    //clear serial library buffer, ALWAYS CLEAN BUFFER AFTER SERIAL SWAP!!!

  mp3.wakeup(2);                     //exit standby mode & initialize sourse 1=USB-Disk, 2=TF-Card, 3=Aux, 5=NOR Flash

  mp3.playTrack(1);                  //play track #1, don’t copy 0003.mp3 and then 0001.mp3, because 0003.mp3 will be played firts
//mp3.playMP3Folder(1);              //1=track, folder name must be "mp3" or "MP3" & files in folder must start with 4 decimal digits with leading zeros
//mp3.playFolder(1, 2);              //1=folder/2=track, folder name must be 01..99 & files in folder must start with 3 decimal digits with leading zeros

  mp3.setResponse(true);             //enable=request a response, to return not only errors, but also OK statuses

  response = mp3.getVolume();        //0..30

  Serial.swap();                     //now hardware serial on GPIO01(TX) & GPIO03(RX)
  Serial.flush();                    //clear serial library buffer, ALWAYS CLEAN BUFFER AFTER SERIAL SWAP!!!

  Serial.println(response);          //print volume to Arduino Serial Monitor

  Serial.swap();                     //now hardware serial on GPIO15(TX) & GPIO13(RX)
  Serial.flush();                    //clear serial library buffer, ALWAYS CLEAN BUFFER AFTER SERIAL SWAP!!!

  response = mp3.getCommandStatus(); //1=Error, module busy|2=Error, module sleep|3=Error, request not fully received|4= Error, checksum not match
                                     //5=Error, requested folder/track out of range|6=Error, requested folder/track not found|7=Error, advert available while track is playing
                                     //8=Error, SD card not found|9=???|10=Error, module sleep|11=OK, command accepted|12=OK, playback completed|13=OK, module ready after reboot

  Serial.swap();                     //now hardware serial on GPIO01(TX) & GPIO03(RX)
  Serial.flush();                    //clear serial library buffer, ALWAYS CLEAN BUFFER AFTER SERIAL SWAP!!!

  Serial.println(response);          //print command status to Arduino Serial Monitor via USB-COM
}
