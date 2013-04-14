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
#include <vector>
#include <stdexcept>
#include <fstream>
#include <cstring>

#include <vdr/device.h>
#include <vdr/tools.h>
#include <vdr/keys.h>

using namespace std;

class StringTool {
public:
    static int strcasecmp (const string &s1, const string &s2) {
        return (::strcasecmp (s1.c_str(), s2.c_str()) == 0);
    }
};

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

typedef vector<sPlayerArgs *> sPlayerArgsList;

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
    int mCharNumber;
    string mConfigFileContent;
public:
    SyntaxErrorException(int nCharNumber, const string &nConfigFileContent);
    int GetLineNumber(void);
    int GetColumnNumber(void);
};

class EntryMissingException {
private:
    string mPlayerCommand;
    string mMenuEntry;
    int mCharNumber;
    string mConfigFileContent;
public:
    EntryMissingException(const string &nPlayerCommand, const string &nMenuEntry,
                          int nCharNumber, const string &nConfigFileContent);
    string GetPlayerCommand(void) {
        return mPlayerCommand;
    }
    string GetMenuEntry(void) {
        return mMenuEntry;
    }
    int GetLineNumber(void);
};

class InvalidKeywordException {
private:
    string mKeyword;
    int mCharNumber;
    string mConfigFileContent;
public:
    InvalidKeywordException(const string &nKeyword, int nCharNumber,
                            const string &nConfigFileContent);
    string GetKeyword(void) {return mKeyword;}
    int GetLineNumber(void);
};

class cExternalplayerConfig {
private:
    string mConfigFileContent;
    sPlayerArgsList configuration;
    string ReadConfigFile(const string &filename);
    sPlayerArgsList ParseConfigFile(void);
    sPlayerArgs *GetConfiguration(unsigned int *position);
    sConfigEntry GetConfigEntry(unsigned int *position);
    void RemoveUnnecessarySymbols(string &stringPtr);
    void ProcessConfigEntry(sPlayerArgs *args, sConfigEntry entry,
                            int position);
    string *GetCodeSpecialKey(string name);
    unsigned int GetLineNumberOfChar(unsigned int charNumber);
    unsigned int GetColumnNumberOfChar(unsigned int charNumber);
public:
    cExternalplayerConfig(string filename);
    ~cExternalplayerConfig();
    sPlayerArgsList GetConfiguration(void) {
        return configuration;
    }
    sPlayerArgs *GetConfiguration(int cnt) throw (std::out_of_range) {
        return (configuration.at(cnt));
    }
    int PlayerCount(void) {
        return configuration.size();
    }
};

#endif /*_EXTERNALPLAYER_CONFIG_H_*/
