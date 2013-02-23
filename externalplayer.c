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

void cPluginExternalplayer::StartPlayer(sPlayerArgs * config) {
  isyslog("externalplayer-plugin: starting player: %s", config->mMenuEntry.c_str());

  int fdsPipe[2];
  if (config->mSlaveMode) {
    pipe(fdsPipe);
  }

  cControl::Launch(new cControlExternalplayer(config, fdsPipe));
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

// --- cOsdExternalplayer ---------------------------------------------------

cOsdExternalplayer::cOsdExternalplayer(cExternalplayerConfig * nPlayerConfig) : cOsdMenu(tr("External Players")) {
  playerConfig = nPlayerConfig;
  list<sPlayerArgs *> playerArgs = playerConfig->GetConfiguration();
  for (list<sPlayerArgs *>::iterator i = playerArgs.begin(); i != playerArgs.end(); i++) {
      Add(new cOsdItemExternalplayer(*i));
  }
}

cOsdExternalplayer::~cOsdExternalplayer() {
}

cOsdItemExternalplayer::cOsdItemExternalplayer(sPlayerArgs * nConfig) : cOsdItem(nConfig->mMenuEntry.c_str()) {
  config = nConfig;
}

// --- cOsdItemExternalplayer -----------------------------------------------

eOSState cOsdItemExternalplayer::ProcessKey(eKeys key) {
  if (key == kOk) {
    cPluginExternalplayer::StartPlayer(config);
    return osEnd;
  }
  return osUnknown;
}

VDRPLUGINCREATOR(cPluginExternalplayer);
