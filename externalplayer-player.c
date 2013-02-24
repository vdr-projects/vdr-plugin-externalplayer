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
    if (kill(pid, 0) == 0) {
        kill(pid, SIGTERM);
        if (Wait (pid)) {
            return;
        }
        isyslog("externalplayer-plugin: player did not terminate properly. Killing process %i", pid);
        kill(pid, SIGKILL);
        if (!Wait (pid)) {
            isyslog("externalplayer-plugin: player did not terminate properly. Can not killing process %i", pid);
        }
    }
}

void cKillThread::KillProc(int npid)
{
    pid = npid;
    SetDescription ("KillThread pid %d", pid);
    Start();
}

cPlayerExternalplayer::cPlayerExternalplayer(ePlayMode playMode, sPlayerArgs * nConfig, int nFdReadPipe)
                                              : cPlayer(playMode) {
    config = nConfig;

    fdReadPipe = nFdReadPipe;

    if (config->mDeactivateRemotes) {
        remotesDisable = new cRemotesDisable();
    }
    else {
        remotesDisable = NULL;
    }
    pid = 0;
}

cPlayerExternalplayer::~cPlayerExternalplayer() {
    Activate(false);
    delete remotesDisable;
}

void cPlayerExternalplayer::Activate(bool On) {
    if (On) {
        if (remotesDisable != NULL) {
            remotesDisable->DeactivateRemotes();
        }

        int nPid = fork();
        if (nPid == 0) {


            isyslog("externalplayer-plugin: executing \"%s\"", config->mPlayerCommand.c_str());
            int MaxPossibleFileDescriptors = getdtablesize();
            for (int i = STDERR_FILENO + 1; i < MaxPossibleFileDescriptors; i++) {
                close(i); //close all dup'ed filedescriptors
            }
            // Start a new session
            pid_t sid = setsid();
            if (sid < 0) {
                isyslog("externalplayer-plugin: can not create new session");
            }
            if (config->mSlaveMode) {
                dup2(fdReadPipe, STDIN_FILENO);
            }
            if (execl("/bin/sh", "sh", "-c", config->mPlayerCommand.c_str(), NULL) == -1) {
                LOG_ERROR_STR(config->mPlayerCommand.c_str());
                exit(-1);
            }
            isyslog("externalplayer-plugin: SystemExec failed");
            exit(0);
        }
        else {
            pid = nPid;
            isyslog("externalplayer-plugin: PID of child process: %i", pid);
        }
    }
    else {
        if (remotesDisable != NULL) {
            remotesDisable->ReactivateRemotes();
        }

        if (pid != 0) {
            mKillThread.KillProc(pid);
        }
    }
}

bool cPlayerExternalplayer::isActive() {
    return (waitpid(pid, NULL, WNOHANG) == 0);
}
