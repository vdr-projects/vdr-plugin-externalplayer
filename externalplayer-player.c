/*
 * externalplayer-player.c: A plugin for the Video Disk Recorder
 *
 * Initially written by Felix Hädicke
 *
 * 2013 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "externalplayer-player.h"

int cKillThread::Wait(int pid)
{
    int stat_loc = 0;
    int cnt = 0;
    while (waitpid (pid, &stat_loc, WNOHANG) != pid) {
        cnt++;
        if (cnt > 5) {
            return false;
        }
        sleep(1);
    }
    return true;
}

void cKillThread::Action(void)
{
    if (kill(mPid, 0) == 0) {
        kill(mPid, SIGTERM);
        if (Wait (mPid)) {
            return;
        }
        isyslog("externalplayer-plugin: player did not terminate properly. Killing process %i", mPid);
        Skins.QueueMessage(mtInfo, tr("player did not terminate properly"));
        kill(mPid, SIGKILL);
        if (!Wait (mPid)) {
            isyslog("externalplayer-plugin: player did not terminate properly. Can not killing process %i", mPid);
        }
    }
}

void cKillThread::KillProc(int npid)
{
    mPid = npid;
    SetDescription ("KillThread pid %d", mPid);
    Start();
}

cPlayerExternalplayer::cPlayerExternalplayer(ePlayMode playMode, sPlayerArgs * nConfig, int nFdReadPipe)
                                              : cPlayer(playMode) {
    mConfig = nConfig;

    fdReadPipe = nFdReadPipe;

    if (mConfig->mDeactivateRemotes) {
        mRemotesDisable = new cRemotesDisable();
    }
    else {
        mRemotesDisable = NULL;
    }
    mPid = 0;
}

cPlayerExternalplayer::~cPlayerExternalplayer() {

    Activate(false);
    delete mRemotesDisable;
}

void cPlayerExternalplayer::Activate(bool On) {
    if (On) {
        if (mRemotesDisable != NULL) {
            mRemotesDisable->DeactivateRemotes();
        }

        int nPid = fork();
        if (nPid == 0) {
            // Start a new session
            pid_t sid = setsid();
            if (sid < 0) {
                isyslog("externalplayer-plugin: can not create new session");
            }
            if (mConfig->mSlaveMode) {
                dup2(fdReadPipe, STDIN_FILENO);
            }
            if (execl("/bin/sh", "sh", "-c", mConfig->mPlayerCommand.c_str(), NULL) == -1) {
                LOG_ERROR_STR(mConfig->mPlayerCommand.c_str());
                exit(-1);
            }
            isyslog("externalplayer-plugin: SystemExec failed");
            exit(0);
        }
        else {
            mPid = nPid;
            isyslog("externalplayer-plugin: PID of child process: %i executing \"%s\"",
                    mPid, mConfig->mPlayerCommand.c_str());
        }
    }
    else {
        if (mRemotesDisable != NULL) {
            mRemotesDisable->ReactivateRemotes();
        }
        if (mPid != 0) {
            mKillThread.KillProc(mPid);
        }
    }
}

bool cPlayerExternalplayer::isActive() {
    int stat_loc = 0;
    if (mPid == 0) {
        return false;
    }
    if (waitpid(mPid, &stat_loc, WNOHANG) == 0) {
        return true;
    }
    mPid = 0;
    if (!WIFEXITED (stat_loc))
    {
        LOG_ERROR_STR("externalplayer-plugin: Child died unexpected\n");
        Skins.QueueMessage(mtError, tr("Programm crashed"));
    }
    return false;
}
