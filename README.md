[![license-badge][]][license] ![version] [![stars][]][stargazers] ![hit-count] [![github-issues][]][issues]

# DFPlayer Mini
This is small and fast Arduino library for:
 - DFPlayer Mini, MP3-TF-16P, FN-M16P (YX5200, YX5300, JL AA20HF)
 - FN-M10P, FN-M10P (FN6100)
 - DFPlayer Mini HW-247A, MP3-TF-16P V3.0 (GD3200B, MH2024K)

![alt text][dfplayer_mini_mod_image]

DFPlayer Mini features:
- 3.2v..5.0v, typical 4.2v
- average curren 24mA
- 24-bit DAC
- 90dB output dynamic range
- SNR over 85dB
- micro SD, card up to 32GB (FAT16, FAT32)
- USB-Disk, up to 32GB (FAT16, FAT32)
- supports up to 100 folders, each folder can be assigned to 001..255 songs
- built-in 3W mono amplifier, MD8002 AB-Class
- UART to communicate

NOTE:
- If you hear a loud noise, add a 1K resistor in series with DFPlayer TX-pin.
- Move the jumper from right to left to automatically switch the amplifier to standby.
- Files are in the root must contain 4 digits with leading zeros. For example: SD_ROOT/0001 - My favorite song.mp3. Player sorts the root by the time of writing to the card. Do not copy 0003.mp3 and then 0001.mp3 as 0003.mp3 will play first.
- Folders must contain 2 digits with leading zeros. Number of folders 01..99. Files inside must contain 3 digits with leading zeros. The number of files in each folder is 001..255. For example: SD_ROOT/01/001 - My favorite song.mp3. Unlike the root, files from folders can be read by the file number.
- Folder "mp3" and "advert". Files inside this folders must contain 4 digits with leading zeros. The number of files is 0001..9999 and can be read by file number. For example: SD_ROOT/mp3/ 0001 - My favorite song.mp3. Files from "advert" are played only if a track is already playing. Then the module pauses the current one, plays the file from "advert" and unpauses the main one.

## Library APIs supports all modules features:
```c++
void begin(Stream& stream, uint16_t threshold = 100, DFPLAYER_MODULE_TYPE = DFPLAYER_MINI, bool response = false, bool bootDelay = true);

void setModel(DFPLAYER_MODULE_TYPE = DFPLAYER_MINI);
void setTimeout(uint16_t threshold); //usually 100msec..200msec
void setResponse(bool enable);

void setSource(uint8_t source); //all sources may not be supported by some modules
void playTrack(uint16_t track);
void next();
void previous();
void pause();
void resume();
void stop();

void playFolder(uint8_t folder, uint8_t track);
void playMP3Folder(uint16_t track);
void play3000Folder(uint16_t track); //may not be supported by some modules
void playAdvertFolder(uint16_t track);
void stopAdvertFolder();

void setVolume(uint8_t volume);
void volumeUp();
void volumeDown();
void enableDAC(bool enable);
void setDACGain(uint8_t gain, bool enable = true);
void setEQ(uint8_t preset); //may not be supported by some modules

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
uint8_t  getPlayMode(); //may not be supported by some modules
uint8_t  getVersion();
uint16_t getTotalTracksSD();
uint16_t getTotalTracksUSB();
uint16_t getTotalTracksNORFlash(); //may not be supported by some modules
uint16_t getTrackSD();
uint16_t getTrackUSB();
uint16_t getTrackNORFlash(); //may not be supported by some modules
uint8_t  getTotalTracksFolder(uint8_t folder);
uint8_t  getTotalFolders(); //may not be supported by some modules
uint8_t  getCommandStatus();
```

Supports:
- Arduino AVR
- Arduino ESP8266
- Arduino ESP32
- Arduino STM32


[license-badge]: https://img.shields.io/badge/License-GPLv3-blue.svg
[license]:       https://choosealicense.com/licenses/gpl-3.0/
[version]:       https://img.shields.io/badge/Version-2.0.0-green.svg
[stars]:         https://img.shields.io/github/stars/enjoyneering/DFPlayer.svg
[stargazers]:    https://github.com/enjoyneering/DFPlayer/stargazers
[hit-count]:     https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2Fenjoyneering%2FDFPlayer%2Ftree%2Fmain&count_bg=%2379C83D&title_bg=%23555555&icon=&icon_color=%23E7E7E7&title=hits&edge_flat=false
[github-issues]: https://img.shields.io/github/issues/enjoyneering/DFPlayer.svg
[issues]:        https://github.com/enjoyneering/DFPlayer/issues/

[dfplayer_mini_mod_image]: https://github.com/enjoyneering/DFPlayer/blob/main/images/DFPlayer_Mini_Modification.jpg
