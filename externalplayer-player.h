/*
 * externalplayer-player.h: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef _EXTERNALPLAYER_PLAYER_H_
#define _EXTERNALPLAYER_PLAYER_H_

#include <vdr/player.h>

#include "externalplayer-remotes.h"
#include "externalplayer-config.h"

using namespace std;

class cKillThread : public cThread {
protected:
    int mPid;
    void Action(void);
    int Wait(int pid);
public:
    void KillProc (int npid);
};

class cPlayerExternalplayer : public cPlayer {
private:
    cKillThread mKillThread;
    cRemotesDisable *mRemotesDisable;
    sPlayerArgs *mConfig;
    int fdReadPipe;
    pid_t mPid;
protected:
    void Activate(bool On);

public:
    cPlayerExternalplayer(ePlayMode playMode, sPlayerArgs * nConfig, int nFdReadPipe);
    ~cPlayerExternalplayer();
    bool isActive();
    void Stop(void) {
        if (mPid != 0) {
            mKillThread.KillProc(mPid);
        }
    }
};

#endif /*_EXTERNALPLAYER_PLAYER_H_*/
