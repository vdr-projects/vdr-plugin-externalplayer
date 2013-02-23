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

pid_t pid = 0;
void killProcess(int noParam) {
  if (kill(pid, 0) == 0) {
    isyslog("externalplayer-plugin: player did not terminate properly. Killing process %i", pid);
    kill(pid, SIGKILL);
    signal(SIGALRM, SIG_DFL);
  }
  pid = 0;
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
      if (config->mSlaveMode) {
        dup2(fdReadPipe, STDIN_FILENO);
      }

      isyslog("externalplayer-plugin: executing \"%s\"", config->mPlayerCommand.c_str());
      execle("/bin/sh", "sh", "-c", config->mPlayerCommand.c_str(), NULL, environ);
      isyslog("externalplayer-plugin: execution (of /bin/sh) failed");
      _exit(0);
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
      if (kill(pid, 0) == 0) {
        kill(pid, SIGTERM);
        alarm(2);
        signal(SIGALRM, killProcess);
      }
    }
  }
}

bool cPlayerExternalplayer::isActive() {
  return (waitpid(pid, NULL, WNOHANG) == 0);
}
