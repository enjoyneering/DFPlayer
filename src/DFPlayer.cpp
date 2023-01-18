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
   - micro SD-card, up to 32GB (FAT16, FAT32)
   - USB-Disk up to 32GB (FAT16, FAT32)
   - supports mp3 sampling rate 8KHz, 11.025KHz, 12KHz, 16KHz, 22.05KHz, 24KHz, 32KHz, 44.1KHz, 48KHz
   - supports up to 100 folders, each folder can be assigned to 001..255 songs
   - built-in 3W mono amplifier, NS8002 AB-Class with standby function
   - UART to communicate, 9600bps (parity:none, data bits:8, stop bits:1, flow control:none)

   NOTE:
   - if you hear a loud noise, add a 1K resistor in series with DFPlayer TX-pin

   Frameworks & Libraries:
   Arduino Core      - https://github.com/arduino/Arduino/tree/master/hardware
   ATtiny  Core      - https://github.com/SpenceKonde/ATTinyCore
   ESP8266 Core      - https://github.com/esp8266/Arduino
   ESP32   Core      - https://github.com/espressif/arduino-esp32
   STM32   Core      - https://github.com/stm32duino/Arduino_Core_STM32
   EspSoftwareSerial - https://github.com/plerup/espsoftwareserial


   GNU GPL license, all text above must be included in any redistribution,
   see link for details  - https://www.gnu.org/licenses/licenses.html
*/
/***************************************************************************************************/

#include "DFPlayer.h"


/**************************************************************************/
/*
    Constructor
*/
/**************************************************************************/
DFPlayer::DFPlayer()
{
  //empty
}


/**************************************************************************/
/*
    begin()

    Class initialization

    NOTE:
    - command feedback timeout 100msec..350msec
    - for "moduleType" see "setModel()" NOTE
    - 0x01=module return feedback after the command, 0x00=module not return feedback
    - wait for player to boot, 1.5sec..3sec depends on SD-card size

    - DAC is turned on by default after boot or reset
    - average consumption 15mA without SD-card, 24mA with SD-card

    - boards boot time:
      - Arduino ATmega328... 2.1sec
      - Arduino SAM21....... 1.7sec
      - ESP8266............. 0.3sec
      - DFPlayer............ 3.0sec
*/
/**************************************************************************/
void DFPlayer::begin(Stream &stream, uint16_t threshold, DFPLAYER_MODULE_TYPE moduleType, bool feedback, bool bootDelay)
{
  _serial     = &stream;    //serial stream
  _threshold  = threshold;  //timeout for feedback (delay after read command), in msec
  _ack        = feedback;   //0x01=module return feedback after the command, 0x00=module not return feedback after the command
  _moduleType = moduleType; //DFPlayer or Clone, differ in how checksum is calculated

  if (bootDelay == true) {delay(DFPLAYER_BOOT_DELAY);} //wait for player to boot
//if (millis() < 6000) {delay(6000 - millis());        //minimum 2100msec + 3000msec = 5100msec, see NOTE
}


/**************************************************************************/
/*
    setModel()

    Set module type, DFPlayer or Clone

    NOTE:
    - moduleType:
      - DFPLAYER_MINI:
        - DFPlayer Mini
        - MP3-TF-16P
        - FN-M16P (for YX5200 chip, YX5300 chip or JL AAxxxx chip from Jieli)
      - DFPLAYER_FN_X10P:
        - FN-M10P
        - FN-S10P (FN6100 chip)
      - DFPLAYER_HW_247A:
        - DFPlayer Mini HW-247A (GD3200B chip)
      - DFPLAYER_NO_CHECKSUM:
        - no checksum calculation (not recomended for MCU without external
          crystal oscillator)

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

    Set feedback timeout (delay after read command), in msec

    NOTE:
    - average feedback timeout 100msec(YX5200/AAxxxx)..350msec(GD3200B/MH2024K)
*/
/**************************************************************************/
void DFPlayer::setTimeout(uint16_t threshold)
{
  _threshold = threshold; //command feedback timeout, in msec
}


/**************************************************************************/
/*
    setFeedback()

    Set whether module should return a feedback after write command

    NOTE:
    - 0x01=module return feedback, 0x00=module not return feedback
*/
/**************************************************************************/
void DFPlayer::setFeedback(bool enable)
{
  _ack = enable; //1=enable feedback, 0=disable feedback
}


/**************************************************************************/
/*
    setSource()

    Set playback source

    NOTE:
    - source:
      - 1=USB-Disk
      - 2=TF-Card
      - 3=Aux
      - 4=sleep (for YX5200)/NOR-Flash (for GD3200)
      - 5=NOR-Flash
      - 6=Sleep
    - source 3..6 may not be supported by some modules!!!

    - module automatically detect source if source is on-line
    - module automatically enter standby after setting source
    - this command interrupt playback!!!
    - wait 200ms to select source
*/
/**************************************************************************/
void DFPlayer::setSource(uint8_t source)
{
  source = constrain(source, 1, 6); //source limit 1..6

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
    - don’t copy 0003.mp3 & then 0001.mp3, because 0003.mp3 will be
      played firts
*/
/**************************************************************************/
void DFPlayer::playTrack(uint16_t track)
{
  track = constrain(track, 1, 9999); //track limit 1..9999

  _sendData(DFPLAYER_PLAY_TRACK, (track >> 8), track);
}


/**************************************************************************/
/*
    next()

    Play next track in chronological order

    NOTE:
    - files in the root must start with 4 decimal digits with leading zeros
      - example: SD_ROOT/0001 - My favorite song.mp3
    - don’t copy 0003.mp3 & then 0001.mp3, because 0003.mp3 will be
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
    - don’t copy 0003.mp3 & then 0001.mp3, because 0003.mp3 will be
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
  folder = constrain(folder, 1, 99); //folder limit 1..99
  track  = constrain(track, 1, 255); //track  limit 1..255

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
  track = constrain(track, 1, 9999); //track limit 1..9999

  _sendData(DFPLAYER_PLAY_MP3_FOLDER, (track >> 8), track);
}


/**************************************************************************/
/*
    play3000Folder()

    Play specific track number from folder, if you need more than 256
    tracks in a folder

    NOTE:
    - folder name must be 01..15
    - up to 0001..3000 songs in each folder
    - feature may not be supported by some modules!!!

    - files in folder must start with 4 decimal digits with leading zeros
      - example: SD_ROOT/01/0001 - My favorite song.mp3
*/
/**************************************************************************/
void DFPlayer::play3000Folder(uint16_t track)
{
  track = constrain(track, 1, 3000); //track limit 1..3000

  _sendData(DFPLAYER_PLAY_3000_FOLDER, (track >> 8), track);
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
  track = constrain(track, 1, 9999); //track limit 1..9999

  _sendData(DFPLAYER_PLAY_ADVERT_FOLDER, (track >> 8), track);
}


/**************************************************************************/
/*
    playAdvertFolder()

    Interrupt current track & play specific track number from specific
    "advert1".."advert9" folder, than resume current track

    NOTE:
    - folder name must be "advert1".."advert9" or  or "ADVERT1".."ADVERT9"
    - up to 001..255 songs in each folder
    - feature may not be supported by some modules!!!

    - files in folder must start with 3 decimal digits with leading zeros
      - example: SD_ROOT/advert1/001 - My favorite song.mp3
*/
/**************************************************************************/
void DFPlayer::playAdvertFolder(uint8_t folder, uint8_t track)
{
  folder = constrain(folder, 1, 9);  //folder limit 1..9
  track  = constrain(track, 1, 255); //track  limit 1..255

  _sendData(DFPLAYER_PLAY_ADVERT_FOLDER_N, folder, track);
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
  volume = constrain(volume, 0, 30); //volume limit 0..30

  _sendData(DFPLAYER_SET_VOL, 0, volume);
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

    Enable/disable DAC, aka mute/unmute

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

    - feature may not be supported by some modules!!!
*/
/**************************************************************************/
void DFPlayer::setDACGain(uint8_t gain, bool enable)
{
  gain = constrain(gain, 0, 31); //gain limit 0..31

  _sendData(DFPLAYER_SET_DAC_GAIN, enable, gain);
}


/************************************************************************************/
/*
    setEQ()

    Set equalizer

    NOTE:
    - 0=Off, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass
    - feature may not be supported by some modules!!!
*/
/************************************************************************************/
void DFPlayer::setEQ(uint8_t preset)
{
  preset = constrain(preset, 0, 5); //preset limit 0..5

  _sendData(DFPLAYER_SET_EQ, 0, preset);
}


/**************************************************************************/
/*
    repeatTrack()

    Playing & looping track number in chronological order

    NOTE:
    - command does't work when module is paused or stopped

    - don’t copy 0003.mp3 & then 0001.mp3, because 0003.mp3 will be
      played firts
*/
 /**************************************************************************/
void DFPlayer::repeatTrack(uint16_t track)
{
  track = constrain(track, 1, 9999); //track limit 1..9999

  _sendData(DFPLAYER_LOOP_TRACK, (track >> 8), track);
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
    - don’t copy 0003.mp3 & then 0001.mp3, because 0003.mp3 will be
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
  folder = constrain(folder, 1, 99); //folder limit 1..99

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
    - use "wakeup()" to exit sleep
    - module does't respond to any playback commands in sleep mode, other
      commands OK
    - looks like does nothing, consumption before & after command 24mA

    - source:
      - 1=USB-Disk
      - 2=TF-Card
      - 3=Aux
      - 4=sleep (for YX5200)/NOR-Flash (for GD3200)
      - 5=NOR-Flash
      - 6=Sleep
    - source 3..6 may not be supported by some modules!!!
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
    - source:
      - 1=USB-Disk
      - 2=TF-Card
      - 3=Aux
      - 4=sleep (for YX5200)/NOR-Flash (for GD3200)
      - 5=NOR-Flash
      - 6=Sleep
    - source 3..6 may not be supported by some modules!!!
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
    - standby not the same as sleep mode
    - use "wakeup()" to exit standby
    - module does't respond to any playback commands in standby mode,
      other commands OK
    - looks like does nothing, consumption before & after command 24mA

    - source:
      - 1=USB-Disk
      - 2=TF-Card
      - 3=Aux
      - 4=sleep (for YX5200)/NOR-Flash (for GD3200)
      - 5=NOR-Flash
      - 6=Sleep
    - source 3..6 may not be supported by some modules!!!
*/
/**************************************************************************/
void DFPlayer::enableStandby(bool enable, uint8_t source)
{
  if (enable == true)
  {
    _sendData(DFPLAYER_SET_STANDBY_MODE, 0, 0);
  }
  else
  {
   wakeup(source); //for some reason "DFPLAYER_SET_NORMAL_MODE" doesn't work here
  }
}


/**************************************************************************/
/*
    reset()

    Reset all settings to factory default

    NOTE:
    - wait for player to boot, 1.5sec..3sec depends on SD-card size
*/
/**************************************************************************/
void DFPlayer::reset()
{
  _sendData(DFPLAYER_RESET, 0, 0);

  delay(DFPLAYER_BOOT_DELAY); //wait for player to boot
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
      - 3, sleep, standby
      - 4, communication error
      - 5, unknown state

    - module response:
    - 7E FF 06 42 00 yy xx zz ww EF
      - yy=01 USB, yy=02 TF-card (for YX5200 chip, YX5300 chip or JL AAxxxx chip from Jieli only)
        - xx=00 stop 
        - xx=01 playing
        - xx=02 pause
      - yy=00
        - xx=00 stop (for GD3200B chip only)
        - xx=01 playing (for GD3200B chip only)
        - xx=02 standby/sleep
*/
/**************************************************************************/
uint8_t DFPlayer::getStatus()
{
  _sendData(DFPLAYER_GET_STATUS, 0, 0);

  switch (_getResponse(DFPLAYER_GET_STATUS))
  {
    case 0x0200:    
      return 0; //TF-card stop

    case 0x0201:
      return 1; //TF-card playing

    case 0x0202:
      return 2; //TF-card pause

    case 0x0002:
      return 3; //sleep or standby

    case 0x0001:
      return ((_moduleType != DFPLAYER_HW_247A) ? 5 : 1); //5=unknown state, 1=GD3200B playing

    case 0x0000:
      return ((_moduleType != DFPLAYER_HW_247A) ? 4 : 0); //4=communication error, 0=GD3200B stop

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
    - feature may not be supported by some modules!!!
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
    - feature may not be supported by some modules!!!
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
    - my JL AAxxxx chip return 0x08
    - my GD3200B chip return
      - 0A 77 77 77 2E 67 64 6B 65 6A 69 2E 63 6F 6D 0A  .www.gdkeji.com.
        47 44 33 32 30 30 42 2D 56 34 2E 30 0A 32 30 32  GD3200B-V4.0.202
        30 2D 31 31 2D 32 35 20 32 32 3A 30 31 0A        0-11-25 22:01. 

    - this command does't interrupt current playback
    - return "0" on communication error
*/
/**************************************************************************/
uint8_t DFPlayer::getVersion()
{
  _sendData(DFPLAYER_GET_VERSION, 0, 0);

  return _getResponse(DFPLAYER_GET_VERSION); //TODO: parse GD3200B version
}


/**************************************************************************/
/*
    getTotalTracksSD()

    Get total number of tracks on TF-Card 

    NOTE:
    - return number even if SD-card is removed
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
    - feature may not be supported by some modules!!!
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
    - feature may not be supported by some modules!!!
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
    - feature may not be supported by some modules!!!
    - return "0" on communication error

    - my module doesn't support this command, 1-st time it returns 0x04
      & 2-nd time 0x07 than stop play
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

    Get player status after last TX command

    NOTE:
    - module returned codes at the end of any playback operation or if any
      command error

    - error values:
      - 0x01, error module busy (this info is returned when the initialization is not done)
      - 0x02, error module in sleep mode (supports only specified device in sleep mode)
      - 0x03, error serial receiving error (request not fully received)
      - 0x04, error checksum incorrect
      - 0x05, error requested rack/folder is out of out of range
      - 0x06, error requested track/folder is not found
      - 0x07, error advert insertion error (inserting operation available when a track is being played)
      - 0x08, error SD-card reading failed (SD-card pulled out or damaged)
      - 0x09, error ???
      - 0x0A, error entered sleep mode

    - extra values:
      - 0x0B, OK, command is accepted (returned only if ACK/feedback byte is set to 0x01)
      - 0x0C, OK, track playback is completed, module return this status automatically after the track has been played
      - 0x0D, OK, ready after boot or reset with DL-byte current source???, module return this status automatically after boot or reset
      - 0x00, unknown status
*/
/**************************************************************************/
uint8_t DFPlayer::getCommandStatus()
{
  switch (_dataBuffer[3])
  {
    case DFPLAYER_RETURN_ERROR:
      return _dataBuffer[6]; //error values, see NOTE

    case DFPLAYER_RETURN_CODE_OK_ACK:
      return 0x0B;

    case DFPLAYER_RETURN_CODE_DONE:
      return 0x0C;

    case DFPLAYER_RETURN_CODE_READY:
      return 0x0D;

    default:
      return 0x00;
  }
}


/**********************************private*********************************/
/**************************************************************************/
/*
    _sendData()

    Send data via Serial port

    NOTE:
    - DFPlayer TX data frame format:
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

  int16_t checksum;

  switch (_moduleType)
  {
    case DFPLAYER_MINI:
    case DFPLAYER_HW_247A:
      checksum = 0;        //0x0000, DON'T TOUCH!!!
      checksum = checksum - _dataBuffer[1] - _dataBuffer[2] - _dataBuffer[3] - _dataBuffer[4] - _dataBuffer[5] - _dataBuffer[6];
      break;

    case DFPLAYER_FN_X10P:
      checksum = 35535;    //0xFFFF, DON'T TOUCH!!!
      checksum = checksum - _dataBuffer[1] - _dataBuffer[2] - _dataBuffer[3] - _dataBuffer[4] - _dataBuffer[5] - _dataBuffer[6] + 1;
      break;

    case DFPLAYER_NO_CHECKSUM:
    default:
      //empty              //no checksum calculation, not recomended for MCU without external crystal oscillator
      break;
  }

  switch (_moduleType)
  {
    case DFPLAYER_MINI:
    case DFPLAYER_FN_X10P:
    case DFPLAYER_HW_247A:
      _dataBuffer[7] = checksum >> 8;
      _dataBuffer[8] = checksum;

      _dataBuffer[9] = DFPLAYER_UART_END_BYTE;

      _serial->write(_dataBuffer, DFPLAYER_UART_FRAME_SIZE);

      if (_moduleType == DFPLAYER_HW_247A) {delay(_threshold);}    //GD3200B/MH2024K chip so slow & need delay after write command
      break;

    case DFPLAYER_NO_CHECKSUM:
    default:
      _dataBuffer[7] = DFPLAYER_UART_END_BYTE;

      _serial->write(_dataBuffer, (DFPLAYER_UART_FRAME_SIZE - 2)); //-2=SUMH & SUML not used
      break;
  }
}


/**************************************************************************/
/*
    _readData()

    Read MP3 player command feedback

    NOTE:
    - command feedback timeout 100msec(YX5200/AAxxxx)..350msec(GD3200B/MH2024K)

    - DFPlayer RX data frame format:
      0      1    2    3    4    5   6   7     8     9-byte
      START, VER, LEN, CMD, ACK, DH, DL, SUMH, SUML, END
*/
 /**************************************************************************/
bool DFPlayer::_readData()
{
  memset(_dataBuffer, 0x00, DFPLAYER_UART_FRAME_SIZE); //clear data buffer

  _serial->flush();                                    //clear serial FIFO
  _serial->setTimeout(_threshold);                     //set maximum msec to wait for "readBytes()", delay after read command

  /* read serial, wait for data during "setTimeout()" period than error (received less than expected) */
  if (_serial->readBytes(_dataBuffer, DFPLAYER_UART_FRAME_SIZE) != DFPLAYER_UART_FRAME_SIZE) {return false;}

  /* check for start byte missing, version byte missing, length byte missing, end byte missing */
  return ((_dataBuffer[0] == DFPLAYER_UART_START_BYTE) && (_dataBuffer[1] == DFPLAYER_UART_VERSION) && (_dataBuffer[2] == DFPLAYER_UART_DATA_LEN) && (_dataBuffer[9] == DFPLAYER_UART_END_BYTE));
}


/**************************************************************************/
/*
    _getResponse()

    Get command feedback value from MP3 player

    NOTE:
    - return "0" on communication error
*/
 /**************************************************************************/
uint16_t DFPlayer::_getResponse(uint8_t command)
{
  if ((_readData() == true) && (_dataBuffer[3] == command)) {return ((uint16_t)_dataBuffer[5] << 8) | _dataBuffer[6];} //return DH, DL
                                                             return 0;
}
