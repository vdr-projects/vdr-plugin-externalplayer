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

#ifndef _EXTERNALPLAYER_CONFIG_H_
#define _EXTERNALPLAYER_CONFIG_H_

#include <string>
#include <list>
#include <fstream>
#include <cstring>

#include <vdr/device.h>
#include <vdr/tools.h>
#include <vdr/keys.h>

using namespace std;

class sKeymap {
private:
    string mKeyMap[k_Setup+1];
public:
    void SetKey (eKeys key, const string &val) {mKeyMap[key] = val;}
    string GetKey (eKeys key) {return mKeyMap[key];}
    sKeymap();
};

struct sPlayerArgs {
    string mMenuEntry;
    string mPlayerCommand;
    ePlayMode mPlayMode;
    bool mSlaveMode;
    bool mDeactivateRemotes;
    bool mBlockMenu;
    sKeymap mKeys;
    sPlayerArgs();
};

struct sConfigEntry {
    string key;
    string value;
};

class FileNotFoundException {
private:
    string mFilename;
public:
    FileNotFoundException(string nFilename) {mFilename = nFilename;}
    string GetFilename(void) {
        return mFilename;
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
