/*
 * externalplayer.c: A plugin for the Video Disk Recorder
 *
 * Initially written by Felix Hädicke
 *
 * 2012 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 */

#include <string.h>
#include <fstream>
#include <list>
#include <getopt.h>

#include "externalplayer.h"
#include "externalplayer-player.h"
#include "externalplayer-control.h"
#include "externalplayer-remotes.h"

cPluginExternalplayer::cPluginExternalplayer() {
  playerConfig = NULL;
  configFilename = "";
}

cPluginExternalplayer::~cPluginExternalplayer() {
  delete playerConfig;
}

void cPluginExternalplayer::StartPlayer(sPlayerArgs *config) {
  isyslog("externalplayer-plugin: starting player: %s", config->mMenuEntry.c_str());

  int fdsPipe[2];
  if (config->mSlaveMode) {
    pipe(fdsPipe);
  }

  cControl::Launch(new cControlExternalplayer(config, fdsPipe));
  cControl::Attach();
}

const char *cPluginExternalplayer::CommandLineHelp() {
  return "  -C FILE,  --config=FILE  specify path to configuration file\n";
}

bool cPluginExternalplayer::ProcessArgs(int argc, char *argv[]) {
  static struct option long_options[] = {
    { "config", required_argument, NULL, 'C' },
    { NULL }
  };

  int c;
  while ((c = getopt_long(argc, argv, "C:", long_options, NULL)) != -1) {
    switch (c) {
      case 'C': configFilename = optarg;
                break;
      default:  return false;
    }
  }
  return true;
}

bool cPluginExternalplayer::Initialize() {
  return true;
}

bool cPluginExternalplayer::Start() {
  if (configFilename == "") {
    configFilename += ConfigDirectory();
    configFilename += "/externalplayer.conf";
  }

  playerConfig = new cExternalplayerConfig(configFilename);

  return true;
}

void cPluginExternalplayer::Stop() {
}

void cPluginExternalplayer::Housekeeping() {
}

const char * cPluginExternalplayer::MainMenuEntry() {
  int count = playerConfig->PlayerCount();
  if (count == 0) {
    return NULL;
  }
  else if (count == 1) {
    return playerConfig->GetConfiguration().front()->mMenuEntry.c_str();
  }
  else {
    return tr("External Players");
  }
}

cOsdObject * cPluginExternalplayer::MainMenuAction() {
  int count = playerConfig->PlayerCount();
  if (count == 0) {
    return NULL;
  }
  else if (count == 1) {
    StartPlayer(playerConfig->GetConfiguration().front());
    return NULL;
  }
  else {
    return new cOsdExternalplayer(playerConfig);
  }
}

cMenuSetupPage *cPluginExternalplayer::SetupMenu() {
  return NULL;
}

bool cPluginExternalplayer::SetupParse(const char *Name, const char *Value) {
  return false;
}

bool cPluginExternalplayer::Service(const char *Id, void *Data) {
  return false;
}

const char **cPluginExternalplayer::SVDRPHelpPages(void)
{
    static const char *HelpPages[] = {
            "EXEC: Execute first entry\n",
            NULL
    };
    return HelpPages;
}

cString cPluginExternalplayer::SVDRPCommand(const char *Command, const char *Option,
                                            int &ReplyCode)
{
    sPlayerArgs *config = NULL;
    char *endptr = NULL;
    long opt = 0;

    if (strcasecmp(Command, "EXEC") != 0) {
        return NULL;
    }
    if ((Option == NULL) || (Option[0] == '\0')) {
        config = playerConfig->GetConfiguration().front();
    }
    else {
        errno = 0;
        opt = strtol (Option, &endptr, 10);
        if (((errno == ERANGE) && ((opt == LONG_MAX) || (opt == LONG_MIN))) ||
             ((errno != 0) && (opt == 0)) ||
             (Option == endptr))  {
            ReplyCode = 504;
            return cString::sprintf("Invalid number \"%s\"", Option);
        }
        try {
            config = playerConfig->GetConfiguration (opt);
        }
        catch (const std::out_of_range &oor)
        {
            ReplyCode = 504;
            return cString::sprintf("Configuration %ld not available", opt);
        }
    }
    StartPlayer(config);
    return "OK";
}

// --- cOsdExternalplayer ---------------------------------------------------

cOsdExternalplayer::cOsdExternalplayer(cExternalplayerConfig * nPlayerConfig) :
                                             cOsdMenu(tr("External Players")) {
  int cnt = 1;
  char num[4];
  sPlayerArgs *nConf;
  string menutxt;
  playerConfig = nPlayerConfig;
  sPlayerArgsList playerArgs = playerConfig->GetConfiguration();
  for (sPlayerArgsList::iterator i = playerArgs.begin(); i != playerArgs.end(); i++) {
      nConf = *i;
      if (cnt <= 9) {
          sprintf(num,"%d ", cnt);
      }
      else {
          strcpy (num, "  ");
      }

      menutxt = num + nConf->mMenuEntry;
      Add(new cOsdItemExternalplayer(cnt, nPlayerConfig, menutxt.c_str()));
      cnt++;
  }
}

cOsdExternalplayer::~cOsdExternalplayer() {
}

cOsdItemExternalplayer::cOsdItemExternalplayer(int cnt,
                                               cExternalplayerConfig *conf,
                                               const char *menutxt) :
                                                        cOsdItem(menutxt) {
  mCnt = cnt;
  mConfig = conf;
}

// --- cOsdItemExternalplayer -----------------------------------------------

eOSState cOsdItemExternalplayer::ProcessKey(eKeys key) {
  eOSState state = osUnknown;

  if (key == kOk) {
    cPluginExternalplayer::StartPlayer(mConfig->GetConfiguration(mCnt-1));
    return osEnd;
  }
  if ((key > k0) && (key <= k9)) {
      try
      {
          cPluginExternalplayer::StartPlayer(mConfig->GetConfiguration (key - k1));
          state = osEnd;
      }
      catch (const std::out_of_range &oor)
      {
      }
  }
  return state;
}

VDRPLUGINCREATOR(cPluginExternalplayer);
