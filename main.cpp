// Get rid of tons of warnings.
#pragma warning(push,1)
#include <foobar2000/SDK/foobar2000.h>
#pragma warning(pop)

#define FOO_CAD_VERSION     "1.0"
#define FOO_PLUGIN_RELEASE  "1"
#define FOO_PLUGIN_NAME     _T("foo_cdartdisplay")
#define FOO_CLASS_NAME      _T("THeliumMainForm")

DECLARE_COMPONENT_VERSION(
    "CD Art Display Interface",
    FOO_CAD_VERSION " release " FOO_PLUGIN_RELEASE,
    "Message handling plug-in to interface with CD Art Display <http://www.closetosoftware.com/?s=cdartdisplay>.\n"
    "Compiled on " __DATE__ ", copyright 2007 by eyebex <eyebex@threekings.tk>."
);

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

    IPC_RATING_CHANGED_NOTIFICATION       = 639, // Message to send to CAD.

    IPC_NEW_COVER_NOTIFICATION            = 800  // Message to send to CAD (ignored).
};

enum HeliumState {
    HS_STOPPED    ,
    HS_PLAYING    ,
    HS_PAUSED
};

class CDArtDisplayInterface:public initquit,public play_callback
{
  public:

    void on_init() {
        if (!s_atom) {
            // Register a minimal window class.
            WNDCLASS cls;
            memset(&cls,0,sizeof(cls));
            cls.style=CS_OWNDC|CS_SAVEBITS;
            cls.lpfnWndProc=WindowProc;
            cls.hInstance=core_api::get_my_instance();
            cls.hCursor=LoadCursor(NULL,IDC_ARROW);
            cls.lpszClassName=FOO_CLASS_NAME;

            s_atom=RegisterClass(&cls);
            if (!s_atom) {
#ifndef NDEBUG
                MessageBox(core_api::get_main_window(),_T("Unable to register the window class."),FOO_PLUGIN_NAME,MB_OK|MB_ICONERROR);
#endif
                return;
            }
        }

        // Create a dummy window.
        m_dummy_window=CreateWindow(
        /* lpClassName  */ MAKEINTATOM(s_atom),
        /* lpWindowName */ FOO_PLUGIN_NAME,
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
#ifndef NDEBUG
            MessageBox(core_api::get_main_window(),_T("Unable to create the dummy window."),FOO_PLUGIN_NAME,MB_OK|MB_ICONERROR);
#endif
            return;
        }

        m_cda_window=NULL;
        ++s_instances;

#ifndef NDEBUG
        MessageBox(core_api::get_main_window(),_T("Construction was successful."),FOO_PLUGIN_NAME,MB_OK|MB_ICONINFORMATION);
#endif
    }

    void on_quit() {
        if (!DestroyWindow(m_dummy_window)) {
#ifndef NDEBUG
            MessageBox(core_api::get_main_window(),_T("Unable to destroy the dummy window."),FOO_PLUGIN_NAME,MB_OK|MB_ICONERROR);
#endif
            return;
        }

        if (--s_instances<=0) {
            if (!UnregisterClass(MAKEINTATOM(s_atom),NULL)) {
#ifndef NDEBUG
                MessageBox(core_api::get_main_window(),_T("Unable to unregister the window class."),FOO_PLUGIN_NAME,MB_OK|MB_ICONERROR);
#endif
                return;
            }

            s_atom=0;
        }

#ifndef NDEBUG
        MessageBox(core_api::get_main_window(),_T("Destruction was successful."),FOO_PLUGIN_NAME,MB_OK|MB_ICONINFORMATION);
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
        } else {
            if (p_command==play_control::track_command_play || p_command==play_control::track_command_resume) {
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
            // TODO: Find out how to get the rating.
            int rating=atoi(info.meta_get("RATING",0));
            SendMessage(m_cda_window,WM_USER,static_cast<WPARAM>(rating),IPC_RATING_CHANGED_NOTIFICATION);
        }
    }

  private:

    static ATOM s_atom;
    static int s_instances;

    HWND m_dummy_window;
    HWND m_cda_window;

    static LRESULT CALLBACK WindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
};

LRESULT CALLBACK CDArtDisplayInterface::WindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
    // Using a dynamic_cast here would be safer, but that requires RTTI support.
    CDArtDisplayInterface *_this=reinterpret_cast<CDArtDisplayInterface*>(GetWindowLong(hWnd,GWL_USERDATA));

    if (uMsg==WM_CREATE) {
        LPVOID params=reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams;
        _this=static_cast<CDArtDisplayInterface*>(params);
        SetWindowLongA(hWnd,GWL_USERDATA,(LONG)_this);

        static_api_ptr_t<play_callback_manager> pcm;
        pcm->register_callback(
            _this,
            play_callback::flag_on_playback_starting |
            play_callback::flag_on_playback_stop |
            play_callback::flag_on_playback_pause |
            play_callback::flag_on_playback_new_track |
            play_callback::flag_on_playback_edited,
            false
        );
    }
    else if (uMsg==WM_DESTROY) {
        static_api_ptr_t<play_callback_manager> pcm;
        pcm->unregister_callback(_this);
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
                // Get the volume scale factor in range ]0,100].
                double scale=static_cast<double>(wParam)/100.0;

                // Clamp the volume to valid input for log10().
                if (scale<=0.0) {
                    scale=1.0e-5;
                } else if (scale>1.0) {
                    scale=1.0;
                }

                pbc->set_volume(static_cast<float>(20.0*log10(scale)));
                return 1;
            }
            case IPC_GET_VOLUME: {
                // Get volume gain in dB in range [-100,0].
                float db=pbc->get_volume();
                return static_cast<LONG>(audio_math::gain_to_scale(db)*100.0);
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
                        char const* title=info.meta_get("TITLE",0);
                        char const* artist=info.meta_get("ARTIST",0);
                        char const* album=info.meta_get("ALBUM",0);
                        char const* genre=info.meta_get("GENRE",0);
                        char const* year=info.meta_get("DATE",0);
                        char const* comment=info.meta_get("COMMENT",0);

                        // TODO: Think about making this an option in the GUI.
                        int number=atoi(info.meta_get("TRACKNUMBER",0));

                        int length=static_cast<int>(pbc->playback_get_length());
                        char const* path=track->get_path()+sizeof("file://")-1;

                        // TODO: Find out how to get the rating.
                        char const* rating=info.meta_get("RATING",0);

                        // TODO: Think about making this an option in the GUI.
                        char const* covers="";

                        _snprintf_s(
                            buffer,
                            _TRUNCATE,
                            "%s\t%s\t%s\t%s\t%s\t%s\t%d\t%d\t%s\t%s\t%s",
                            title,artist,album,genre,
                            year,comment,number,length,
                            path,
                            rating,
                            covers
                        );
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
                plm->activeplaylist_execute_default_action(static_cast<t_size>(wParam));
                return 1;
            }
            case IPC_GET_LIST_ITEM: {
                if (!_this) {
                    return 0;
                }

                // Copy the information to a buffer.
                static char buffer[4096];
                ZeroMemory(buffer,sizeof(buffer));

                // TODO: Copy song information to buffer.

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
                static_api_ptr_t<user_interface>()->activate();
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

            default: {
                return 0;
            }
        }
    }

    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

ATOM CDArtDisplayInterface::s_atom=0;
int CDArtDisplayInterface::s_instances=0;

static initquit_factory_t<CDArtDisplayInterface> foo_initquit;
