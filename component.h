/*
 * Copyright (C) 2007-2011  Sebastian Schuberth <sschuberth_AT_gmail_DOT_com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef COMPONENT_H
#define COMPONENT_H

// Get rid of tons of warnings.
#pragma warning(push,1)
#include <foobar2000/SDK/foobar2000.h>
#pragma warning(pop)

#include "./version.inl"

#define FOO_COMP_FILE     _T("foo_cdartdisplay")
#define FOO_COMP_CLASS    _T("foo_cdartdisplay_class")

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
