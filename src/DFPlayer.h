/***************************************************************************************************/
/*
   This is an Arduino library for DFPlayer Mini MP3 module

   written by : enjoyneering
   source code: https://github.com/enjoyneering/DFPlayer

   DFPlayer Mini features:
   - 3.2v..5.0v, typical 4.2v
   - 15mA without flash drive, typical 24mA
   - 24-bit DAC
   - 90dB output dynamic range
   - SNR over 85dB
   - micro SD card up to 32GB (FAT16, FAT32)
   - USB-Disk up to 32GB (FAT16, FAT32)
   - supports up to 100 folders, each folder can be assigned to 001..255 songs
   - built-in 3W mono amplifier, MD8002 AB-Class
   - UART to communicate

   NOTE:
   - if you hear a loud noise, add a 1K resistor in series with DFPlayer TX-pin

   Frameworks & Libraries:
   AVR     Core      -  https://github.com/arduino/ArduinoCore-avr
   ATtiny  Core      -  https://github.com/SpenceKonde/ATTinyCore
   ESP32   Core      -  https://github.com/espressif/arduino-esp32
   ESP8266 Core      -  https://github.com/esp8266/Arduino
   STM32   Core      -  https://github.com/stm32duino/Arduino_Core_STM32
                     -  https://github.com/rogerclarkmelbourne/Arduino_STM32
   SoftwareSerial    -  https://github.com/PaulStoffregen/SoftwareSerial
   EspSoftwareSerial -  https://github.com/plerup/espsoftwareserial


   GNU GPL license, all text above must be included in any redistribution,
   see link for details  - https://www.gnu.org/licenses/licenses.html
*/
/***************************************************************************************************/

#ifndef DFPLAYER_h
#define DFPLAYER_h

#include <Arduino.h>



/* UART frame values */
#define DFPLAYER_UART_FRAME_SIZE      0x0A //total number of bytes in UART packet, same for cmd & feedback
#define DFPLAYER_UART_START_BYTE      0x7E //start byte
#define DFPLAYER_UART_VERSION         0xFF //protocol version
#define DFPLAYER_UART_DATA_LEN        0x06 //number of data bytes, except start byte, checksum & end byte
#define DFPLAYER_UART_END_BYTE        0xEF //end byte

/* command controls */
#define DFPLAYER_PLAY_NEXT            0x01 //play next uploaded file
#define DFPLAYER_PLAY_PREV            0x02 //play prev uploaded file
#define DFPLAYER_PLAY_TRACK           0x03 //play tracks in chronological order, by upload time
#define DFPLAYER_SET_VOL_UP           0x04 //increment volume by 1
#define DFPLAYER_SET_VOL_DOWN         0x05 //decrement volume by 1
#define DFPLAYER_SET_VOL              0x06 //volume range 0..30
#define DFPLAYER_SET_EQ               0x07 //0=Off, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass
#define DFPLAYER_LOOP_TRACK           0x08 //playing & looping track number 0001..9999
#define DFPLAYER_SET_PLAY_SRC         0x09 //1=USB-Disc, 2=TF-Card, 3=Aux, 4=???, 5=NOR-Flash, 6=Sleep
#define DFPLAYER_SET_STANDBY_MODE     0x0A //put player in standby mode, not the same as sleep mode
#define DFPLAYER_SET_NORMAL_MODE      0x0B //pull player out of standby mode, DOES NOT WORK!!!
#define DFPLAYER_RESET                0x0C //reset all settings to factory default
#define DFPLAYER_RESUME_PLAYBACK      0x0D //resume playing current track
#define DFPLAYER_PAUSE                0x0E //pause playing current track
#define DFPLAYER_PLAY_FOLDER          0x0F //play track number 1..255 from folder number 1..99
#define DFPLAYER_SET_DAC_GAIN         0x10 //set DAC output gain/output voltage swing
#define DFPLAYER_REPEAT_ALL           0x11 //repeat playback all files in chronological order
#define DFPLAYER_PLAY_MP3_FOLDER      0x12 //play track number 0001..9999 from "mp3" folder 
#define DFPLAYER_PLAY_ADVERT_FOLDER   0x13 //interrupt current track & play track number 0001..9999 from "advert" folder, than resume current track
#define DFPLAYER_STOP_ADVERT_FOLDER   0x15 //stop interrupting current track to play track from "advert" folder
#define DFPLAYER_STOP_PLAYBACK        0x16 //stop playing current track
#define DFPLAYER_REPEAT_FOLDER        0x17 //repeat playback folder number 01..99
#define DFPLAYER_RANDOM_ALL_FILES     0x18 //play all tracks in random order
#define DFPLAYER_LOOP_CURRENT_TRACK   0x19 //loop currently played track
#define DFPLAYER_SET_DAC              0x1A //enable/disable DAC, 0=enable, 1=disable

/* request command controls */
#define DFPLAYER_GET_STATUS           0x42 //get current stutus, see NOTE
#define DFPLAYER_GET_VOL              0x43 //get current volume, range 0..30
#define DFPLAYER_GET_EQ               0x44 //get current EQ, 0=Off, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass
#define DFPLAYER_GET_PLAY_MODE        0x45 //get current loop mode, 0=loop all, 1=loop folder, 2=loop track, 3=random, 4=disable
#define DFPLAYER_GET_VERSION          0x46 //get software version
#define DFPLAYER_GET_QNT_USB_FILES    0x47 //get total number of tracks on USB-Disk
#define DFPLAYER_GET_QNT_TF_FILES     0x48 //get total number of tracks on TF-card
#define DFPLAYER_GET_QNT_FLASH_FILES  0x49 //get total number of tracks on NOR-Flash
#define DFPLAYER_GET_USB_TRACK        0x4B //get currently playing track number on USB-Disk
#define DFPLAYER_GET_TF_TRACK         0x4C //get currently playing track number on TF-card
#define DFPLAYER_GET_FLASH_TRACK      0x4D //get currently playing track number on NOR-Flash
#define DFPLAYER_GET_QNT_FOLDER_FILES 0x4E //get total number of tracks in folder
#define DFPLAYER_GET_QNT_FOLDERS      0x4F //DOES NOT WORK, return 04, second time 07 & stop play!!! get total number of folders in current source

/* module returned codes at the end of any playback operation or if any command error */
#define DFPLAYER_RETURN_CODE_OK       0x41 //OK, return if command is accepted & ACK/feedback byte is set to 0x01
#define DFPLAYER_RETURN_ERROR         0x40 //see NOTE
#define DFPLAYER_RETURN_CODE_DONE     0x3D //track playback is over
#define DFPLAYER_RETURN_CODE_READY    0x3F //ready after boot or reset, DL-byte current source???

class DFPlayer
{
  public:
   void begin(Stream& stream, uint16_t threshold = 100, bool response = false, bool bootDelay = true);

   void setTimeout(uint16_t threshold);
   void setResponse(bool enable);

   void setSource(uint8_t source);
   void playTrack(uint16_t track);
   void next();
   void previous();
   void pause();
   void resume();
   void stop();

   void playFolder(uint8_t folder, uint8_t track);
   void playMP3Folder(uint16_t track);
   void playAdvertFolder(uint16_t track);
   void stopAdvertFolder();

   void setVolume(uint8_t volume);
   void volumeUp();
   void volumeDown();
   void enableDAC(bool enable);
   void setDACGain(uint8_t gain, bool enable = true);
   void setEQ(uint8_t preset);

   void repeatTrack(uint16_t track);
   void repeatCurrentTrack(bool enable);
   void repeatAll(bool enable);
   void repeatFolder(uint16_t folder);
   void randomAll();

   void sleep();
   void wakeup(uint8_t source = 2);
   void enableStandby(bool enable, uint8_t source = 2);
   void reset();

   uint8_t  getStatus();
   uint8_t  getVolume();
   uint8_t  getEQ();
   uint8_t  getPlayMode();
   uint8_t  getVersion();
   uint16_t getTotalTracksSD();
   uint16_t getTotalTracksUSB();
   uint16_t getTotalTracksNORFlash();
   uint16_t getTrackSD();
   uint16_t getTrackUSB();
   uint16_t getTrackNORFlash();
   uint8_t  getTotalTracksFolder(uint8_t folder);
   uint8_t  getCommandStatus();

  private:
   Stream*  _serial;
   uint8_t  _dataBuffer[DFPLAYER_UART_FRAME_SIZE];
   bool     _ack;

   uint16_t _getResponse();
   void     _sendData(uint8_t command, uint8_t dataMSB, uint8_t dataLSB);
   uint8_t  _readData();
};

#endif
