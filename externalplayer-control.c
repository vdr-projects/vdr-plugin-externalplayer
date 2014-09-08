/*
 * externalplayer-control.c: A plugin for the Video Disk Recorder
 *
* Initially written by Felix Hädicke
 *
 * 2013 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 */

#include "externalplayer-control.h"
#include "externalplayer-player.h"
/*
cStatusExternalplayer::cStatusExternalplayer(sPlayerArgs *nConfig) : cStatus() {
	mConfig = nConfig;
}

void cStatusExternalplayer::OsdTitle(const char * title) {
	if (mConfig->mBlockMenu) {
		cRemote::SetEnabled(kMenu);
		isyslog("externalplayer-plugin: menu blocked");
	}
}
*/
// --- cControlExternalplayer ------------------------------------------------

cControlExternalplayer::cControlExternalplayer(sPlayerArgs * nConfig, int fdsPipe[2])
: cControl(player = new cPlayerExternalplayer(nConfig->mPlayMode, nConfig, fdsPipe[0])) {
	mConfig = nConfig;
//	mStatus = new cStatusExternalplayer(mConfig);
	fdWritePipe = fdsPipe[1];
	fdReadPipe = fdsPipe[0];
	if (mConfig->mBlockMenu) {
		cRemote::SetEnabled(false);
	}
}

cControlExternalplayer::~cControlExternalplayer() {
	if (mConfig->mBlockMenu) {
			cRemote::SetEnabled(true);
		}
	dsyslog("externalplayer-plugin: shutting down player");
	if (fdWritePipe != -1) {
		close (fdWritePipe);
	}
	if (fdReadPipe != -1) {
		close (fdReadPipe);
	}
	delete player;
//	delete mStatus;
	player = NULL;
}

eOSState cControlExternalplayer::ProcessKey(eKeys key) {
	if (!(((cPlayerExternalplayer *)player)->isActive())) {
		isyslog("externalplayer-plugin: command stopped");
		return osEnd;
	}

	if (mConfig->mSlaveMode) {
		string keyval = mConfig->mKeys.GetKey (key);
		if (!keyval.empty()) {
			write(fdWritePipe, keyval.c_str(), keyval.size());
		}
		if ((key == kChanUp) || (key == kChanDn)) {
			return osEnd;
		}
	}
	else {
		switch (key) {
		case kStop:
		case kBlue:
		case kChanUp:
		case kChanDn:
			return osEnd;
		default:
			break;
		}
	}

	return osContinue;
}
