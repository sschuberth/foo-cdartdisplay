/*
 * Copyright (C) 2007-2010  Sebastian Schuberth <sschuberth_AT_gmail_DOT_com>
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

// "Default" {bfc61179-49ad-4e95-8d60-a22706485505}
static GUID const ORDER_DEFAULT=
{ 0xbfc61179, 0x49ad, 0x4e95, { 0x8d, 0x60, 0xa2, 0x27, 0x06, 0x48, 0x55, 0x05 } };

// "Repeat (playlist)" {681cc6ea-60ae-4bf9-913b-bb5f4e864f2a}
static GUID const ORDER_REPEAT_PLAYLIST=
{ 0x681cc6ea, 0x60ae, 0x4bf9, { 0x91, 0x3b, 0xbb, 0x5f, 0x4e, 0x86, 0x4f, 0x2a } };

// "Repeat (track)" {4bf4b280-0bb4-4dd0-8e84-37c3209c3da2}
static GUID const ORDER_REPEAT_TRACK=
{ 0x4bf4b280, 0x0bb4, 0x4dd0, { 0x8e, 0x84, 0x37, 0xc3, 0x20, 0x9c, 0x3d, 0xa2 } };

// "Shuffle (tracks)" {c5cf4a57-8c01-480c-b334-3619645ada8b}
static GUID const ORDER_SHUFFLE_TRACKS=
{ 0xc5cf4a57, 0x8c01, 0x480c, { 0xb3, 0x34, 0x36, 0x19, 0x64, 0x5a, 0xda, 0x8b } };

// "Shuffle (albums)" {499e0b08-c887-48c1-9cca-27377c8bfd30}
static GUID const ORDER_SHUFFLE_ALBUMS=
{ 0x499e0b08, 0xc887, 0x48c1, { 0x9c, 0xca, 0x27, 0x37, 0x7c, 0x8b, 0xfd, 0x30 } };

// "Shuffle (directories)" {83c37600-d725-4727-b53c-bdeffe5f8dc7}
static GUID const ORDER_SHUFFLE_DIRECTORIES=
{ 0x83c37600, 0xd725, 0x4727, { 0xb5, 0x3c, 0xbd, 0xef, 0xfe, 0x5f, 0x8d, 0xc7 } };

// Returns the index of a GUID-identified order mode.
static t_size get_playback_order_index(static_api_ptr_t<playlist_manager>& plm,GUID const& guid)
{
    for (t_size i=0;i<plm->playback_order_get_count();++i) {
        if (plm->playback_order_get_guid(i)==guid) {
            return i;
        }
    }
    return pfc::infinite_size;
}

static audio_sample scale_to_gain(double p_scale)
{
    return static_cast<audio_sample>(20.0*log10(p_scale));
}

static int round_to_even(double d)
{
    return _mm_cvtsd_si32(_mm_load_sd(&d));
}
