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

// Primary chat functions

/** Broadcast a message to a channel, using @chatformat if it's
 *  available, and mogrifying.
 * \param channel pointer to channel to broadcast to.
 * \param player message speaker.
 * \param flags broadcast flag mask (see CB_* constants in extchat.h)
 * \param fmt message format string.
 */
void
kchan_send(CHAN *channel, dbref player, int flags, const char *origmessage);

// Colorize Functions



/** Add color from a variety of sources to the text argument of say and +<chan>
 *  available, and mogrifying.
 * \param tbuf1 buffer to return output to
 * \param tp pointer to output buffer
 * \param message text string to be formatted
 * \param player dbref of caller
 * \param channel channel being requested
 */
int speech_colorize(char *tbuf1, char **tp, char *message, dbref player,  CHAN channel);

/** Add color from a variety of sources to the text argument of pose and semipose
 *  available, and mogrifying.
 * \param tbuf1 buffer to return output to
 * \param tp pointer to output buffer
 * \param message text string to be formatted
 * \param player dbref of caller
 * \param channel channel being requested
 * \param flags takes argument CB_SEMIPOSE or CB_POSE, NULL/invalid is assumed as CB_POSE
 */
int pose_colorize(char *tbuf1, char **tp, char *message, dbref player,  CHAN channel, int flags);

/** colorize a channel name based on mogrifier and chat formatting
 *  available, and mogrifying.
 * \param tbuf1 buffer to return output to
 * \param tp pointer to output buffer
 * \param message text string to be formatted
 * \param channel channel being requested
 */
int chan_colorize(char *tbuf1, char **tp, char *message, CHAN channel);

/** applies things like nameformat() to the player
 *  available, and mogrifying.
 * \param tbuf1 buffer to return output to
 * \param tp pointer to output buffer
 * \param message text string to be formatted
 * \param player player to customize
 */
int player_colorize(char *tbuf1, char **tp, dbref player);




#endif