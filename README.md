[![license-badge][]][license] ![version] [![stars][]][stargazers] ![hit-count] [![github-issues][]][issues]

# DFPlayer Mini
This is small and fast Arduino library for DFPlayer Mini, FN-M16P & FN-M10P MP3 Player. Supports all modules features.

![alt text][dfplayer_mini_mod_image]

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
- If you hear a loud noise, add a 1K resistor in series with DFPlayer TX-pin.
- Move the jumper from right to left to automatically switch the amplifier to standby.
- Files are in the root must contain 4 digits with leading zeros. For example: SD_ROOT/0001 - My favorite song.mp3. Player sorts the root by the time of writing to the card. Do not copy 0003.mp3 and then 0001.mp3 as 0003.mp3 will play first.
- Folders must contain 2 digits with leading zeros. Number of folders 01..99. Files inside must contain 3 digits with leading zeros. The number of files in each folder is 001..255. For example: SD_ROOT/01/001 - My favorite song.mp3. Unlike the root, files from folders can be read by the file number.
- Folder "mp3" and "advert". Files inside this folders must contain 4 digits with leading zeros. The number of files is 0001..9999 and can be read by file number. For example: SD_ROOT/mp3/ 0001 - My favorite song.mp3. Files from "advert" are played only if a track is already playing. Then the module pauses the current one, plays the file from "advert" and unpauses the main one.


Supports:
- Arduino AVR
- Arduino ESP8266
- Arduino ESP32
- Arduino STM32


[license-badge]: https://img.shields.io/badge/License-GPLv3-blue.svg
[license]:       https://choosealicense.com/licenses/gpl-3.0/
[version]:       https://img.shields.io/badge/Version-1.0.1-green.svg
[stars]:         https://img.shields.io/github/stars/enjoyneering/DFPlayer.svg
[stargazers]:    https://github.com/enjoyneering/DFPlayer/stargazers
[hit-count]:     https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2Fenjoyneering%2FDFPlayer%2Ftree%2Fmain&count_bg=%2379C83D&title_bg=%23555555&icon=&icon_color=%23E7E7E7&title=hits&edge_flat=false
[github-issues]: https://img.shields.io/github/issues/enjoyneering/DFPlayer.svg
[issues]:        https://github.com/enjoyneering/DFPlayer/issues/

[dfplayer_mini_mod_image]: https://github.com/enjoyneering/DFPlayer/blob/main/images/DFPlayer_Mini_Modification.jpg
