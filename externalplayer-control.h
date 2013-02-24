/*
 * externalplayer-control.h: A plugin for the Video Disk Recorder
 *
 * Initially written by Felix Hädicke
 *
 * 2013 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 */

#ifndef _EXTERNALPLAYER_CONTROL_H_
#define _EXTERNALPLAYER_CONTROL_H_

#include <vdr/player.h>
#include <vdr/status.h>

#include "externalplayer-player.h"

using namespace std;

class cStatusExternalplayer : public cStatus {
private:
  sPlayerArgs * config;
public:
  cStatusExternalplayer(sPlayerArgs * nConfig);
  ~cStatusExternalplayer() {}
  void OsdTitle(const char * title);
};

class cControlExternalplayer : public cControl {
private:
  sPlayerArgs *config;
  cStatusExternalplayer *status;
  int fdWritePipe;
  int fdReadPipe;
public:
  cControlExternalplayer(sPlayerArgs * nConfig, int fdsPipe[2]);
  ~cControlExternalplayer();
  void Hide() {}
  eOSState ProcessKey(eKeys key);
};

#endif /*_EXTERNALPLAYER_CONTROL_H_*/
