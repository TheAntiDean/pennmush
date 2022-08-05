#ifndef __LOCK_H
#define __LOCK_H


#include "config.h"
#include "extchat.h"
#include "boolexp.h"
#include "bufferq.h"


// Temporary config values
#define MOGSPEECHTEXT (options.cspeechtext ? options.cspeechtext : "MOGRIFY`SPEECHTEXT")
#define MOGFSPEECH (options.cfspeech ? options.cfspeech : "MOGRIFY`FORMAT`SPEECH")
#define MOGFPOSE (options.cfpose ? options.cfpose : "MOGRIFY`FORMAT`POSE")
#define MOGCHAN (options.cfname ? options.cfname : "MOGRIFY`CHANNAME")
#define MOGDBREF (options.ancestor_channel ? options.ancestor_channel : -1)







#endif