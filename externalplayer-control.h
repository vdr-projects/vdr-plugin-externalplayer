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
  sPlayerArgs *mConfig;
public:
  cStatusExternalplayer(sPlayerArgs * nConfig);
  ~cStatusExternalplayer() {}
  void OsdTitle(const char * title);
};

class cControlExternalplayer : public cControl {
private:
  sPlayerArgs *mConfig;
  cStatusExternalplayer *mStatus;
  int fdWritePipe;
  int fdReadPipe;
public:
  cControlExternalplayer(sPlayerArgs * nConfig, int fdsPipe[2]);
  ~cControlExternalplayer();
  void Hide() {}
  eOSState ProcessKey(eKeys key);
  void Stop(void) {
      cPlayerExternalplayer *pl = (cPlayerExternalplayer *)player;
      pl->Stop();
  }
};

#endif /*_EXTERNALPLAYER_CONTROL_H_*/
