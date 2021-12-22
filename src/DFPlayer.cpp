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
   - micro SD card, up to 32GB (FAT16, FAT32)
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
    - DAC is turned on by default after boot or reset
    - average consumption 24mA
*/
/**************************************************************************/
void DFPlayer::begin(Stream &stream, uint16_t threshold, DFPLAYER_MODULE_TYPE moduleType, bool response, bool bootDelay)
{
  _serial     = &stream;    //serial stream
  _threshold  = threshold;  //timeout responses, in msec
  _ack        = response;   //0x01=module return response after the command, 0x00=module not return response after the command
  _moduleType = moduleType; //DFPlayer or Clone, differ in how checksum is calculated

  if (bootDelay == true) {delay(3000);}
}


/**************************************************************************/
/*
    setModel()

    Set module type, DFPlayer or Clone

    NOTE:
    - moduleType:
      - DFPLAYER_MINI: DFPlayer Mini, MP3-TF-16P, FN-M16P (YX5200, YX5300,
        JL AA20HF)
      - DFPLAYER_FN_X10P: FN-M10P, FN-S10P (FN6100)
      - DFPLAYER_HW_247A: DFPlayer Mini HW-247A (GD3200B)
      - DFPLAYER_NO_CHECKSUM: no checksum calculation (not recomended for
        MCU without external crystal oscillator)

    - DFPlayer clones have more or less the same commands, the difference
      is in the checksum calculation
*/
/**************************************************************************/
void DFPlayer::setModel(DFPLAYER_MODULE_TYPE moduleType)
{
  _moduleType = moduleType;
}


/**************************************************************************/
/*
    setTimeout()

    Set timeout responses, in msec

    NOTE:
    - average timeout responses 100msec..200msec
*/
/**************************************************************************/
void DFPlayer::setTimeout(uint16_t threshold)
{
  _threshold = threshold; //timeout responses, in msec
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
    - 1=USB-Disc, 2=TF-Card, 3=Aux, 4=???, 5=NOR-Flash, 6=Sleep (3..6 may
      not be supported by some modules!!!)
    - module automatically detect if source is on-line
    - module automatically enter standby after setting source
    - this command interrupt playback!!!
    - wait 200ms to select source
*/
/**************************************************************************/
void DFPlayer::setSource(uint8_t source)
{
  if      (source == 0) {source = 1;}
  else if (source >  6) {source = 6;}

  _sendData(DFPLAYER_SET_PLAY_SRC, 0, source);

  if (source != 6) {delay(200);} //6=Sleep
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
  if      (track == 0)   {track = 1;}
  else if (track > 9999) {track = 9999;}

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
    - up to 001..255 songs in each folder
    - files in folder must start with 3 decimal digits with leading zeros
      - example: SD_ROOT/01/001 - My favorite song.mp3
*/
/**************************************************************************/
void DFPlayer::playFolder(uint8_t folder, uint8_t track)
{
  if      (folder == 0) {folder = 1;}
  else if (folder > 99) {folder = 99;}

  if (track == 0) {track = 1;} //upper limit uint8_t=255

  _sendData(DFPLAYER_PLAY_FOLDER, folder, track);
}


/**************************************************************************/
/*
    playMP3Folder()

    Play specific track number from "mp3" folder

    NOTE:
    - folder name must be "mp3" or "MP3"
    - up to 0001..9999 songs in each folder
    - files in folder must start with 4 decimal digits with leading zeros
      - example: SD_ROOT/mp3/0001 - My favorite song.mp3
    - module speed will decrease as the folder gets bigger, place no more
      than 3000 tracks to keep the speed
*/
/**************************************************************************/
void DFPlayer::playMP3Folder(uint16_t track)
{
  if      (track == 0)   {track = 1;}
  else if (track > 9999) {track = 9999;}

  _sendData(DFPLAYER_PLAY_MP3_FOLDER, track >> 8, track);
}


/**************************************************************************/
/*
    play3000Folder()

    Play specific track number from folder, if you need more than 256
    tracks in a folder

    NOTE:
    - folder name must be 01..15
    - up to 0001..3000 songs in each folder
    - may not be supported by some modules!!!
    - files in folder must start with 4 decimal digits with leading zeros
      - example: SD_ROOT/01/0001 - My favorite song.mp3
*/
/**************************************************************************/
void DFPlayer::play3000Folder(uint16_t track)
{
  if      (track == 0)   {track = 1;}
  else if (track > 3000) {track = 3000;}

  _sendData(DFPLAYER_PLAY_3000_FOLDER, track >> 8, track);
}


/**************************************************************************/
/*
    playAdvertFolder()

    Interrupt current track & play specific track number from "advert"
    folder, than resume current track

    NOTE:
    - folder name must be "advert" or "ADVERT"
    - up to 0001..9999 songs in each folder
    - command does't work when module is paused or stopped
    - files in folder must start with 4 decimal digits with leading zeros
      - example: SD_ROOT/advert/0001 - My favorite song.mp3
*/
/**************************************************************************/
void DFPlayer::playAdvertFolder(uint16_t track)
{
  if      (track == 0)   {track = 1;}
  else if (track > 9999) {track = 9999;}

  _sendData(DFPLAYER_PLAY_ADVERT_FOLDER, track >> 8, track);
}


/**************************************************************************/
/*
    stopAdvertFolder()

    Stop interrupting current track while playing track from "advert" folder

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
  if (volume <= 30) {_sendData(DFPLAYER_SET_VOL, 0, volume);}
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
    - 1=enable, 0=disable/high resistance
    - DAC is turned on by default after boot or reset
*/
/**************************************************************************/
void DFPlayer::enableDAC(bool enable)
{
  _sendData(DFPLAYER_SET_DAC, 0, !enable); //0=enable, 1=disable/high resistance
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
  if (gain > 31) {gain = 31;}

  _sendData(DFPLAYER_SET_DAC_GAIN, enable, gain);
}


/************************************************************************************/
/*
    setEQ()

    Set equalizer

    NOTE:
    - 0=Off, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass
    - may not be supported by some modules!!!
*/
/************************************************************************************/
void DFPlayer::setEQ(uint8_t preset)
{
  if (preset <= 5) {_sendData(DFPLAYER_SET_EQ, 0, preset);}
}


/**************************************************************************/
/*
    repeatTrack()

    Playing & looping track number in chronological order

    NOTE:
    - command does't work when module is paused or stopped
    - don’t copy 0003.mp3 and then 0001.mp3, because 0003.mp3 will be
      played firts
*/
 /**************************************************************************/
void DFPlayer::repeatTrack(uint16_t track)
{
  if      (track == 0)   {track = 1;}
  else if (track > 9999) {track = 9999;}

  _sendData(DFPLAYER_LOOP_TRACK, track >> 8, track);
}


/**************************************************************************/
/*
    repeatCurrentTrack()

    Loop currently played track

    NOTE:
    - 0=stop repeating, 1=repeat currently played file
    - command does't work when module is paused or stopped
    - any playback command will switch back to normal playback mode
*/
/**************************************************************************/
void DFPlayer::repeatCurrentTrack(bool enable)
{
  _sendData(DFPLAYER_LOOP_CURRENT_TRACK, 0, !enable); //0=repeat, 1=stop repeat
}


/**************************************************************************/
/*
    repeatAll()

    Repeat playback all files in chronological order until it receives
    stop or pause command

    NOTE:
    - 0x00=stop repeat playback & 0x01=start repeat playback
    - command does't work when module is paused or stopped
    - any playback command will switch back to normal playback mode
    - files in the root must start with 4 decimal digits with leading zeros
      - example: SD_ROOT/0001 - My favorite song.mp3
    - don’t copy 0003.mp3 and then 0001.mp3, because 0003.mp3 will be
      played firts
*/
/**************************************************************************/
void DFPlayer::repeatAll(bool enable)
{
  _sendData(DFPLAYER_REPEAT_ALL, 0, enable); //0x00=stop repeat playback & 0x01=start repeat playback
}


/**************************************************************************/
/*
    repeatFolder()

    Repeat playback all tracks in chronological order until stop command

    NOTE:
    - up to 01..99 folder
    - command does't work when module is paused or stopped
    - any playback command will switch back to normal playback mode
    - folder must start with 2 decimal digits with leading zeros
*/
/**************************************************************************/
void DFPlayer::repeatFolder(uint16_t folder)
{
  if      (folder == 0)  {folder = 1;}
  else if (folder >  99) {folder = 99;}

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
    - module does't respond to any playback commands in sleep mode, other
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
    - 1=USB-Disc, 2=TF-Card, 3=Aux, 4=???, 5=NOR-Flash, 6=Sleep (3..6 may
      not be supported by some modules!!!)
*/
/**************************************************************************/
void DFPlayer::wakeup(uint8_t source)
{
  if (source != 6) {setSource(source);} //6=Sleep prohibited
}


/**************************************************************************/
/*
    enableStandby()

    Enable/disable standby or normal mode

    NOTE:
    - true=enable standby, false=normal mode
    - 1=USB-Disc, 2=TF-Card, 3=Aux, 4=???, 5=NOR-Flash, 6=Sleep
    - use wakeup() to exit standby
    - module does't respond to any playback commands in standby mode,
      other commands OK
    - standby not the same as sleep mode
    - looks like does nothing, before & after 24mA
*/
/**************************************************************************/
void DFPlayer::enableStandby(bool enable, uint8_t source)
{
  if (enable == true)
  {
    _sendData(DFPLAYER_SET_STANDBY_MODE, 0, 0); //DFPLAYER_SET_NORMAL_MODE reserved & doesn't work here
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
    - this command does't interrupt current playback
    - status list:
      - 0, stop 
      - 1, playing
      - 2, pause
      - 3, sleep or standby
      - 4, communication error
      - 5, unknown state

    - module response:
    - 7E FF 06 42 00 yy xx zz zz EF
      - yy=01 USB, yy=02 TF
        - xx=00 stop
        - xx=01 playing
        - xx=02 pause
      - yy=00
        - xx=02 standby/sleep
*/
/**************************************************************************/
uint8_t DFPlayer::getStatus()
{
  _sendData(DFPLAYER_GET_STATUS, 0, 0);

  switch (_getResponse(DFPLAYER_GET_STATUS))
  {
    case 0x0200:     
      return 0; //stop 

    case 0x0201:
      return 1; //playing

    case 0x0202:
      return 2; //pause

    case 0x0002:
      return 3; //sleep or standby

    case 0x0000:
      return 4; //communication error

    default:
      return 5; //unknown state
  }
}


/**************************************************************************/
/*
    getVolume()

    Get current volume

    NOTE:
    - volume range 0..30
    - this command does't interrupt current playback
    - return "0" on communication error
*/
/**************************************************************************/
uint8_t DFPlayer::getVolume()
{
  _sendData(DFPLAYER_GET_VOL, 0, 0);

  return _getResponse(DFPLAYER_GET_VOL);
}


/**************************************************************************/
/*
    getEQ()

    Get current EQ

    NOTE:
    - 0=Off, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass
    - this command does't interrupt current playback
    - may not be supported by some modules!!!
    - return "0" on communication error
*/
/**************************************************************************/
uint8_t DFPlayer::getEQ()
{
  _sendData(DFPLAYER_GET_EQ, 0, 0);

  return _getResponse(DFPLAYER_GET_EQ);
}


/**************************************************************************/
/*
    getPlayMode()

    Get current play mode

    NOTE:
    - 0=loop all, 1=loop folder, 2=loop track, 3=random, 4=disable
    - this command does't interrupt current playback
    - may not be supported by some modules!!!
    - return "0" on communication error
*/
/**************************************************************************/
uint8_t DFPlayer::getPlayMode()
{
  _sendData(DFPLAYER_GET_PLAY_MODE, 0, 0);

  return _getResponse(DFPLAYER_GET_PLAY_MODE);
}


/**************************************************************************/
/*
    getVersion()

    Get software version

    NOTE:
    - my module return 0x08
    - this command does't interrupt current playback
    - return "0" on communication error
*/
/**************************************************************************/
uint8_t DFPlayer::getVersion()
{
  _sendData(DFPLAYER_GET_VERSION, 0, 0);

  return _getResponse(DFPLAYER_GET_VERSION);
}


/**************************************************************************/
/*
    getTotalTracksSD()

    Get total number of tracks on TF-Card 

    NOTE:
    - return number even if SD card is removed
    - this command interrupt current playback!!!
    - return "0" on communication error
*/
/**************************************************************************/
uint16_t DFPlayer::getTotalTracksSD()
{
  _sendData(DFPLAYER_GET_QNT_TF_FILES, 0, 0);

  return _getResponse(DFPLAYER_GET_QNT_TF_FILES);
}


/**************************************************************************/
/*
    getTotalTracksUSB()

    Get total number of tracks on USB-Disk

    NOTE:
    - this command interrupt current playback!!!
    - return "0" on communication error
*/
/**************************************************************************/
uint16_t DFPlayer::getTotalTracksUSB()
{
  _sendData(DFPLAYER_GET_QNT_USB_FILES, 0, 0);

  return _getResponse(DFPLAYER_GET_QNT_USB_FILES);
}


/**************************************************************************/
/*
    getTotalTracksNORFlash()

    Get total number of tracks on NOR-Flash

    NOTE:
    - this command interrupt current playback!!!
    - may not be supported by some modules!!!
    - return "0" on communication error
*/
/**************************************************************************/
uint16_t DFPlayer::getTotalTracksNORFlash()
{
  _sendData(DFPLAYER_GET_QNT_FLASH_FILES, 0, 0);

  return _getResponse(DFPLAYER_GET_QNT_FLASH_FILES);
}


/**************************************************************************/
/*
    getTrackSD()

    Get currently playing track number on TF-Card

    NOTE:
    - return track number while track is playing
    - this command does't interrupt current playback
    - return "0" on communication error
*/
/**************************************************************************/
uint16_t DFPlayer::getTrackSD()
{
  _sendData(DFPLAYER_GET_TF_TRACK, 0, 0);

  return _getResponse(DFPLAYER_GET_TF_TRACK);
}


/**************************************************************************/
/*
    getTrackUSB()

    Get currently playing track number on USB-Disk

    NOTE:
    - return track number while track is playing
    - this command does't interrupt current playback
    - return "0" on communication error
*/
/**************************************************************************/
uint16_t DFPlayer::getTrackUSB()
{
  _sendData(DFPLAYER_GET_USB_TRACK, 0, 0);

  return _getResponse(DFPLAYER_GET_USB_TRACK);
}


/**************************************************************************/
/*
    getTrackNORFlash()

    Get currently playing track number on NOR-Flash

    NOTE:
    - may not be supported by some modules!!!
    - return "0" on communication error
*/
/**************************************************************************/
uint16_t DFPlayer::getTrackNORFlash()
{
  _sendData(DFPLAYER_GET_FLASH_TRACK, 0, 0);

  return _getResponse(DFPLAYER_GET_FLASH_TRACK);
}


/**************************************************************************/
/*
    getTotalTracksFolder()

    Get total number of tracks in folder

    NOTE:
    - this command interrupt current playback!!!
    - return "0" on communication error
*/
/**************************************************************************/
uint8_t DFPlayer::getTotalTracksFolder(uint8_t folder)
{
  _sendData(DFPLAYER_GET_QNT_FOLDER_FILES, 0, folder);

  return _getResponse(DFPLAYER_GET_QNT_FOLDER_FILES);
}


/**************************************************************************/
/*
    getTotalFolders()

    Get total number of root folders in current source

    NOTE:
    - only work with USB-Disk & TF-card!!!
    - this command interrupt current playback!!!
    - may not be supported by some modules!!!
    - return "0" on communication error

    - my module doesn't support, 1-st time return 0x04 & 2-nd time 0x07
      than stop play
*/
/**************************************************************************/
uint8_t DFPlayer::getTotalFolders()
{
  _sendData(DFPLAYER_GET_QNT_FOLDERS, 0, 0);

  return _getResponse(DFPLAYER_GET_QNT_FOLDERS);
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
  if (_dataBuffer[3] == DFPLAYER_RETURN_ERROR)      {return _dataBuffer[6];} //see NOTE

  if (_dataBuffer[3] == DFPLAYER_RETURN_CODE_OK)    {return 0x0B;}
  if (_dataBuffer[3] == DFPLAYER_RETURN_CODE_DONE)  {return 0x0C;}
  if (_dataBuffer[3] == DFPLAYER_RETURN_CODE_READY) {return 0x0D;}
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
  _dataBuffer[0] = DFPLAYER_UART_START_BYTE;
  _dataBuffer[1] = DFPLAYER_UART_VERSION;
  _dataBuffer[2] = DFPLAYER_UART_DATA_LEN;
  _dataBuffer[3] = command;
  _dataBuffer[4] = _ack;
  _dataBuffer[5] = dataMSB;
  _dataBuffer[6] = dataLSB;

  int32_t checksum;

  switch (_moduleType)
  {
    case DFPLAYER_MINI:
      checksum = 0;        //0x0000, DON'T TOUCH!!!
      checksum = checksum - _dataBuffer[1] - _dataBuffer[2] - _dataBuffer[3] - _dataBuffer[4] - _dataBuffer[5] - _dataBuffer[6];
      break;

    case DFPLAYER_FN_X10P:
      checksum = 35535;    //0xFFFF, DON'T TOUCH!!!
      checksum = checksum - _dataBuffer[1] - _dataBuffer[2] - _dataBuffer[3] - _dataBuffer[4] - _dataBuffer[5] - _dataBuffer[6] + 1;
      break;

    case DFPLAYER_HW_247A:
    default:
      //empty              //no checksum calculation, not recomended for MCU without external crystal oscillator
      break;
  }

  switch (_moduleType)
  {
    case DFPLAYER_MINI:
    case DFPLAYER_FN_X10P:
      _dataBuffer[7] = checksum >> 8;
      _dataBuffer[8] = checksum;

      _dataBuffer[9] = DFPLAYER_UART_END_BYTE;

      _serial->write(_dataBuffer, DFPLAYER_UART_FRAME_SIZE);
      break;

    case DFPLAYER_HW_247A:
    default:
      _dataBuffer[7] = DFPLAYER_UART_END_BYTE;

      _serial->write(_dataBuffer, (DFPLAYER_UART_FRAME_SIZE - 2)); //-2=SUMH & SUML not used
      break;
  }
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
  memset(_dataBuffer, 0x00, DFPLAYER_UART_FRAME_SIZE);                          //clear data buffer

  _serial->flush();                                                             //clear serial FIFO

  _serial->setTimeout(_threshold);                                              //set maximum msec to wait for Serial.readBytes()

  uint8_t dataSize = _serial->readBytes(_dataBuffer, DFPLAYER_UART_FRAME_SIZE); //read serial, wait for data during setTimeout() period

  if (dataSize < DFPLAYER_UART_FRAME_SIZE)        {return 0;}                   //short answer (received less than expected)

  if (_dataBuffer[0] != DFPLAYER_UART_START_BYTE) {return 1;}                   //start byte missing
  if (_dataBuffer[1] != DFPLAYER_UART_VERSION)    {return 2;}                   //version byte missing
  if (_dataBuffer[2] != DFPLAYER_UART_DATA_LEN)   {return 3;}                   //length byte missing
  if (_dataBuffer[9] != DFPLAYER_UART_END_BYTE)   {return 4;}                   //end byte missing

                                                   return 5;                    //OK, no errors!!!
}


/**************************************************************************/
/*
    _getResponse()

    Get response value from MP3 player

    NOTE:
    - return "0" on communication error
*/
 /**************************************************************************/
uint16_t DFPlayer::_getResponse(uint8_t command)
{
  if ((_readData() == 5) && (_dataBuffer[3] == command)) {return ((uint16_t)_dataBuffer[5] << 8) | _dataBuffer[6];}
                                                          return 0;
}
