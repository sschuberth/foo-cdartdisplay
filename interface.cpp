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

#include "component.h"

extern cfg_bool cfg_cad_start;
extern cfg_string cfg_cad_path;
extern cfg_bool cfg_write_rating;

DECLARE_COMPONENT_VERSION(
    FOO_COMP_NAME,
    FOO_COMP_VERSION " " FOO_COMP_STATE " " FOO_COMP_RELEASE,
    "This is a message interface component for CD Art Display:\nhttp://www.cdartdisplay.com/\n\n"
    "Compiled on " __DATE__ "\nCopyright (C) 2007-2010  Sebastian Schuberth <sschuberth_AT_gmail_DOT_com>\n\n"
    "The source code is available under LGPL at:\nhttp://foo-cdartdisplay.googlecode.com/"
);

#include "helpers.inl"

//#define VERBOSE_MESSAGE_BOXES

class CDArtDisplayInterface:public initquit,public play_callback
{
  public:

    void on_init() {
        if (!s_atom) {
            // Register a minimal window class.
            WNDCLASS cls;
            memset(&cls,0,sizeof(cls));
            cls.lpfnWndProc=WindowProc;
            cls.hInstance=core_api::get_my_instance();
            cls.hCursor=LoadCursor(NULL,IDC_ARROW);
            cls.lpszClassName=FOO_COMP_CLASS;

            s_atom=RegisterClass(&cls);
            if (!s_atom) {
                MessageBox(core_api::get_main_window(),_T("Unable to register the window class."),FOO_COMP_FILE,MB_OK|MB_ICONERROR);
                return;
            }
        }

        // Create a dummy window.
        m_dummy_window=CreateWindow(
        /* lpClassName  */ MAKEINTATOM(s_atom),
        /* lpWindowName */ FOO_COMP_FILE,
        /* dwStyle      */ 0,
        /* x            */ 0,
        /* y            */ 0,
        /* nWidth       */ 0,
        /* nHeight      */ 0,
        /* hWndParent   */ core_api::get_main_window(),
        /* hMenu        */ NULL,
        /* hInstance    */ core_api::get_my_instance(),
        /* lpParam      */ this
        );
        if (!m_dummy_window) {
            MessageBox(core_api::get_main_window(),_T("Unable to create the dummy window."),FOO_COMP_FILE,MB_OK|MB_ICONERROR);
            return;
        }
        ++s_instances;

        // Try to find a running CAD instance.
        m_cda_window=FindWindow(NULL,_T("CD Art Display 1.x Class"));

#ifdef VERBOSE_MESSAGE_BOXES
        MessageBox(core_api::get_main_window(),_T("Construction was successful."),FOO_COMP_FILE,MB_OK|MB_ICONINFORMATION);
#endif

        // If enabled, start CAD together with foobar2000 ...
        if (cfg_cad_start) {
            // ... but only if it is not already running.
            if (m_cda_window) {
                SendMessage(m_cda_window,WM_USER,0,IPC_TRACK_CHANGED_NOTIFICATION);
            }
            else {
                pfc::string8 cmd_line("\"");
                cmd_line+=cfg_cad_path;
                cmd_line+="\" foobar2000";

                // Get the foobar2000 configuration strings and convert them to the
                // OS' format.
                pfc::stringcvt::string_os_from_utf8 path2os;
                path2os.convert(cmd_line);

                STARTUPINFO si={0};
                PROCESS_INFORMATION pi;

                BOOL result=CreateProcess(
                    NULL,
                    const_cast<LPWSTR>(path2os.get_ptr()),
                    NULL,
                    NULL,
                    FALSE,
                    0,
                    NULL,
                    NULL,
                    &si,
                    &pi
                );

                if (!result) {
                    MessageBox(core_api::get_main_window(),_T("Unable to launch CD Art Display."),FOO_COMP_FILE,MB_OK|MB_ICONERROR);
                }
                else {
                    // Wait at most 5 seconds for CAD to register itself.
                    int i=50;
                    MSG msg;

                    while (m_cda_window==NULL && i>0) {
                        // Dispatch any message in the queue.
                        if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }

                        Sleep(100);
                        --i;
                    }

                    if (i==0) {
                        MessageBox(
                            core_api::get_main_window(),
                            _T("Timeout while waiting for the CD Art Display window to register itself. Make sure you are running CD Art Display version 2.0 or newer."),
                            FOO_COMP_FILE,
                            MB_OK|MB_ICONWARNING
                        );
                    }
                }
            }
        }

        static_api_ptr_t<play_callback_manager>()->register_callback(
            this,
            play_callback::flag_on_playback_starting  |
            play_callback::flag_on_playback_stop      |
            play_callback::flag_on_playback_pause     |
            play_callback::flag_on_playback_new_track |
            play_callback::flag_on_playback_edited    |
            play_callback::flag_on_playback_dynamic_info_track,
            false
        );
    }

    void on_quit() {
        static_api_ptr_t<play_callback_manager>()->unregister_callback(this);

        if (!DestroyWindow(m_dummy_window)) {
            MessageBox(core_api::get_main_window(),_T("Unable to destroy the dummy window."),FOO_COMP_FILE,MB_OK|MB_ICONERROR);
            return;
        }

        if (--s_instances<=0) {
            if (!UnregisterClass(MAKEINTATOM(s_atom),NULL)) {
                MessageBox(core_api::get_main_window(),_T("Unable to unregister the window class."),FOO_COMP_FILE,MB_OK|MB_ICONERROR);
                return;
            }

            s_atom=0;
        }

#ifdef VERBOSE_MESSAGE_BOXES
        MessageBox(core_api::get_main_window(),_T("Destruction was successful."),FOO_COMP_FILE,MB_OK|MB_ICONINFORMATION);
#endif
    }

    // warning C4100: unreferenced formal parameter
    #pragma warning(disable:4100)

    void on_playback_seek(double p_time) {}
    void on_playback_dynamic_info(file_info const& p_info) {}
    void on_playback_time(double p_time) {}
    void on_volume_change(float p_new_val) {}

    #pragma warning(default:4100)

    void on_playback_starting(play_control::t_track_command p_command,bool p_paused) {
        if (p_paused) {
            SendMessage(m_cda_window,WM_USER,static_cast<WPARAM>(HS_PAUSED),IPC_PLAYER_STATE_CHANGED_NOTIFICATION);
        }
        else {
            if (p_command==play_control::track_command_play
             || p_command==play_control::track_command_resume
             || p_command==play_control::track_command_settrack) {
                SendMessage(m_cda_window,WM_USER,static_cast<WPARAM>(HS_PLAYING),IPC_PLAYER_STATE_CHANGED_NOTIFICATION);
            }
        }
    }

    void on_playback_stop(play_control::t_stop_reason p_reason) {
        if (p_reason==play_control::stop_reason_user || p_reason==play_control::stop_reason_shutting_down) {
            SendMessage(m_cda_window,WM_USER,static_cast<WPARAM>(HS_STOPPED),IPC_PLAYER_STATE_CHANGED_NOTIFICATION);
        }
    }

    void on_playback_pause(bool p_state) {
        HeliumState state=p_state?HS_PAUSED:HS_PLAYING;
        SendMessage(m_cda_window,WM_USER,static_cast<WPARAM>(state),IPC_PLAYER_STATE_CHANGED_NOTIFICATION);
    }

    void on_playback_new_track(metadb_handle_ptr p_track) {
        SendMessage(m_cda_window,WM_USER,0,IPC_TRACK_CHANGED_NOTIFICATION);
    }

    void on_playback_edited(metadb_handle_ptr p_track) {
        service_ptr_t<titleformat_object> script;

        // See <http://wiki.hydrogenaudio.org/index.php?title=Foobar2000:Titleformat_Reference>.
        pfc::string8 format=
            "$min($max(0,%rating%),5)"
        ;

        if (static_api_ptr_t<titleformat_compiler>()->compile(script,format)) {
            static_api_ptr_t<playback_control>()->playback_format_title_ex(p_track,NULL,format,script,NULL,playback_control::display_level_titles);
        }

        // Report the rating in range [0,5].
        int rating=atoi(format);
        SendMessage(m_cda_window,WM_USER,static_cast<WPARAM>(rating),IPC_RATING_CHANGED_NOTIFICATION);
    }

    #pragma warning(disable:4100)

    void on_playback_dynamic_info_track(file_info const& p_info) {
        SendMessage(m_cda_window,WM_USER,0,IPC_TRACK_CHANGED_NOTIFICATION);
    }

    #pragma warning(default:4100)

  private:

    static LRESULT CALLBACK WindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
        // Using a dynamic_cast here would be safer, but that requires RTTI support.
        CDArtDisplayInterface* _this=reinterpret_cast<CDArtDisplayInterface*>(GetWindowLong(hWnd,GWL_USERDATA));

        if (uMsg==WM_CREATE) {
            LPVOID params=reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams;
            _this=static_cast<CDArtDisplayInterface*>(params);
            SetWindowLongA(hWnd,GWL_USERDATA,(LONG)_this);
        }
        else if (uMsg==WM_DESTROY) {
            SendMessage(_this->m_cda_window,WM_USER,0,IPC_SHUTDOWN_NOTIFICATION);
        }
        else if (uMsg==WM_COPYDATA) {
            if (!_this) {
                return 0;
            }

            PCOPYDATASTRUCT cds=reinterpret_cast<PCOPYDATASTRUCT>(lParam);

            char buffer[MAX_PATH];
            ZeroMemory(buffer,sizeof(buffer));

            if (cds->cbData>=sizeof(buffer)) {
                cds->cbData=sizeof(buffer)-1;
            }
            strncpy_s(buffer,static_cast<char const*>(cds->lpData),cds->cbData);

            static_api_ptr_t<playlist_manager> plm;
            if (cds->dwData==IPC_ADDFILE_PLAY_PLAYLIST) {
                if (plm->activeplaylist_add_locations(pfc::list_single_ref_t<char const*>(buffer),false,_this->m_cda_window)) {
                    // Newly added files come last in the playlist.
                    t_size item=plm->activeplaylist_get_item_count()-1;

                    // TODO: Find a way to make this work if playback is not the default action.
                    return plm->activeplaylist_execute_default_action(item);
                }
                return 0;
            }
            else if (cds->dwData==IPC_ADDFILE_QUEUE_PLAYLIST) {
                return plm->activeplaylist_add_locations(pfc::list_single_ref_t<char const*>(buffer),false,_this->m_cda_window);
            }
        }
        else if (uMsg==WM_USER) {
            static_api_ptr_t<playback_control> pbc;
            static_api_ptr_t<playlist_manager> plm;

            switch (lParam) {
                case IPC_PLAY: {
                    pbc->start();
                    return 1;
                }
                case IPC_PLAYPAUSE: {
                    pbc->play_or_pause();
                    return 1;
                }
                case IPC_FORCEPAUSE: {
                    pbc->pause(true);
                    return 1;
                }
                case IPC_STOP: {
                    pbc->stop();
                    return 1;
                }
                case IPC_NEXT: {
                    pbc->start(playback_control::track_command_next);
                    return 1;
                }
                case IPC_PREVIOUS: {
                    pbc->start(playback_control::track_command_prev);
                    return 1;
                }

                case IPC_SET_VOLUME: {
                    // Get the volume scale in range [0,100].
                    LONG scale=static_cast<LONG>(wParam);

                    // Clamp due to mouse scroll wheel events.
                    if (scale<1) {
                        scale=1;
                    }
                    else if (scale>100) {
                        scale=100;
                    }

                    // Set the volume gain in dB. For some hints about the formula, see
                    // http://www.hydrogenaudio.org/forums/index.php?showtopic=47858
                    // NOTE: foobar2000 seems to use a factor of 3.0 here, but 2.5 maps
                    // more nicely 1 to -100.
                    audio_sample gain=scale_to_gain(scale/100.0)*2.5;
                    pbc->set_volume(static_cast<float>(gain));
                    return 1;
                }
                case IPC_GET_VOLUME: {
                    // Get the volume gain in range [-100,0].
                    float gain=pbc->get_volume();

                    // Return the volume scale.
                    audio_sample scale=audio_math::gain_to_scale(gain/2.5)*100;
                    return static_cast<LONG>(round_to_even(scale));
                }
                case IPC_GET_CURRENT_TRACK: {
                    metadb_handle_ptr track;
                    if (!_this || !pbc->get_now_playing(track)) {
                        return 0;
                    }

                    service_ptr_t<titleformat_object> script;

                    // See <http://wiki.hydrogenaudio.org/index.php?title=Foobar2000:Titleformat_Reference>.
                    pfc::string8 format1=
                        "[%title%]"                "\t"
                        "[%artist%]"               "\t"
                        "[%album%]"                "\t"
                        "[%genre%]"                "\t"
                        "[%date%]"                 "\t"
                        "[%comment%]"              "\t"
                        "$num(%tracknumber%,0)"    "\t"
                        "%length_seconds%"         "\t"
                        "%path%"                   "\t"
                        "$min($max(0,%rating%),5)"
                    ;
                    pfc::string8 format2=
                        "[%composer%]"             "\t"
                        "[%lyricist%]"             "\t"
                        "[%publisher%]"            "\t"
                        "[%conductor%]"            "\t"
                        "[%producer%]"             "\t"
                        "[%copyright%]"            "\t"
                        "[%bitrate%]"
                    ;

                    if (static_api_ptr_t<titleformat_compiler>()->compile(script,format1)) {
                        pbc->playback_format_title_ex(track,NULL,format1,script,NULL,playback_control::display_level_titles);
                    }
                    if (static_api_ptr_t<titleformat_compiler>()->compile(script,format2)) {
                        pbc->playback_format_title_ex(track,NULL,format2,script,NULL,playback_control::display_level_titles);
                    }

                    // Copy the information to a buffer.
                    char buffer[4096];
                    ZeroMemory(buffer,sizeof(buffer));

                    pfc::string_directory* cfg_cad_root=new pfc::string_directory(cfg_cad_path);

                    int result=_snprintf_s(
                        buffer,
                        _TRUNCATE,
                        "%s\t%s%s\t%s",
                        format1.get_ptr(),
                        (char const*)(*cfg_cad_root),"\\Skins\\Default\\nocover.png",
                        format2.get_ptr()
                    );
                    assert(result>0);

                    delete cfg_cad_root;

                    // Pass the buffer to CDA.
                    COPYDATASTRUCT cds;
                    cds.dwData=IPC_GET_CURRENT_TRACK;
                    cds.cbData=result;
                    cds.lpData=buffer;

                    return SendMessage(_this->m_cda_window,WM_COPYDATA,reinterpret_cast<WPARAM>(hWnd),reinterpret_cast<LPARAM>(&cds));
                }

                case IPC_GET_DURATION: {
                    return static_cast<LONG>(pbc->playback_get_length());
                }
                case IPC_SET_POSITION: {
                    pbc->playback_seek(static_cast<double>(wParam));
                    return static_cast<LONG>(pbc->playback_get_position());
                }
                case IPC_IS_PLAYING: {
                    return pbc->is_playing();
                }
                case IPC_IS_PAUSED: {
                    return pbc->is_paused();
                }
                case IPC_GET_LIST_LENGTH: {
                    return plm->activeplaylist_get_item_count();
                }
                case IPC_SET_LIST_POS: {
                    // TODO: Find a way to make this work if playback is not the default action.
                    return plm->activeplaylist_execute_default_action(static_cast<t_size>(wParam));
                }
                case IPC_GET_LIST_ITEM: {
                    metadb_handle_ptr track;
                    if (!_this || !plm->activeplaylist_get_item_handle(track,static_cast<t_size>(wParam))) {
                        return 0;
                    }

                    service_ptr_t<titleformat_object> script;

                    // See <http://wiki.hydrogenaudio.org/index.php?title=Foobar2000:Titleformat_Reference>.
                    pfc::string8 format=
                        "[%artist%]"               "\t"
                        "[%title%]"                "\t"
                        "%length_seconds%"         "\t"
                        "%path%"                   "\t"
                        "$min($max(0,%rating%),5)"
                    ;

                    if (static_api_ptr_t<titleformat_compiler>()->compile(script,format)) {
                        pbc->playback_format_title_ex(track,NULL,format,script,NULL,playback_control::display_level_titles);
                    }

                    // Copy the information to a buffer.
                    char buffer[4096];
                    ZeroMemory(buffer,sizeof(buffer));

                    int result=_snprintf_s(
                        buffer,
                        _TRUNCATE,
                        "%s",
                        format.get_ptr()
                    );
                    assert(result>0);

                    // Pass the buffer to CDA.
                    COPYDATASTRUCT cds;
                    cds.dwData=IPC_GET_LIST_ITEM;
                    cds.cbData=result;
                    cds.lpData=buffer;

                    return SendMessage(_this->m_cda_window,WM_COPYDATA,reinterpret_cast<WPARAM>(hWnd),reinterpret_cast<LPARAM>(&cds));
                }
                case IPC_SET_CALLBACK_HWND: {
                    if (!_this) {
                        return 0;
                    }
                    _this->m_cda_window=reinterpret_cast<HWND>(wParam);
                    return 1;
                }
                case IPC_GET_LIST_POS: {
                    return static_cast<LONG>(plm->activeplaylist_get_focus_item());
                }
                case IPC_GET_POSITION: {
                    return static_cast<LONG>(pbc->playback_get_position());
                }

                case IPC_SHOW_PLAYER_WINDOW: {
                    static_api_ptr_t<ui_control> uic;
                    if (uic->is_visible()) {
                        uic->hide();
                    }
                    else {
                        uic->activate();
                    }
                    return 1;
                }
                case IPC_GET_PLAYER_STATE: {
                    if (pbc->is_paused()) {
                        return HS_PAUSED;
                    }
                    if (pbc->is_playing()) {
                        return HS_PLAYING;
                    }
                    return HS_STOPPED;
                }

                case IPC_SET_REPEAT: {
                    GUID guid=plm->playback_order_get_guid(plm->playback_order_get_active());

                    // Cycle through all "Repeat" modes and "Default".
                    if (guid==ORDER_REPEAT_PLAYLIST) {
                        plm->playback_order_set_active(get_playback_order_index(plm,ORDER_REPEAT_TRACK));
                    }
                    else if (guid==ORDER_REPEAT_TRACK) {
                        plm->playback_order_set_active(get_playback_order_index(plm,ORDER_DEFAULT));
                    }
                    else {
                        plm->playback_order_set_active(get_playback_order_index(plm,ORDER_REPEAT_PLAYLIST));
                    }

                    return 1;
                }
                case IPC_GET_REPEAT: {
                    // Check if the current order mode is any of the "Repeat" modes.
                    GUID guid=plm->playback_order_get_guid(plm->playback_order_get_active());
                    return guid==ORDER_REPEAT_PLAYLIST || guid==ORDER_REPEAT_TRACK;
                }

                case IPC_CLOSE_PLAYER: {
                    standard_commands::main_exit();
                    return 1;
                }

                case IPC_SET_SHUFFLE: {
                    GUID guid=plm->playback_order_get_guid(plm->playback_order_get_active());

                    // Cycle through all "Shuffle" modes and "Default".
                    if (guid==ORDER_SHUFFLE_TRACKS) {
                        plm->playback_order_set_active(get_playback_order_index(plm,ORDER_SHUFFLE_ALBUMS));
                    }
                    else if (guid==ORDER_SHUFFLE_ALBUMS) {
                        plm->playback_order_set_active(get_playback_order_index(plm,ORDER_SHUFFLE_DIRECTORIES));
                    }
                    else if (guid==ORDER_SHUFFLE_DIRECTORIES) {
                        plm->playback_order_set_active(get_playback_order_index(plm,ORDER_DEFAULT));
                    }
                    else {
                        plm->playback_order_set_active(get_playback_order_index(plm,ORDER_SHUFFLE_TRACKS));
                    }

                    return 1;
                }
                case IPC_GET_SHUFFLE: {
                    // Check if the current order mode is any of the "Shuffle" modes.
                    GUID guid=plm->playback_order_get_guid(plm->playback_order_get_active());
                    return guid==ORDER_SHUFFLE_TRACKS || guid==ORDER_SHUFFLE_ALBUMS || guid==ORDER_SHUFFLE_DIRECTORIES;
                }

                // For compatibility with Helium.
                case IPC_SET_RATING:

                case IPC_RATING_CHANGED_NOTIFICATION: {
                    if (!cfg_write_rating) {
                        return 0;
                    }

                    int rating=static_cast<int>(wParam);
                    if (rating<0) {
                        rating=0;
                    }
                    else if (rating>5) {
                        rating=5;
                    }
                    char const rating_str[]={'0'+static_cast<char>(rating),'\0'};

                    metadb_handle_ptr track;
                    if (pbc->get_now_playing(track)) {
                        file_info_impl info;
                        if (track->get_info(info)) {
                            track->metadb_lock();
                            if (rating>0) {
                                info.meta_set("RATING",rating_str);
                            }
                            else {
                                info.meta_remove_field("RATING");
                            }
                            static_api_ptr_t<metadb_io_v2>()->update_info_async_simple(
                                pfc::list_single_ref_t<metadb_handle_ptr>(track),
                                pfc::list_single_ref_t<file_info const*>(&info),
                                core_api::get_main_window(),
                                metadb_io_v2::op_flag_delay_ui,
                                NULL
                            );
                            track->metadb_unlock();
                        }
                    }

                    return 1;
                }

                case IPC_GET_CURRENT_LYRICS: {
                    metadb_handle_ptr track;
                    file_info_impl info;
                    if (!_this || !plm->activeplaylist_get_item_handle(track,static_cast<t_size>(wParam)) || !track->get_info(info)) {
                        return 0;
                    }

                    // Copy the information to a buffer.
                    char buffer[16384];
                    ZeroMemory(buffer,sizeof(buffer));

                    char const* lyrics=info.meta_get("UNSYNCEDLYRICS",0);
                    t_size length=pfc::strcpy_utf8_truncate(lyrics,buffer,sizeof(buffer));

                    // Pass the buffer to CDA.
                    COPYDATASTRUCT cds;
                    cds.dwData=IPC_GET_CURRENT_LYRICS;
                    cds.cbData=length;
                    cds.lpData=buffer;

                    return SendMessage(_this->m_cda_window,WM_COPYDATA,reinterpret_cast<WPARAM>(hWnd),reinterpret_cast<LPARAM>(&cds));
                }

                default: {
                    return 0;
                }
            }
        }

        return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }

    static ATOM s_atom;
    static int s_instances;

    HWND m_dummy_window;
    HWND m_cda_window;
};

ATOM CDArtDisplayInterface::s_atom=0;
int CDArtDisplayInterface::s_instances=0;

static initquit_factory_t<CDArtDisplayInterface> foo_interface;
