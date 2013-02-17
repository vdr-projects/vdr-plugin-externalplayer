/*
 * externalplayer-remotes.c: A plugin for the Video Disk Recorder
 *
 * Initially written by Felix Hädicke
 *
 * 2013 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 */

#include <string.h>

#include "externalplayer-remotes.h"

void cRemotesDisable::DeactivateRemotes() {
  if (!deactivated) {
    isyslog("externalplayer-plugin: deactivating remotes");
    for (cRemote * i = Remotes.First(); i != NULL; i = Remotes.Next(i)) {
      if (strcmp(i->Name(), "LIRC") == 0) {
        ((cRemotesDisableHelper *) i)->Deactivate();
      }
      else if (strcmp(i->Name(), "RCU") == 0) {
        ((cRemotesDisableHelper *) i)->Deactivate();
      }
    }
  }

  deactivated = true;
}

void cRemotesDisable::ReactivateRemotes() {
  if (deactivated) {
    isyslog("externalplayer-plugin: reactivating remotes");
    for (cRemote * i = Remotes.First(); i != NULL; i = Remotes.Next(i)) {
      if (strcmp(i->Name(), "LIRC") == 0) {
        ((cRemotesDisableHelper *) i)->Reactivate();
      }
      else if (strcmp(i->Name(), "RCU") == 0) {
        ((cRemotesDisableHelper *) i)->Reactivate();
      }
    }
  }

  deactivated = false;
}

// --- cRemotesDisableHelper -------------------------------------------------

cRemotesDisableHelper::cRemotesDisableHelper(const char * name) : cRemote(name) {
}

void cRemotesDisableHelper::Deactivate() {
  if (Active()) {
    Cancel();
  }
}

void cRemotesDisableHelper::Reactivate() {
  if (!Active()) {
    Start();
  }
}
