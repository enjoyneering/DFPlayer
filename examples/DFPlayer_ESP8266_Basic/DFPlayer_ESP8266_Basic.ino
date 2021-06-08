/***************************************************************************************************/
/*
   This is an Arduino sketch for DFPlayer Mini MP3 module

   written by : enjoyneering
   source code: https://github.com/enjoyneering/DFPlayer

   DFPlayer Mini features:
   - 3.2v..5.0v, typical 4.2v
   - 15mA, without flash drive
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

   Frameworks & Libraries:
   ESP8266 Core      -  https://github.com/esp8266/Arduino
   EspSoftwareSerial -  https://github.com/plerup/espsoftwareserial


   GNU GPL license, all text above must be included in any redistribution,
   see link for details  - https://www.gnu.org/licenses/licenses.html
*/
/***************************************************************************************************/

#include <SoftwareSerial.h>
#include <DFPlayer.h>


#define MP3_RX_PIN              5     //GPIO5/D1
#define MP3_TX_PIN              4     //GPIO4/D2
#define MP3_SERIAL_SPEED        9600  //DFPlayer Mini &suport only 9600-baud
#define MP3_SERIAL_BUFFER_SIZE  32    //software serial buffer size in bytes, to send 8-bytes you need 11-bytes buffer (start byte+8-data bytes+parity-byte+stop-byte=11-bytes)
#define MP3_SERIAL_TIMEOUT      100   //average DFPlayer response timeout 100msec..200msec


SoftwareSerial mp3Serial;
DFPlayer       mp3;


void setup()
{
  Serial.begin(115200);

  mp3Serial.begin(MP3_SERIAL_SPEED, SWSERIAL_8N1, MP3_RX_PIN, MP3_TX_PIN, false, MP3_SERIAL_BUFFER_SIZE, 0); //false=signal not inverted, 0=ISR/RX buffer size (shared with serial TX buffer)

  mp3.begin(mp3Serial, MP3_SERIAL_TIMEOUT, false); //false=no response from module

  mp3.stop();                                      //if player was runing during ESP8266 reboot
  mp3.reset();                                     //reset all setting to default
  
  mp3.setSource(2);                                //1=USB-Disk, 2=TF-Card, 3=Aux, 4=Sleep, 5=NOR Flash
  
  mp3.setEQ(0);                                    //0=Off, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass
  mp3.setVolume(25);                               //0..30, module persists volume on power failure

  mp3.sleep();                                     //inter standby mode, 24mA
}

void loop()
{
  mp3.wakeup(2);                                   //exit standby mode & initialize sourse 1=USB-Disk, 2=TF-Card, 3=Aux, 5=NOR Flash

  mp3.playTrack(1);                                //play track #1, don’t copy 0003.mp3 and then 0001.mp3, because 0003.mp3 will be played firts
//mp3.playMP3Folder(1);                            //1=track, folder name must be "mp3" or "MP3" & files in folder must start with 4 decimal digits with leading zeros
//mp3.playFolder(1, 2);                            //1=folder/2=track, folder name must be 01..99 & files in folder must start with 3 decimal digits with leading zeros

//mp3Serial.enableRx(true);                        //enable interrupts on RX-pin, if serial ISR/RX buffer shared with TX buffer (less overhead than mp3Serial.listen())
//Serial.println(getVolume());                     //0..30
//Serial.println(getCommandStatus());              //1=Error, module busy|2=Error, module sleep|3=Error, request not fully received|4= Error, checksum not match
                                                   //5=Error, requested folder/track out of range|6=Error, requested folder/track not found|7=Error, advert available while track is playing
                                                   //8=Error, SD card not found|9=???|10=Error, module sleep|11=OK, command accepted|12=OK, playback completed|13=OK, module ready after reboot
//mp3Serial.enableRx(false);                       //disable interrupts on RX-pin, if serial ISR/RX buffer shared with TX buffer (less overhead than mp3Serial.listen())
}
