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
  mControl = NULL;
  mPlayerConfig = NULL;
  mConfigFilename = "";
}

cPluginExternalplayer::~cPluginExternalplayer() {
  delete mPlayerConfig;
}

void cPluginExternalplayer::StartPlayer(sPlayerArgs *config) {
  isyslog("externalplayer-plugin: starting player: %s",
          config->mMenuEntry.c_str());

  int fdsPipe[2];
  if (config->mSlaveMode) {
    pipe(fdsPipe);
  }

  mControl = new cControlExternalplayer(config, fdsPipe);
  cControl::Launch(mControl);
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
      case 'C': mConfigFilename = optarg;
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
  if (mConfigFilename == "") {
    mConfigFilename += ConfigDirectory();
    mConfigFilename += "/externalplayer.conf";
  }

  mPlayerConfig = new cExternalplayerConfig(mConfigFilename);

  return true;
}

void cPluginExternalplayer::Stop() {
    if (mControl != NULL) {
        mControl->Stop();
    }
}

void cPluginExternalplayer::Housekeeping() {
}

const char * cPluginExternalplayer::MainMenuEntry() {
  int count = mPlayerConfig->PlayerCount();
  if (count == 0) {
    return NULL;
  }
  else if (count == 1) {
    return mPlayerConfig->GetConfiguration().front()->mMenuEntry.c_str();
  }
  else {
    return tr("External Players");
  }
}

cOsdObject * cPluginExternalplayer::MainMenuAction() {
  int count = mPlayerConfig->PlayerCount();
  if (count == 0) {
    return NULL;
  }
  else if (count == 1) {
    StartPlayer(mPlayerConfig->GetConfiguration().front());
    return NULL;
  }
  else {
    return new cOsdExternalplayer(this);
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
            "LIST: List available configurations\n",
            "EXEC <no>: Execute entry no <no>\n",
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

    if (strcasecmp(Command, "EXEC") == 0) {
        if ((Option == NULL) || (Option[0] == '\0')) {
            config = mPlayerConfig->GetConfiguration().front();
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
                config = mPlayerConfig->GetConfiguration (opt);
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
    else if (strcasecmp(Command, "LIST") == 0) {
        int cnt = 1;
        string ret = "";
        char buf[10];
        sPlayerArgs *nConf;
        sPlayerArgsList playerArgs = mPlayerConfig->GetConfiguration();
        for (sPlayerArgsList::iterator i = playerArgs.begin(); i != playerArgs.end(); i++) {
            nConf = *i;
            sprintf(buf,"%3d ", cnt);
            if (cnt > 1) {
                ret += "\n";
            }
            ret += buf + nConf->mMenuEntry;
            cnt++;
        }
        if (ret.empty()) {
            ReplyCode = 504;
            return "No config available";
        }
        return cString(ret.c_str());
    }
    return NULL;
}

// --- cOsdExternalplayer ---------------------------------------------------

cOsdExternalplayer::cOsdExternalplayer(cPluginExternalplayer *plugin) :
                                             cOsdMenu(tr("External Players")) {
  cExternalplayerConfig *playerconfig;
  int cnt = 1;
  char num[4];
  sPlayerArgs *nConf;
  string menutxt;
  playerconfig = plugin->GetConfig();
  sPlayerArgsList playerArgs = playerconfig->GetConfiguration();
  for (sPlayerArgsList::iterator i = playerArgs.begin(); i != playerArgs.end(); i++) {
      nConf = *i;
      if (cnt <= 9) {
          sprintf(num,"%d ", cnt);
      }
      else {
          strcpy (num, "  ");
      }

      menutxt = num + nConf->mMenuEntry;
      Add(new cOsdItemExternalplayer(cnt, plugin, menutxt.c_str()));
      cnt++;
  }
}

cOsdExternalplayer::~cOsdExternalplayer() {
}

cOsdItemExternalplayer::cOsdItemExternalplayer(int cnt,
                                               cPluginExternalplayer *plugin,
                                               const char *menutxt) :
                                                        cOsdItem(menutxt) {
  mCnt = cnt;
  mPlugin = plugin;
}

// --- cOsdItemExternalplayer -----------------------------------------------

eOSState cOsdItemExternalplayer::ProcessKey(eKeys key) {
  eOSState state = osUnknown;
  cExternalplayerConfig *playerconfig = mPlugin->GetConfig();

  if (key == kOk) {
      mPlugin->StartPlayer(playerconfig->GetConfiguration(mCnt-1));
    return osEnd;
  }
  if ((key > k0) && (key <= k9)) {
      try
      {
          mPlugin->StartPlayer(playerconfig->GetConfiguration(key - k1));
          state = osEnd;
      }
      catch (const std::out_of_range &oor)
      {
      }
  }
  return state;
}

VDRPLUGINCREATOR(cPluginExternalplayer);
