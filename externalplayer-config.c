/*
 * externalplayer-config.c: A plugin for the Video Disk Recorder
 *
  * Initially written by Felix Hädicke
 *
 * 2013 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#include "externalplayer-config.h"

sKeymap::sKeymap() {
    mKeyMap[kUp] = "\e[A";
    mKeyMap[kDown] = "\e[B";
    mKeyMap[kLeft] = "\e[D";
    mKeyMap[kRight] = "\e[C";

    mKeyMap[k0] = "0";
    mKeyMap[k1] = "1";
    mKeyMap[k2] = "2";
    mKeyMap[k3] = "3";
    mKeyMap[k4] = "4";
    mKeyMap[k5] = "5";
    mKeyMap[k6] = "6";
    mKeyMap[k7] = "7";
    mKeyMap[k8] = "8";
    mKeyMap[k9] = "9";
}

// --- sPlayerArgs ----------------------------------------------------------

sPlayerArgs::sPlayerArgs() {
  mMenuEntry = "";
  mPlayerCommand = "";
  mSlaveMode = false;
  mPlayMode = pmExtern_THIS_SHOULD_BE_AVOIDED;
  mDeactivateRemotes = false;
  mBlockMenu = false;
}

// --- SyntaxErrorException -------------------------------------------------

SyntaxErrorException::SyntaxErrorException(int nCharNumber, const string &nConfigFileContent) {
  mCharNumber = nCharNumber;
  mConfigFileContent = nConfigFileContent;
}

int SyntaxErrorException::GetLineNumber() {
  int lineNumber = 0;
  for (int i = 0; i < mCharNumber; i++) {
    if (mConfigFileContent[i] == '\n') {
      lineNumber++;
    }
  }
  return lineNumber;
}

int SyntaxErrorException::GetColumnNumber() {
  int columnNumber = 1;
  for (int i = 0; i < mCharNumber; i++) {
    if (mConfigFileContent[i] == '\n') {
      columnNumber = 1;
    }
    else {
      columnNumber++;
    }
  }
  return columnNumber;
}

// --- EntryMissingException -------------------------------------------------

EntryMissingException::EntryMissingException(const string &nPlayerCommand, const string &nMenuEntry,
                                             int nCharNumber, const string &nConfigFileContent) {
  mPlayerCommand = nPlayerCommand;
  mMenuEntry = nMenuEntry;
  mCharNumber = nCharNumber;
  mConfigFileContent = nConfigFileContent;
}

int EntryMissingException::GetLineNumber() {
  int lineNumber = 0;
  for (int i = 0; i < mCharNumber; i++) {
    if (mConfigFileContent[i] == '\n') {
      lineNumber++;
    }
  }
  return lineNumber;
}

// --- InvalidKeywordException -----------------------------------------------

InvalidKeywordException::InvalidKeywordException(const string &nKeyword, int nCharNumber,
                                                 const string &nConfigFileContent) {
  mKeyword = nKeyword;
  mCharNumber = nCharNumber;
  mConfigFileContent = nConfigFileContent;
}

int InvalidKeywordException::GetLineNumber() {
  int lineNumber = 0;
  for (int i = 0; i < mCharNumber; i++) {
    if (mConfigFileContent[i] == '\n') {
      lineNumber++;
    }
  }
  return lineNumber;
}

// --- cExternalplayerConfig -------------------------------------------------

cExternalplayerConfig::cExternalplayerConfig(string filename) {
  try {
    mConfigFileContent = ReadConfigFile(filename);
    configuration = ParseConfigFile();
  }
  catch (FileNotFoundException &fnfEx) {
    mConfigFileContent.clear();
    esyslog("externalplayer-plugin: Configuration file \"%s\" not found!\n", fnfEx.GetFilename().c_str());
  }
}

cExternalplayerConfig::~cExternalplayerConfig() {
  while (!configuration.empty()) {
    delete configuration.back();
    configuration.pop_back();
  }
}

string cExternalplayerConfig::ReadConfigFile(const string &filename) {
  ifstream playerConfigStream;
  playerConfigStream.open(filename.c_str(), ios::in);

  if (playerConfigStream == NULL) {
    throw FileNotFoundException(filename);
  }

  string configFileContent;
  char buffer[256];

  while (!playerConfigStream.eof()) {
    playerConfigStream.getline(buffer, 256);
    configFileContent = configFileContent + '\n' + buffer;
  }
  configFileContent = configFileContent + '\n';

  playerConfigStream.close();

  return configFileContent;
}

sPlayerArgsList cExternalplayerConfig::ParseConfigFile() {
  sPlayerArgsList configuration;
  sPlayerArgs * playerConfig = NULL;

  for (unsigned int i = 0; i < mConfigFileContent.size(); i++) {
    switch (mConfigFileContent[i]) {
      case ' ':
      case '\t':
      case '\n':
        break;
      case '#':
        while (mConfigFileContent[i] != '\n') {
          i++;
        }
        break;
      case '{':
        playerConfig = new sPlayerArgs();
        i++;
        try {
          playerConfig = GetConfiguration(&i);
          configuration.push_back(playerConfig);
        }
        catch (EntryMissingException &emEx) {
          if (emEx.GetMenuEntry() == "") {
            esyslog("externalplayer-plugin: error in config file: \"MenuEntry\" missing or invalid, line %i",
                    emEx.GetLineNumber());
          }
          if (emEx.GetPlayerCommand() == "") {
            esyslog("externalplayer-plugin: error in config file: \"Command\" missing or invalid, line %i!\n",
                    emEx.GetLineNumber());
          }
        }
        break;
      default:
        unsigned int errorPosition = i;
        while (mConfigFileContent[i] != '\n') {
          i++;
        }
        i++;
        esyslog("externalplayer-plugin: syntax error in config file: line %i, column %i! Ignoring rest of this line.",
                GetLineNumberOfChar(errorPosition), GetColumnNumberOfChar(errorPosition));
        break;
    }
  }

  return configuration;
}

sPlayerArgs * cExternalplayerConfig::GetConfiguration(unsigned int * position) {
  sPlayerArgs * args = new sPlayerArgs();

  bool endOfFile = false;
  while (mConfigFileContent[*position] != '}' && !endOfFile) {
    switch (mConfigFileContent[*position]) {
      case ' ':
      case '\t':
      case '\n':
        (*position)++;
        break;
      case '#':
        while (mConfigFileContent[*position] != '\n') {
          (*position)++;
        }
        (*position)++;
        break;
      default:
        if (*position >= mConfigFileContent.size()) {
          esyslog("externalplayer-plugin: no \"}\" at end of file!");
          endOfFile = true;
        }
        else {
          try {
            sConfigEntry entry = GetConfigEntry(position);
            dsyslog ("Entry %s, %s", entry.key.c_str(), entry.value.c_str());
            ProcessConfigEntry(args, entry, *position);
          }
          catch(SyntaxErrorException &seEx) {
            esyslog("externalplayer-plugin: syntax error in config file: line %i, column %i! Ignoring entry.",
                    seEx.GetLineNumber(), seEx.GetColumnNumber());
          }
          catch (InvalidKeywordException &ikEx) {
            esyslog("externalplayer-plugin: error in config file: invalig keyword \"%s\" line %i!",
                  ikEx.GetKeyword().c_str(), ikEx.GetLineNumber());
          }
        }
        break;
    }
  }

  if ((args->mPlayerCommand == "") || (args->mMenuEntry == "")) {
    throw EntryMissingException(args->mPlayerCommand, args->mMenuEntry, *position, mConfigFileContent);
  }

  return args;
}

sConfigEntry cExternalplayerConfig::GetConfigEntry(unsigned int * position)
{
  sConfigEntry entry;

  while (mConfigFileContent[*position] != '=') {
    switch(mConfigFileContent[*position]) {
      case '{':
      case '}':
      case '\n':
      case '\"':
      case ';':
        {
          unsigned int errorPosition = *position;
          (*position)++;
          while ((mConfigFileContent[*position] != '\n') && (mConfigFileContent[*position] != ';') &&
                 (mConfigFileContent[*position] != '{') && (mConfigFileContent[*position] != '}')) {
            (*position)++;
          }
          if (mConfigFileContent[*position] != '{' && mConfigFileContent[*position] != '}') {
            (*position)++;
          }
          throw SyntaxErrorException(errorPosition, mConfigFileContent);
        }
        break;
      case '#':
        {
          unsigned int errorPosition = *position;
          (*position)++;
          while ((mConfigFileContent[*position] != '\n') && (mConfigFileContent[*position] != ';') &&
                 (mConfigFileContent[*position] != '{') && (mConfigFileContent[*position] != '}')) {
            (*position)++;
          }
          if ((mConfigFileContent[*position] != '{') && (mConfigFileContent[*position] != '}')) {
            (*position)++;
          }
          throw SyntaxErrorException(errorPosition, mConfigFileContent);
        }
        break;
      default:
        entry.key += mConfigFileContent[*position];
        (*position)++;
        break;
    }
  }
  (*position)++;

  while (mConfigFileContent[*position] != ';') {
    switch(mConfigFileContent[*position]) {
      case '{':
      case '}':
      case '\n':
      case '=':
        {
          unsigned int errorPosition = *position;
          (*position)++;
          while ((mConfigFileContent[*position] != '\n') && (mConfigFileContent[*position] != ';') &&
                 (mConfigFileContent[*position] != '{') && (mConfigFileContent[*position] != '}')) {
            (*position)++;
          }
          if ((mConfigFileContent[*position] != '{') && (mConfigFileContent[*position] != '}')) {
            (*position)++;
          }
          throw SyntaxErrorException(errorPosition, mConfigFileContent);
        }
      break;
      case '\"':
        while (mConfigFileContent[*position] != '\"') {
          (*position)++;
          if (mConfigFileContent[*position] == '\n') {
            throw SyntaxErrorException(*position, mConfigFileContent);
          }
          entry.value += mConfigFileContent[*position];
        }
        (*position)++;
        break;
      case '#':
        {
          int errorPosition = *position;
          (*position)++;
          while ((mConfigFileContent[*position] != '\n') && (mConfigFileContent[*position] != ';') &&
                 (mConfigFileContent[*position] != '{') && (mConfigFileContent[*position] != '}')) {
            (*position)++;
          }
          if ((mConfigFileContent[*position] != '{') && (mConfigFileContent[*position] != '}')) {
            (*position)++;
          }
          throw SyntaxErrorException(errorPosition, mConfigFileContent);
        }
        break;
      default:
        entry.value += mConfigFileContent[*position];
        (*position)++;
        break;
    }
  }
  (*position)++;

  RemoveUnnecessarySymbols(entry.key);
  RemoveUnnecessarySymbols(entry.value);

  return entry;
}

void cExternalplayerConfig::RemoveUnnecessarySymbols(string &stringPtr) {
  while ((stringPtr[0] == ' ') || (stringPtr[0] == '\t')){
    stringPtr.erase(stringPtr.begin());
  }

  while ((stringPtr[stringPtr.size() - 1] == ' ') || (stringPtr[stringPtr.size() - 1] == '\t')){
    stringPtr.erase((stringPtr.end() - 1), (stringPtr.end()));
  }
}

void cExternalplayerConfig::ProcessConfigEntry(sPlayerArgs *args, sConfigEntry entry, int position) {
    cKey keys;
    bool found = true;

    if (entry.key.empty() || entry.value.empty()) {
        throw SyntaxErrorException(position, mConfigFileContent);
    }

    if (StringTool::strcasecmp (entry.key, "Command")) {
        args->mPlayerCommand = entry.value;
    }
    else if (StringTool::strcasecmp (entry.key, "MenuEntry")) {
        args->mMenuEntry = entry.value;
    }
    else if (StringTool::strcasecmp (entry.key, "InputMode")) {
        if (StringTool::strcasecmp (entry.value, "deactivateRemotes")) {
            args->mDeactivateRemotes = true;
            args->mSlaveMode = false;
        }
        else if (StringTool::strcasecmp (entry.value, "slave")) {
            args->mDeactivateRemotes = false;
            args->mSlaveMode = true;
        }
        else if (StringTool::strcasecmp (entry.value, "normal") ||
                 StringTool::strcasecmp (entry.value, "default")) {
            args->mDeactivateRemotes = false;
            args->mSlaveMode = false;
        }
        else {
            throw InvalidKeywordException(entry.value, position, mConfigFileContent);
        }
    }
    else if (StringTool::strcasecmp (entry.key, "OutputMode")) {
        if (StringTool::strcasecmp (entry.value, "extern")) {
            args->mPlayMode = pmExtern_THIS_SHOULD_BE_AVOIDED;
        }
        else if (StringTool::strcasecmp (entry.value, "none")) {
            args->mPlayMode = pmNone;
        }
        else if (StringTool::strcasecmp (entry.value, "audioOnly")) {
            args->mPlayMode = pmAudioOnly;
        }
        else if (StringTool::strcasecmp (entry.value, "audioOnlyBlack")) {
            args->mPlayMode = pmAudioOnlyBlack;
        }
    }
    else if (StringTool::strcasecmp (entry.key, "BlockMenu")) {
        if ((StringTool::strcasecmp (entry.value, "true")) || (entry.value == "1")) {
            args->mBlockMenu = true;
        }
        else if (StringTool::strcasecmp (entry.value, "false") || (entry.value == "0")) {
            args->mBlockMenu = false;
        }
        else {
            throw InvalidKeywordException(entry.value, position, mConfigFileContent);
        }
    }
    else {
        found = false;
        string keyname;
        const char *vdrkeyname;
        for (int key = kUp; key <= kKbd; key++) {
            vdrkeyname = keys.ToString((eKeys)key, false);
            if (vdrkeyname != NULL)
            {
                keyname = "vdrKey";
                keyname += vdrkeyname;
                if (StringTool::strcasecmp (entry.key, keyname)) {
                    found = true;
                    string *keyString = GetCodeSpecialKey(entry.value);
                    if (keyString != NULL) {
                        args->mKeys.SetKey ((eKeys)key, *keyString);
                        delete keyString;
                    }
                    else {
                        args->mKeys.SetKey((eKeys)key, entry.value);
                    }
                    break;
                }
            }
        }
    }

    if (!found) {
        throw InvalidKeywordException(entry.key, position, mConfigFileContent);
    }
}

string *cExternalplayerConfig::GetCodeSpecialKey(string name) {
    if (name == "noKey") {
        return new string("");
    }
    else if (name == "specialKeyUp") {
        return new string("\e[A");
    }
    else if (name == "specialKeyDown") {
        return new string("\e[B");
    }
    else if (name == "specialKeyRight") {
        return new string("\e[C");
    }
    else if (name == "specialKeyLeft") {
        return new string("\e[D");
    }
    else if (name == "specialKeyF1") {
        return new string("\eOP");
    }
    else if (name == "specialKeyF2") {
        return new string("\eOQ");
    }
    else if (name == "specialKeyF3") {
        return new string("\eOR");
    }
    else if (name == "specialKeyF4") {
        return new string("\eOS");
    }
    else if (name == "specialKeyF5") {
        return new string("\e[15~");
    }
    else if (name == "specialKeyF6") {
        return new string("\e[17~");
    }
    else if (name == "specialKeyF7") {
        return new string("\e[18~");
    }
    else if (name == "specialKeyF8") {
        return new string("\e[19~");
    }
    else if (name == "specialKeyF9") {
        return new string("\e[20~");
    }
    else if (name == "specialKeyF10") {
        return new string("\e[21~");
    }
    else if (name == "specialKeyF11") {
        return new string("\e[23~");
    }
    else if (name == "specialKeyF12") {
        return new string("\e[24~");
    }
    else if (name == "specialKeyIns") {
        return new string("\e[2~");
    }
    else if (name == "specialKeyDel") {
        return new string("\e[3~");
    }
    else if (name == "specialKeyHome") {
        return new string("\e[H");
    }
    else if (name == "specialKeyEnd") {
        return new string("\e[F");
    }
    else if (name == "specialKeyPageUp") {
        return new string("\e[5~");
    }
    else if (name == "specialKeyPageDown") {
        return new string("\e[6~");
    }
    else if (name == "specialKeySpace") {
        return new string(" ");
    }
    else if (name == "specialKeyReturn") {
        return new string("\n");
    }
    else {
        return NULL;
    }
}

unsigned int cExternalplayerConfig::GetLineNumberOfChar(unsigned int charNumber) {
    unsigned int lineNumber = 0;

    for (unsigned int i = 0; i < charNumber; i++) {
        if (mConfigFileContent[i] == '\n') {
            lineNumber++;
        }
    }
    return lineNumber;
}

unsigned int cExternalplayerConfig::GetColumnNumberOfChar(unsigned int charNumber) {
    unsigned int columnNumber = 1;

    for (unsigned int i = 0; i < charNumber; i++) {
        if (mConfigFileContent[i] == '\n') {
            columnNumber = 1;
        }
        else {
            columnNumber++;
        }
    }

    return columnNumber;
}
