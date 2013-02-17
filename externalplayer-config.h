/*
 * externalplayer-config.h: A plugin for the Video Disk Recorder
 *
 * Initially written by Felix Hädicke
 *
 * 2013 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 */

#include <string>
#include <list>
#include <fstream>
#include <cstring>

#include <vdr/device.h>
#include <vdr/tools.h>

#ifndef _EXTERNALPLAYER_CONFIG_H_
#define _EXTERNALPLAYER_CONFIG_H_

using namespace std;

struct sKeymap {
    string * vdrKeyUp;
    string * vdrKeyDown;
    string * vdrKeyLeft;
    string * vdrKeyRight;
    string * vdrKeyOk;
    string * vdrKeyBack;
    string * vdrKeyRed;
    string * vdrKeyGreen;
    string * vdrKeyYellow;
    string * vdrKeyBlue;
    string * vdrKey0;
    string * vdrKey1;
    string * vdrKey2;
    string * vdrKey3;
    string * vdrKey4;
    string * vdrKey5;
    string * vdrKey6;
    string * vdrKey7;
    string * vdrKey8;
    string * vdrKey9;
    string * vdrKeyPlay;
    string * vdrKeyPause;
    string * vdrKeyStop;
    string * vdrKeyRecord;
    string * vdrKeyFastFwd;
    string * vdrKeyFaswRew;
    string * vdrKeyChannelUp;
    string * vdrKeyChannelDown;
    string * vdrKeyAudio;
    string * vdrKeySchedule;
    string * vdrKeyChannels;
    string * vdrKeyTimers;
    string * vdrKeyRecordings;
    string * vdrKeySetup;
    string * vdrKeyCommands;
    string * vdrKeyUser1;
    string * vdrKeyUser2;
    string * vdrKeyUser3;
    string * vdrKeyUser4;
    string * vdrKeyUser5;
    string * vdrKeyUser6;
    string * vdrKeyUser7;
    string * vdrKeyUser8;
    string * vdrKeyUser9;
    sKeymap();
    ~sKeymap();
};

struct sPlayerArgs {
    string menuEntry;
    string playerCommand;
    ePlayMode playMode;
    bool slaveMode;
    bool deactivateRemotes;
    bool blockMenu;
    sKeymap * keys;
    sPlayerArgs();
    ~sPlayerArgs();
};

struct sConfigEntry {
    string key;
    string value;
};

class FileNotFoundException {
private:
    string filename;
public:
    FileNotFoundException(string nFilename);
    string GetFilename(void) {
        return filename;
    }
};

class SyntaxErrorException {
private:
    int charNumber;
    string * configFileContent;
public:
    SyntaxErrorException(int nCharNumber, string * nConfigFileContent);
    int GetLineNumber(void);
    int GetColumnNumber(void);
};

class EntryMissingException {
private:
    string playerCommand;
    string menuEntry;
    int charNumber;
    string * configFileContent;
public:
    EntryMissingException(string nPlayerCommand, string nMenuEntry,
                          int nCharNumber, string * nConfigFileContent);
    string GetPlayerCommand(void) {
        return playerCommand;
    }
    string GetMenuEntry(void) {
        return menuEntry;
    }
    int GetLineNumber(void);
};

class InvalidKeywordException {
private:
    string keyword;
    int charNumber;
    string *configFileContent;
public:
    InvalidKeywordException(string nKeyword, int nCharNumber,
                            string *nConfigFileContent);
    string GetKeyword(void) {
        return keyword;
    }
    int GetLineNumber(void);
};

class cExternalplayerConfig {
private:
    string *configFileContent;
    list<sPlayerArgs *> configuration;
    string *ReadConfigFile(string filename);
    list<sPlayerArgs *> ParseConfigFile(void);
    sPlayerArgs *GetConfiguration(unsigned int *position);
    sConfigEntry GetConfigEntry(unsigned int *position);
    void RemoveUnnecessarySymbols(string *stringPtr);
    void ProcessConfigEntry(sPlayerArgs *args, sConfigEntry entry,
                            int position);
    string *GetCodeSpecialKey(string name);
    unsigned int GetLineNumberOfChar(unsigned int charNumber);
    unsigned int GetColumnNumberOfChar(unsigned int charNumber);
public:
    cExternalplayerConfig(string filename);
    ~cExternalplayerConfig();
    list<sPlayerArgs *> GetConfiguration(void) {
        return configuration;
    }
    int PlayerCount(void) {
        return configuration.size();
    }
};

#endif /*_EXTERNALPLAYER_CONFIG_H_*/
