/*
 * This is a foobar2000 0.9.x <http://www.foobar2000.org/> component to interface
 * with CD Art Display 1.x <http://www.closetosoftware.com/?s=cdartdisplay>.
 *
 * It is an alternative to using foo_winamp_spam in that it does not simulate the
 * Winamp 1.x API <http://www.oldversion.com/program.php?n=winamp> but the more
 * powerful Helium Music Manager 2007 API <http://www.helium-music-manager.com/>.
 *
 * This version of the component is written against the API specification version
 * 1.6 dated 2007-09-04.
 */

#ifndef COMPONENT_H
#define COMPONENT_H

// Get rid of tons of warnings.
#pragma warning(push,1)
#include <foobar2000/SDK/foobar2000.h>
#pragma warning(pop)

#define FOO_PLUGIN_NAME         "CD Art Display Interface"
#define FOO_TARGET_CAD_VERSION  "1.0"
#define FOO_PLUGIN_RELEASE      "1"

#define FOO_PLUGIN_FILE         _T("foo_cdartdisplay")
#define FOO_WINDOW_CLASS_NAME   _T("foo_cdartdisplay_class")

enum HeliumMessage {
    IPC_PLAY                              = 100,
    IPC_PLAYPAUSE                              ,
    IPC_FORCEPAUSE                             ,
    IPC_STOP                                   ,
    IPC_NEXT                                   ,
    IPC_PREVIOUS                               ,

    IPC_SET_VOLUME                        = 108,
    IPC_GET_VOLUME                             ,
    IPC_GET_CURRENT_TRACK                      ,

    IPC_GET_DURATION                      = 113,
    IPC_SET_POSITION                           ,
    IPC_IS_PLAYING                             ,
    IPC_IS_PAUSED                              ,
    IPC_GET_LIST_LENGTH                        ,
    IPC_SET_LIST_POS                           ,
    IPC_GET_LIST_ITEM                          ,
    IPC_SET_CALLBACK_HWND                      ,
    IPC_GET_LIST_POS                           ,
    IPC_GET_POSITION                           ,
    IPC_TRACK_CHANGED_NOTIFICATION             , // Message to send to CAD.
    IPC_SHOW_PLAYER_WINDOW                     ,
    IPC_GET_PLAYER_STATE                       ,
    IPC_PLAYER_STATE_CHANGED_NOTIFICATION      , // Message to send to CAD.
    IPC_AUTOENQUEUE_OPTIONS                    , // Ignored.
    IPC_SET_REPEAT                             ,
    IPC_SHUTDOWN_NOTIFICATION                  , // Message to send to CAD.
    IPC_GET_REPEAT                             ,
    IPC_CLOSE_PLAYER                           ,
    IPC_SET_RATING                             , // See IPC_RATING_CHANGED_NOTIFICATION.

    IPC_GET_SHUFFLE                       = 140,
    IPC_SET_SHUFFLE                            ,

    IPC_RATING_CHANGED_NOTIFICATION       = 639, // Message to both send and receive.

    IPC_NEW_COVER_NOTIFICATION            = 800, // Message to send to CAD (ignored).
    IPC_GET_CURRENT_LYRICS                     ,
    IPC_ADDFILE_PLAY_PLAYLIST                  ,
    IPC_ADDFILE_QUEUE_PLAYLIST
};

enum HeliumState {
    HS_STOPPED    ,
    HS_PLAYING    ,
    HS_PAUSED
};

#endif // COMPONENT_H
