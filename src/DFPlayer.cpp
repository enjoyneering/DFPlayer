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

#include "DFPlayer.h"



/**************************************************************************/
/*
    begin()

    Class initialization

    NOTE:
    - wait for player to boot, 1.5sec..3sec depends on SD card size
    - timeout for responses to requests 100msec..200msec
    - 0x01=module return response, 0x00=module not return response
    - average consumption 24mA
*/
/**************************************************************************/
void DFPlayer::begin(Stream &stream, uint16_t threshold, bool response, bool bootDelay)
{
  _serial = &stream;                  //serial stream
  _ack    = response;                 //0x01=module return response, 0x00=module not return response

  _serial->setTimeout(threshold);     //set maximum msec to wait for Serial.readBytes()

  if (bootDelay == true) delay(3000);
}


/**************************************************************************/
/*
    setTimeout()

    Set timeout value for request responses, in msec

    NOTE:
    - timeout for responses to requests 100msec..200msec
*/
/**************************************************************************/
void DFPlayer::setTimeout(uint16_t threshold)
{
  _serial->setTimeout(threshold); //set maximum msec to wait for Serial.readBytes()
}


/**************************************************************************/
/*
    setResponse()

    Set whether module should return a response or not

    NOTE:
    - 0x01=module return response, 0x00=module not return response
*/
/**************************************************************************/
void DFPlayer::setResponse(bool enable)
{
  _ack = enable; //1=enable, 0=disable
}


/**************************************************************************/
/*
    setSource()

    Set playback source

    NOTE:
    - source will be selected automatically if only one is present
    - this command interrupts playback
    - 1=USB-Disc, 2=TF-Card, 3=Aux, 4=???, 5=NOR-Flash, 6=Sleep
    - wait 200ms to select source
*/
/**************************************************************************/
void DFPlayer::setSource(uint8_t source)
{
  if      (source == 0) source = 1;
  else if (source >  5) source = 5;

  _sendData(DFPLAYER_SET_PLAY_SRC, 0, source);

  if (source != 6) delay(200); //6=Sleep
}


/**************************************************************************/
/*
    playTrack()

    Play tracks by number, sorted in chronological order

    NOTE:
    - files in the root must start with 4 decimal digits with leading zeros
    - example: SD_ROOT/0001 - My favorite song.mp3
    - don’t copy 0003.mp3 and then 0001.mp3, because 0003.mp3 will be
      played firts
*/
/**************************************************************************/
void DFPlayer::playTrack(uint16_t track)
{
  if      (track == 0)   track = 1;
  else if (track > 9999) track = 9999;

  _sendData(DFPLAYER_PLAY_TRACK, track >> 8, track);
}


/**************************************************************************/
/*
    next()

    Play next track in chronological order

    NOTE:
    - files in the root must start with 4 decimal digits with leading zeros
    - example: SD_ROOT/0001 - My favorite song.mp3
    - don’t copy 0003.mp3 and then 0001.mp3, because 0003.mp3 will be
      played firts
*/
/**************************************************************************/
void DFPlayer::next()
{
  _sendData(DFPLAYER_PLAY_NEXT, 0, 0);
}


/**************************************************************************/
/*
    previous()

    Play previous track in chronological order

    NOTE:
    - files in the root must start with 4 decimal digits with leading zeros
    - example: SD_ROOT/0001 - My favorite song.mp3
    - don’t copy 0003.mp3 and then 0001.mp3, because 0003.mp3 will be
      played firts
*/
/**************************************************************************/
void DFPlayer::previous()
{
  _sendData(DFPLAYER_PLAY_PREV, 0, 0);
}


/**************************************************************************/
/*
    pause()

    Pause current track
*/
/**************************************************************************/
void DFPlayer::pause()
{
  _sendData(DFPLAYER_PAUSE, 0, 0);
}


/**************************************************************************/
/*
    resume()

    Resume playing current track, after pause or stop
*/
/**************************************************************************/
void DFPlayer::resume()
{
  _sendData(DFPLAYER_RESUME_PLAYBACK, 0, 0);
}


/**************************************************************************/
/*
    stop()

    Stop playing current track

    NOTE:
    - always call stop() after pause(), otherwise a new track from another
      folder won't play
*/
/**************************************************************************/
void DFPlayer::stop()
{
  _sendData(DFPLAYER_STOP_PLAYBACK, 0, 0);
}


/**************************************************************************/
/*
    playFolder()

    Play specific track from specific folder

    NOTE:
    - folder name must be 01..99
    - files in folder must start with 3 decimal digits with leading zeros
    - up to 001..255 songs in the folder
    - example: SD_ROOT/01/001 - My favorite song.mp3
*/
/**************************************************************************/
void DFPlayer::playFolder(uint8_t folder, uint8_t track)
{
  if      (folder == 0) folder = 1;
  else if (folder > 99) folder = 99;

  if (track == 0) track = 1; //upper limit uint8_t=255

  _sendData(DFPLAYER_PLAY_FOLDER, folder, track);
}


/**************************************************************************/
/*
    playMP3Folder()

    Play specific track number from "mp3" folder

    NOTE:
    - folder name must be "mp3" or "MP3"
    - files in folder must start with 4 decimal digits with leading zeros
    - up to 0001..9999 songs in the folder
    - example: SD_ROOT/mp3/0001 - My favorite song.mp3
*/
/**************************************************************************/
void DFPlayer::playMP3Folder(uint16_t track)
{
  if      (track == 0)   track = 1;
  else if (track > 9999) track = 9999;

  _sendData(DFPLAYER_PLAY_MP3_FOLDER, track >> 8, track);
}


/**************************************************************************/
/*
    playAdvertFolder()

    Interrupt current track & play specific track number from "advert"
    folder, than resume current track

    NOTE:
    - folder name must be "advert" or "ADVERT"
    - files in folder must start with 4 decimal digits with leading zeros
    - up to 0001..9999 songs in the folder
    - example: SD_ROOT/advert/0001 - My favorite song.mp3
*/
/**************************************************************************/
void DFPlayer::playAdvertFolder(uint16_t track)
{
  if      (track == 0)   track = 1;
  else if (track > 9999) track = 9999;

  _sendData(DFPLAYER_PLAY_ADVERT_FOLDER, track >> 8, track);
}


/**************************************************************************/
/*
    stopAdvertFolder()

    Stop interrupting current track to play track from "advert" folder

    NOTE:
    - see playAdvertFolder() for details
*/
/**************************************************************************/
void DFPlayer::stopAdvertFolder()
{
  _sendData(DFPLAYER_STOP_ADVERT_FOLDER, 0, 0);
}


/************************************************************************************/
/*
    setVolume()

    Set volume

    NOTE:
    - volume range 0..30
*/
/************************************************************************************/
void DFPlayer::setVolume(uint8_t volume)
{
  if (volume <= 30) _sendData(DFPLAYER_SET_VOL, 0, volume);
}


/************************************************************************************/
/*
    volumeUp()

    Increment volume by 1
*/
/************************************************************************************/
void DFPlayer::volumeUp()
{
  _sendData(DFPLAYER_SET_VOL_UP, 0, 0);
}


/************************************************************************************/
/*
    volumeDown()

    Decrement volume by 1
*/
/************************************************************************************/
void DFPlayer::volumeDown()
{
  _sendData(DFPLAYER_SET_VOL_DOWN, 0, 0);
}


/**************************************************************************/
/*
    enableDAC()

    Enable/disable DAC

    NOTE:
    - 1=enable, 0=disable
*/
/**************************************************************************/
void DFPlayer::enableDAC(bool enable)
{
  _sendData(DFPLAYER_SET_DAC, 0, !enable); //0=enable, 1=disable
}


/**************************************************************************/
/*
    setDACGain()

    Set DAC output gain/output voltage swing

    NOTE:
    - HD-byte value, 0x01=enable gain & 0x00=disable gain
    - LD-byte value, 0..31=gain
*/
/**************************************************************************/
void DFPlayer::setDACGain(uint8_t gain, bool enable)
{
  if (gain > 31) gain = 31;

  _sendData(DFPLAYER_SET_DAC_GAIN, enable, gain);
}


/************************************************************************************/
/*
    setEQ()

    Set equalizer

    NOTE:
    - 0=Off, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass
*/
/************************************************************************************/
void DFPlayer::setEQ(uint8_t preset)
{
  if (preset <= 5) _sendData(DFPLAYER_SET_EQ, 0, preset);
}


/**************************************************************************/
/*
    setPlayMode()

    Playing & looping track number in chronological order

    NOTE:
    - don’t copy 0003.mp3 and then 0001.mp3, because 0003.mp3 will be
      played firts
*/
 /**************************************************************************/
void DFPlayer::repeatTrack(uint16_t track)
{
  if      (track == 0)   track = 1;
  else if (track > 9999) track = 9999;

  _sendData(DFPLAYER_LOOP_TRACK, track >> 8, track);
}


/**************************************************************************/
/*
    repeatCurrentTrack()

    Loop currently played track

    NOTE:
    - any playback command will switch back to normal playback mode
    - 0=stop repeating, 1=repeat currently played file
*/
/**************************************************************************/
void DFPlayer::repeatCurrentTrack(bool enable)
{
  _sendData(DFPLAYER_LOOP_CURRENT_TRACK, 0, !enable); //0=repeat, 1=stop repeat
}


/**************************************************************************/
/*
    repeatAll()

    Repeat playback all files in chronological order

    NOTE:
    - any playback command will switch back to normal playback mode
    - LD-byte values 0x01=stop repeat playback & 0x00=start repeat playback ???
    - files in the root must start with 4 decimal digits with leading zeros
    - example:  SD_ROOT/0001 - My favorite song.mp3
    - don’t copy 0003.mp3 and then 0001.mp3, because 0003.mp3 will be
      played firts
*/
/**************************************************************************/
void DFPlayer::repeatAll(bool enable)
{
  _sendData(DFPLAYER_REPEAT_ALL, 0, enable);
}


/**************************************************************************/
/*
    repeatFolder()

    Repeat playback all tracks in chronological order

    NOTE:
    - any playback command will switch back to normal playback mode
    - folder must start with 2 decimal digits with leading zeros
    - up to 01..99 folder
*/
/**************************************************************************/
void DFPlayer::repeatFolder(uint16_t folder)
{
  if      (folder == 0)  folder = 1;
  else if (folder >  99) folder = 99;

  _sendData(DFPLAYER_REPEAT_FOLDER, 0, folder);
}


/**************************************************************************/
/*
    randomAll()

    Play all tracks in random order

    NOTE:
    - any playback command will switch back to normal playback mode ??
*/
/**************************************************************************/
void DFPlayer::randomAll()
{
  _sendData(DFPLAYER_RANDOM_ALL_FILES, 0, 0);
}


/**************************************************************************/
/*
    sleep()

    Enable sleep mode

    NOTE:
    - use wakeup() to exit sleep
    - module does not respond to any playback commands in sleep mode, other
      commands OK
    - 1=USB-Disc, 2=TF-Card, 3=Aux, 4=???, 5=NOR-Flash, 6=Sleep
    - looks like does nothing, before & after 24mA
*/
/**************************************************************************/
void DFPlayer::sleep()
{
  setSource(6); //6=Sleep
}


/**************************************************************************/
/*
    wakeup()

    Disable sleep mode

    NOTE:
    - source will be selected automatically if only one is present
    - 1=USB-Disc, 2=TF-Card, 3=Aux, 4=???, 5=NOR-Flash, 6=Sleep
*/
/**************************************************************************/
void DFPlayer::wakeup(uint8_t source)
{
  if (source == 6) return; //6=Sleep prohibited

  setSource(source);
}


/**************************************************************************/
/*
    enableStandby()

    Enable/disable standby or normal mode

    NOTE:
    - true=enable standby, false=normal mode
    - 1=USB-Disc, 2=TF-Card, 3=Aux, 4=Sleep, 5=NOR Flash
    - use wakeup() to exit standby
    - module does not respond to any playback commands in standby mode,
      other commands OK
    - standby not the same as sleep mode
    - looks like does nothing, before & after 24mA
*/
/**************************************************************************/
void DFPlayer::enableStandby(bool enable, uint8_t source)
{
  if (enable == true)
  {
    _sendData(DFPLAYER_SET_STANDBY_MODE, 0, 0); //DFPLAYER_SET_NORMAL_MODE - DOES NOT WORK!!!
  }
  else
  {
   wakeup(source);
  }
}


/**************************************************************************/
/*
    reset()

    Reset all settings to factory default

    NOTE:
    - wait for player to boot, 1.5sec..3sec depends on SD card size
*/
/**************************************************************************/
void DFPlayer::reset()
{
  _sendData(DFPLAYER_RESET, 0, 0);

  delay(3000);
}


/**************************************************************************/
/*
    getStatus()

    Get current module status

    NOTE:
    - 7E FF 06 42 00 yy xx ?? ?? EF
      - yy=02 normal mode or TF??
        - xx=00 stop
        - xx=01 playing
        - xx=02 pause
      -yy=00
        - xx=02 standby
    - doesn't stop current playback
*/
/**************************************************************************/
uint8_t DFPlayer::getStatus()
{
  _sendData(DFPLAYER_GET_STATUS, 0, 0);

  switch (_getResponse())
  {
    case 0x0200:     
      return 0x00; //stop 

    case 0x0201:
      return 0x01; //playing

    case 0x0202:
      return 0x02; //pause

    case 0x0001:
      return 0x03; //sleep or standby

    default:
      return 0x04; //communication error or unknown state
  }
}


/**************************************************************************/
/*
    getVolume()

    Get current volume

    NOTE:
    - volume range 0..30
    - doesn't stop current playback
    - return "0" on communication error
*/
/**************************************************************************/
uint8_t DFPlayer::getVolume()
{
  _sendData(DFPLAYER_GET_VOL, 0, 0);

  return _getResponse();
}


/**************************************************************************/
/*
    getEQ()

    Get current EQ

    NOTE:
    - 0=Off, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass
    - doesn't stop current playback
    - return "0" on communication error
*/
/**************************************************************************/
uint8_t DFPlayer::getEQ()
{
  _sendData(DFPLAYER_GET_EQ, 0, 0);

  return _getResponse();
}


/**************************************************************************/
/*
    getPlayMode()

    Get current play mode

    NOTE:
    - 0=loop all, 1=loop folder, 2=loop track, 3=random, 4=disable
    - doesn't stop current playback
    - return "0" on communication error
*/
/**************************************************************************/
uint8_t DFPlayer::getPlayMode()
{
  _sendData(DFPLAYER_GET_PLAY_MODE, 0, 0);

  return _getResponse();
}


/**************************************************************************/
/*
    getVersion()

    Get software version

    NOTE:
    - my module return 0x08
    - doesn't stop current playback
    - return "0" on communication error
*/
/**************************************************************************/
uint8_t DFPlayer::getVersion()
{
  _sendData(DFPLAYER_GET_VERSION, 0, 0);

  return _getResponse();
}


/**************************************************************************/
/*
    getTotalTracksSD()

    Get total number of tracks on TF-Card 

    NOTE:
    - return number even if SD card is removed
    - stop current playback
    - return "0" on communication error
*/
/**************************************************************************/
uint16_t DFPlayer::getTotalTracksSD()
{
  _sendData(DFPLAYER_GET_QNT_TF_FILES, 0, 0);

  return _getResponse();
}


/**************************************************************************/
/*
    getTotalTracksUSB()

    Get total number of tracks on USB-Disk

    NOTE:
    - return "0" on communication error
    - stop current playback
*/
/**************************************************************************/
uint16_t DFPlayer::getTotalTracksUSB()
{
  _sendData(DFPLAYER_GET_QNT_USB_FILES, 0, 0);

  return _getResponse();
}


/**************************************************************************/
/*
    getTotalTracksNORFlash()

    Get total number of tracks on NOR Flash

    NOTE:
    - return "0" on communication error
    - stop current playback
*/
/**************************************************************************/
uint16_t DFPlayer::getTotalTracksNORFlash()
{
  _sendData(DFPLAYER_GET_QNT_FLASH_FILES, 0, 0);

  return _getResponse();
}


/**************************************************************************/
/*
    getTrackSD()

    Get currently playing track number on TF-Card, sorted in chronological
    order

    NOTE:
    - return track number while track is playing
    - return "0" on communication error
    - don’t copy 0003.mp3 and then 0001.mp3, because 0003.mp3 will be
      played firts
    
*/
/**************************************************************************/
uint16_t DFPlayer::getTrackSD()
{
  _sendData(DFPLAYER_GET_TF_TRACK, 0, 0);

  return _getResponse();
}


/**************************************************************************/
/*
    getTrackUSB()

    Get currently playing track number on USB-Disk, sorted in chronological
    order

    NOTE:
    - return track number while track is playing
    - return "0" on communication error
    - don’t copy 0003.mp3 and then 0001.mp3, because 0003.mp3 will be
      played firts
*/
/**************************************************************************/
uint16_t DFPlayer::getTrackUSB()
{
  _sendData(DFPLAYER_GET_USB_TRACK, 0, 0);

  return _getResponse();
}


/**************************************************************************/
/*
    getTrackUSB()

    Get currently playing track number on NOR-Flash, sorted in chronological
    order

    NOTE:
    - return "0" on communication error
*/
/**************************************************************************/
uint16_t DFPlayer::getTrackNORFlash()
{
  _sendData(DFPLAYER_GET_FLASH_TRACK, 0, 0);

  return _getResponse();
}


/**************************************************************************/
/*
    getTotalTracksFolder()

    Get total number of tracks in folder

    NOTE:
    - return "0" on communication error
    - stop current playback
*/
/**************************************************************************/
uint8_t DFPlayer::getTotalTracksFolder(uint8_t folder)
{
  _sendData(DFPLAYER_GET_QNT_FOLDER_FILES, 0, folder);

  return _getResponse();
}


/**************************************************************************/
/*
    getCommandStatus()

    Status of last get???() command

    NOTE:
    - 0x01, error module busy (this info is returned when the initialization is not done)
    - 0x02, error module in sleep mode (supports only specified device in sleep mode)
    - 0x03, error serial receiving error (request not fully received)
    - 0x04, error checksum incorrect
    - 0x05, error requested rack/folder is out of out of range
    - 0x06, error requested track/folder is not found
    - 0x07, error advert insertion error (inserting operation available when a track is being played)
    - 0x08, error SD card reading failed (SD card pulled out or damaged)
    - 0x09, error ???
    - 0x0A, error entered sleep mode

    - 0x0B, OK, return if command is accepted and feedback byte is set to 0x01
    - 0x0C, OK, track playback is completed, module return this status automatically after the track has been played
    - 0x0D, OK, ready after boot or reset with DL-byte current source???, module return this status automatically
    - 0x00, unknown status
*/
/**************************************************************************/
uint8_t DFPlayer::getCommandStatus()
{
  if (_dataBuffer[3] == DFPLAYER_RETURN_ERROR)      return _dataBuffer[6]; //see NOTE

  if (_dataBuffer[3] == DFPLAYER_RETURN_CODE_OK)    return 0x0B;
  if (_dataBuffer[3] == DFPLAYER_RETURN_CODE_DONE)  return 0x0C;
  if (_dataBuffer[3] == DFPLAYER_RETURN_CODE_READY) return 0x0D;
                                                    return 0x00;
}




/**************************************************************************/
/*
    _sendData()

    Send data via Serial port

    NOTE:
    - DFPlayer data frame format:
      0      1    2    3    4    5   6   7     8     9-byte
      START, VER, LEN, CMD, ACK, DH, DL, SUMH, SUML, END
             -------- checksum --------
*/
 /**************************************************************************/
void DFPlayer::_sendData(uint8_t command, uint8_t dataMSB, uint8_t dataLSB)
{
  int16_t checksum = 0; //zero, DON'T TOUCH!!!

  _dataBuffer[0] = DFPLAYER_UART_START_BYTE;
  _dataBuffer[1] = DFPLAYER_UART_VERSION;
  _dataBuffer[2] = DFPLAYER_UART_DATA_LEN;
  _dataBuffer[3] = command;
  _dataBuffer[4] = _ack;
  _dataBuffer[5] = dataMSB;
  _dataBuffer[6] = dataLSB;

  checksum = checksum - _dataBuffer[1] - _dataBuffer[2] - _dataBuffer[3] - _dataBuffer[4] - _dataBuffer[5] - _dataBuffer[6];

  _dataBuffer[7] = checksum >> 8;
  _dataBuffer[8] = checksum; 

  _dataBuffer[9] = DFPLAYER_UART_END_BYTE;

  _serial->write(_dataBuffer, DFPLAYER_UART_FRAME_SIZE);
}


/**************************************************************************/
/*
    _readData()

    Read MP3 player request response

    NOTE:
    - timeout for responses to requests 100msec..200msec

    - DFPlayer data frame format:
      0      1    2    3    4    5   6   7     8     9-byte
      START, VER, LEN, CMD, ACK, DH, DL, SUMH, SUML, END
*/
 /**************************************************************************/
uint8_t DFPlayer::_readData()
{
  _serial->flush();                                                             //clear serial FIFO

  memset(_dataBuffer, 0x00, DFPLAYER_UART_FRAME_SIZE);                          //clear data buffer

  uint8_t dataSize = _serial->readBytes(_dataBuffer, DFPLAYER_UART_FRAME_SIZE); //read serial, wait for data during setTimeout() period

  if (dataSize < DFPLAYER_UART_FRAME_SIZE)        return 0;                     //request not fully received

  if (_dataBuffer[0] != DFPLAYER_UART_START_BYTE) return 1;                     //start byte missing
  if (_dataBuffer[1] != DFPLAYER_UART_VERSION)    return 2;                     //version byte missing
  if (_dataBuffer[2] != DFPLAYER_UART_DATA_LEN)   return 3;                     //length byte missing
  if (_dataBuffer[3] == DFPLAYER_RETURN_ERROR)    return 4;                     //error received, call getCommandStatus() for details
  if (_dataBuffer[9] != DFPLAYER_UART_END_BYTE)   return 5;                     //end byte missing

                                                  return 6;                     //OK, no errors!!!
}


/**************************************************************************/
/*
    _getResponse()

    Get response value from MP3 player

    NOTE:
    - return "0" on communication error
*/
 /**************************************************************************/
uint16_t DFPlayer::_getResponse()
{
  if (_readData() == 6) return ((uint16_t)_dataBuffer[5] << 8) | _dataBuffer[6]; 
                        return 0;
}
