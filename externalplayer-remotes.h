/*
 * externalplayer-remotes.h: A plugin for the Video Disk Recorder
 *
 * Initially written by Felix Hädicke
 *
 * 2013 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 */

#ifndef _EXTERNALPLAYER_REMOTES_H_
#define _EXTERNALPLAYER_REMOTES_H_

#include <vdr/thread.h>
#include <vdr/remote.h>

using namespace std;

class cRemotesDisable {
private:
    bool deactivated;
public:
    void DeactivateRemotes(void);
    void ReactivateRemotes(void);
};

class cRemotesDisableHelper: public cRemote, public cThread {
public:
    cRemotesDisableHelper(const char * name);
    void Deactivate(void);
    void Reactivate(void);
};

#endif /*_EXTERNALPLAYER_REMOTES_H_*/
