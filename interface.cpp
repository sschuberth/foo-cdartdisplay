#include "component.h"

extern cfg_bool cfg_cad_start;
extern cfg_string cfg_cad_path;
extern cfg_bool cfg_write_rating;

DECLARE_COMPONENT_VERSION(
    FOO_PLUGIN_NAME,
    FOO_TARGET_CAD_VERSION " release " FOO_PLUGIN_RELEASE,
    "Message handling plug-in to interface with CD Art Display <http://www.closetosoftware.com/?s=cdartdisplay>.\n"
    "Compiled on " __DATE__ ", Copyright 2007 by eyebex <eyebex@threekings.tk>."
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
            cls.lpszClassName=FOO_WINDOW_CLASS_NAME;

            s_atom=RegisterClass(&cls);
            if (!s_atom) {
                MessageBox(core_api::get_main_window(),_T("Unable to register the window class."),FOO_PLUGIN_FILE,MB_OK|MB_ICONERROR);
                return;
            }
        }

        // Create a dummy window.
        m_dummy_window=CreateWindow(
        /* lpClassName  */ MAKEINTATOM(s_atom),
        /* lpWindowName */ FOO_PLUGIN_FILE,
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
            MessageBox(core_api::get_main_window(),_T("Unable to create the dummy window."),FOO_PLUGIN_FILE,MB_OK|MB_ICONERROR);
            return;
        }

        m_cda_window=NULL;
        ++s_instances;

#ifdef VERBOSE_MESSAGE_BOXES
        MessageBox(core_api::get_main_window(),_T("Construction was successful."),FOO_PLUGIN_FILE,MB_OK|MB_ICONINFORMATION);
#endif

        // If enabled, start CAD with foobar2000.
        if (cfg_cad_start) {
            if (WinExec(cfg_cad_path,SW_SHOWDEFAULT)<32) {
                MessageBox(core_api::get_main_window(),_T("Unable to launch CD Art Display."),FOO_PLUGIN_FILE,MB_OK|MB_ICONERROR);
            }
            else {
                // Wait at most 10 seconds for CAD to register itself.
                int i=100;
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
                    MessageBox(core_api::get_main_window(),_T("Timeout while waiting for the CD Art Display window to register itself."),FOO_PLUGIN_FILE,MB_OK|MB_ICONWARNING);
                }
            }
        }

        static_api_ptr_t<play_callback_manager>()->register_callback(
            this,
            play_callback::flag_on_playback_starting  |
            play_callback::flag_on_playback_stop      |
            play_callback::flag_on_playback_pause     |
            play_callback::flag_on_playback_new_track |
            play_callback::flag_on_playback_edited,
            false
        );
    }

    void on_quit() {
        static_api_ptr_t<play_callback_manager>()->unregister_callback(this);

        if (!DestroyWindow(m_dummy_window)) {
            MessageBox(core_api::get_main_window(),_T("Unable to destroy the dummy window."),FOO_PLUGIN_FILE,MB_OK|MB_ICONERROR);
            return;
        }

        if (--s_instances<=0) {
            if (!UnregisterClass(MAKEINTATOM(s_atom),NULL)) {
                MessageBox(core_api::get_main_window(),_T("Unable to unregister the window class."),FOO_PLUGIN_FILE,MB_OK|MB_ICONERROR);
                return;
            }

            s_atom=0;
        }

#ifdef VERBOSE_MESSAGE_BOXES
        MessageBox(core_api::get_main_window(),_T("Destruction was successful."),FOO_PLUGIN_FILE,MB_OK|MB_ICONINFORMATION);
#endif
    }

    // warning C4100: unreferenced formal parameter
    #pragma warning(disable:4100)

    void on_playback_seek(double p_time) {}
    void on_playback_dynamic_info(const file_info & p_info) {}
    void on_playback_dynamic_info_track(const file_info & p_info) {}
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
        file_info_impl info;
        if (p_track->get_info(info)) {
            // Report the rating in range [0,5].
            int rating=atoi(get_rating(info));
            if (rating<0) {
                rating=0;
            }
            else if (rating>5) {
                rating=5;
            }

            SendMessage(m_cda_window,WM_USER,static_cast<WPARAM>(rating),IPC_RATING_CHANGED_NOTIFICATION);
        }
    }

  private:

    static LRESULT CALLBACK WindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
        // Using a dynamic_cast here would be safer, but that requires RTTI support.
        CDArtDisplayInterface *_this=reinterpret_cast<CDArtDisplayInterface*>(GetWindowLong(hWnd,GWL_USERDATA));

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

            static char buffer[MAX_PATH];
            ZeroMemory(buffer,sizeof(buffer));

            if (cds->cbData>=sizeof(buffer)-1) {
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
                    LONG volume=static_cast<LONG>(wParam);

                    // Clamp due to mouse scroll wheel events.
                    if (volume<0) {
                        volume=0;
                    }
                    else if (volume>100) {
                        volume=100;
                    }

                    // Set the volume gain in dB.
                    pbc->set_volume(static_cast<float>(volume)-100.0f);
                    return 1;
                }
                case IPC_GET_VOLUME: {
                    // Get volume gain in range [-100,0].
                    float volume=pbc->get_volume();

                    // Return the volume scale.
                    return static_cast<LONG>(volume+100.0f);
                }
                case IPC_GET_CURRENT_TRACK: {
                    if (!_this) {
                        return 0;
                    }

                    // Copy the information to a buffer.
                    static char buffer[4096];
                    ZeroMemory(buffer,sizeof(buffer));

                    metadb_handle_ptr track;
                    if (pbc->get_now_playing(track)) {
                        file_info_impl info;
                        if (track->get_info(info)) {
                            // If there is no track number, do not return a NULL string.
                            static char number_buf[8];
                            ZeroMemory(number_buf,sizeof(number_buf));
                            char const* number=info.meta_get("TRACKNUMBER",0);
                            if (number) {
                                // Get rid of the total number of tracks if present.
                                _snprintf_s(number_buf,_TRUNCATE,"%d",atoi(number));
                            }
                            number=number_buf;

                            int length=static_cast<int>(pbc->playback_get_length());
                            char const* path=track->get_path();
                            if (pfc::string_find_first(path,"file://")==0) {
                                path+=sizeof("file://")-1;
                            }

                            // Report the rating in range [0,5].
                            int rating=atoi(get_rating(info));
                            if (rating<0) {
                                rating=0;
                            }
                            else if (rating>5) {
                                rating=5;
                            }

                            pfc::string_directory* cfg_cad_root=new pfc::string_directory(cfg_cad_path);

                            // See <http://wiki.hydrogenaudio.org/index.php?title=Foobar2000:ID3_Tag_Mapping>.
                            _snprintf_s(
                                buffer,
                                _TRUNCATE,
                                "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d\t%s\t%d\t%s%s\t%s\t%s\t%s\t%s\t%s\t%s",
                                get_meta(info,"TITLE"),
                                get_meta(info,"ARTIST"),
                                get_meta(info,"ALBUM"),
                                get_meta(info,"GENRE"),
                                get_meta(info,"DATE"),
                                get_meta(info,"COMMENT"),
                                number,
                                length,
                                path,
                                rating,
                                (char const*)(*cfg_cad_root),
                                "\\Skins\\Default\\nocover.png",
                                get_meta(info,"COMPOSER"),
                                get_meta(info,"LYRICIST"),
                                get_meta(info,"PUBLISHER"),
                                get_meta(info,"CONDUCTOR"),
                                get_meta(info,"PRODUCER"),
                                get_meta(info,"COPYRIGHT")
                            );

                            delete cfg_cad_root;
                        }
                    }

                    // Pass the buffer to CDA.
                    static COPYDATASTRUCT cds;
                    cds.dwData=IPC_GET_CURRENT_TRACK;
                    cds.cbData=sizeof(buffer);
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
                    if (!_this) {
                        return 0;
                    }

                    // Copy the information to a buffer.
                    static char buffer[4096];
                    ZeroMemory(buffer,sizeof(buffer));

                    metadb_handle_ptr track;
                    if (plm->activeplaylist_get_item_handle(track,static_cast<t_size>(wParam))) {
                        file_info_impl info;
                        if (track->get_info(info)) {
                            int length=static_cast<int>(pbc->playback_get_length());
                            char const* path=track->get_path();
                            if (pfc::string_find_first(path,"file://")==0) {
                                path+=sizeof("file://")-1;
                            }

                            // Report the rating in range [0,5].
                            int rating=atoi(get_rating(info));
                            if (rating<0) {
                                rating=0;
                            }
                            else if (rating>5) {
                                rating=5;
                            }

                            // See <http://wiki.hydrogenaudio.org/index.php?title=Foobar2000:ID3_Tag_Mapping>.
                            _snprintf_s(
                                buffer,
                                _TRUNCATE,
                                "%s\t%s\t%d\t%s\t%d",
                                get_meta(info,"ARTIST"),
                                get_meta(info,"TITLE"),
                                length,
                                path,
                                rating
                            );
                        }
                    }

                    // Pass the buffer to CDA.
                    static COPYDATASTRUCT cds;
                    cds.dwData=IPC_GET_LIST_ITEM;
                    cds.cbData=sizeof(buffer);
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
                    if (!_this) {
                        return 0;
                    }

                    // Copy the information to a buffer.
                    static char buffer[16384];
                    ZeroMemory(buffer,sizeof(buffer));

                    metadb_handle_ptr track;
                    if (plm->activeplaylist_get_item_handle(track,static_cast<t_size>(wParam))) {
                        file_info_impl info;
                        if (track->get_info(info)) {
                            char const* lyrics=info.meta_get("UNSYNCEDLYRICS",0);
                            pfc::strcpy_utf8_truncate(lyrics,buffer,sizeof(buffer)-1);
                        }
                    }

                    // Pass the buffer to CDA.
                    static COPYDATASTRUCT cds;
                    cds.dwData=IPC_GET_CURRENT_LYRICS;
                    cds.cbData=pfc::strlen_utf8(buffer);
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
