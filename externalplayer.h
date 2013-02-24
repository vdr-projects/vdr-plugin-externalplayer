/*
 * externalplayer.h: A plugin for the Video Disk Recorder
 *
* Initially written by Felix Hädicke
 *
 * 2012 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 */

#ifndef _EXTERNALPLAYER_H_
#define _EXTERNALPLAYER_H_

#include <vdr/plugin.h>
#include <vdr/thread.h>
#include <vdr/player.h>
#include <vdr/menu.h>

#include "externalplayer-config.h"

using namespace std;

static const char *VERSION        = "0.3.0";
static const char *DESCRIPTION    = tr("launch external players");

class cPluginExternalplayer : public cPlugin {
private:
  string configFilename;
  cExternalplayerConfig * playerConfig;
public:
  cPluginExternalplayer();
  virtual ~cPluginExternalplayer();
  virtual const char * Version() { return VERSION; }
  virtual const char * Description() { return DESCRIPTION; }
  virtual const char * CommandLineHelp();
  virtual bool ProcessArgs(int argc, char * argv[]);
  virtual bool Initialize();
  virtual bool Start();
  virtual void Stop();
  virtual void Housekeeping();
  virtual const char * MainMenuEntry();
  virtual cOsdObject * MainMenuAction();
  virtual cMenuSetupPage * SetupMenu();
  virtual bool SetupParse(const char * Name, const char * Value);
  virtual bool Service(const char * Id, void * Data = NULL);
  static void StartPlayer(sPlayerArgs * config);
};

class cOsdExternalplayer : public cOsdMenu {
private:
  cExternalplayerConfig * playerConfig;
public:
  cOsdExternalplayer(cExternalplayerConfig * nPlayerConfig);
  ~cOsdExternalplayer();
};

class cOsdItemExternalplayer : public cOsdItem {
private:
  sPlayerArgs * config;
public:
  cOsdItemExternalplayer(sPlayerArgs * nConfig);
  ~cOsdItemExternalplayer() {}
  virtual eOSState ProcessKey(eKeys key);
};

#endif /*_EXTERNALPLAYER_H_*/
