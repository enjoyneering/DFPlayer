[![license-badge][]][license] ![version] [![stars][]][stargazers] ![hit-count] [![github-issues][]][issues]

# DFPlayer Mini
This is small and fast Arduino library for:
 - DFPlayer Mini, MP3-TF-16P, FN-M16P (**YX5200** chip, **YX5300** chip or **JL AAxxxx** chip from Jieli)
 - FN-M10P, FN-M10P (**FN6100** chip)
 - DFPlayer Mini HW-247A (**GD3200B** chip)

![alt text][dfplayer_mini_mod_image]

DFPlayer Mini features:
- +3.2v..+5.0v, typical +4.2v
- average curren 24mA
- 24-bit DAC with 90dB output dynamic range and SNR over 85dB
- micro SD-card, up to 32GB (FAT16, FAT32)
- USB-Disk up to 32GB (FAT16, FAT32)
- supports mp3 sampling rate 8KHz, 11.025KHz, 12KHz, 16KHz, 22.05KHz, 24KHz, 32KHz, 44.1KHz, 48KHz
- supports up to 100 folders, each folder can be assigned to 001..255 songs
- built-in 3W mono amplifier, NS8002 AB-Class with standby function
- UART to communicate, 9600bps (parity: none/data bits: 8/stop bits: 1/flow control: none)

NOTE:
- If you hear a loud noise, add a 1K resistors in series with DFPlayer TX-pin & RX-pin.
- Move the jumper from right to left to automatically switch the amplifier to standby.
- Files are in the root must contain 4 digits with leading zeros. For example: SD_ROOT/0001 - My favorite song.mp3. Player sorts the root by the time of writing to the card. Do not copy 0003.mp3 and then 0001.mp3 as 0003.mp3 will play first.
- Folders must contain 2 digits with leading zeros. Number of folders 01..99. Files inside must contain 3 digits with leading zeros. The number of files in each folder is 001..255. For example: SD_ROOT/01/001 - My favorite song.mp3. Unlike the root, files from folders can be read by the file number.
- Folder "mp3" and "advert". Files inside this folders must contain 4 digits with leading zeros. The number of files is 0001..9999 and can be read by file number. For example: SD_ROOT/mp3/ 0001 - My favorite song.mp3. Files from "advert" are played only if a track is already playing. Then the module pauses the current one, plays the file from "advert" and unpauses the main one.

## Library APIs supports all modules features:
```c++
void begin(Stream& stream, uint16_t threshold = 350, DFPLAYER_MODULE_TYPE = DFPLAYER_MINI, bool feedback = false, bool bootDelay = true);

void setModel(DFPLAYER_MODULE_TYPE = DFPLAYER_MINI);
void setTimeout(uint16_t threshold); //usually 200msec..300msec for YX5200/AAxxxx chip & 350msec..500msec for GD3200B/MH2024K chip
void setFeedback(bool enable);

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
void playAdvertFolder(uint8_t folder, uint8_t track); //may not be supported by some modules
void stopAdvertFolder();

void setVolume(uint8_t volume);
void volumeUp();
void volumeDown();
void enableDAC(bool enable); //flase=mute/true=unmute
void setDACGain(uint8_t gain, bool enable = true); //may not be supported by some modules
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
uint8_t  getEQ(); //may not be supported by some modules
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
[version]:       https://img.shields.io/badge/Version-3.0.0-green.svg
[stars]:         https://img.shields.io/github/stars/enjoyneering/DFPlayer.svg
[stargazers]:    https://github.com/enjoyneering/DFPlayer/stargazers
[hit-count]:     https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2Fenjoyneering%2FDFPlayer%2Ftree%2Fmain&count_bg=%2379C83D&title_bg=%23555555&icon=&icon_color=%23E7E7E7&title=hits&edge_flat=false
[github-issues]: https://img.shields.io/github/issues/enjoyneering/DFPlayer.svg
[issues]:        https://github.com/enjoyneering/DFPlayer/issues/

[dfplayer_mini_mod_image]: https://github.com/enjoyneering/DFPlayer/blob/main/images/DFPlayer_Mini_Modification.jpg
