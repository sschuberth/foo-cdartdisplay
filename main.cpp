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
    IPC_AUTOENQUEUE_OPTIONS                    ,
    IPC_SET_REPEAT                             ,
    IPC_SHUTDOWN_NOTIFICATION                  , // Message to send to CAD.
    IPC_GET_REPEAT                             ,
    IPC_CLOSE_HELIUM                           ,

    IPC_RATING_CHANGED_NOTIFICATION       = 639, // Message to send to CAD.

    IPC_NEW_COVER_NOTIFICATION            = 800  // Message to send to CAD (ignored).
};

enum HeliumState {
    HS_STOPPED    ,
    HS_PLAYING    ,
    HS_PAUSED
};

// "Default" {BFC61179-49AD-4E95-8D60-A22706485505}
static GUID const ORDER_DEFAULT=
{ 0xBFC61179, 0x49AD, 0x4E95, { 0x8D, 0x60, 0xA2, 0x27, 0x06, 0x48, 0x55, 0x05 } };

// "Repeat (playlist)" {681CC6EA-60AE-4BF9-913B-BB5F4E864F2A}
static GUID const ORDER_REPEAT_PLAYLIST=
{ 0x681CC6EA, 0x60AE, 0x4BF9, { 0x91, 0x3B, 0xBB, 0x5F, 0x4E, 0x86, 0x4F, 0x2A } };

// "Repeat (track)" {4BF4B280-0BB4-4DD0-8E84-37C3209C3DA2}
static GUID const ORDER_REPEAT_TRACK=
{ 0x4BF4B280, 0x0BB4, 0x4DD0, { 0x8E, 0x84, 0x37, 0xC3, 0x20, 0x9C, 0x3D, 0xA2 } };

// "Shuffle (tracks)" {C5CF4A57-8C01-480C-B334-3619645ADA8B}
static GUID const ORDER_SHUFFLE_TRACKS=
{ 0xC5CF4A57, 0x8C01, 0x480C, { 0xB3, 0x34, 0x36, 0x19, 0x64, 0x5A, 0xDA, 0x8B } };

// "Shuffle (albums)" {499E0B08-C887-48C1-9CCA-27377C8BFD30}
static GUID const ORDER_SHUFFLE_ALBUMS=
{ 0x499E0B08, 0xC887, 0x48C1, { 0x9C, 0xCA, 0x27, 0x37, 0x7C, 0x8B, 0xFD, 0x30 } };

// "Shuffle (directories)" {83C37600-D725-4727-B53C-BDEFFE5F8DC7}
static GUID const ORDER_SHUFFLE_DIRECTORIES=
{ 0x83C37600, 0xD725, 0x4727, { 0xB5, 0x3C, 0xBD, 0xEF, 0xFE, 0x5F, 0x8D, 0xC7 } };

static t_size get_playback_order_index(static_api_ptr_t<playlist_manager>& plm,GUID const& guid)
{
    for (t_size i=0;i<plm->playback_order_get_count();++i) {
        if (plm->playback_order_get_guid(i)==guid) {
            return i;
        }
    }
    return infinite;
}

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
        }
        else {
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
            // Map rating from range [0,5] to [0,10].
            int rating=atoi(get_rating(info))*2;
            SendMessage(m_cda_window,WM_USER,static_cast<WPARAM>(rating),IPC_RATING_CHANGED_NOTIFICATION);
        }
    }

  private:

    static char const* get_meta(file_info_impl const& info,char const* name) {
        static char const empty[]="\0";
        char const* value=info.meta_get(name,0);
        if (!value) {
            value=empty;
        }
        return value;
    }

    static char const* get_rating(file_info_impl const& info) {
        static char const empty[]="0";
        char const* rating=info.meta_get("RATING",0);
        if (!rating) {
            rating=info.meta_get("TRACKRATING",0);
        }
        if (!rating) {
            rating=info.meta_get("ALBUMRATING",0);
        }
        if (!rating) {
            rating=empty;
        }
        return rating;
    }

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
            play_callback::flag_on_playback_starting  |
            play_callback::flag_on_playback_stop      |
            play_callback::flag_on_playback_pause     |
            play_callback::flag_on_playback_new_track |
            play_callback::flag_on_playback_edited,
            false
        );
    }
    else if (uMsg==WM_DESTROY) {
        static_api_ptr_t<play_callback_manager> pcm;
        pcm->unregister_callback(_this);
        SendMessage(_this->m_cda_window,WM_USER,0,IPC_SHUTDOWN_NOTIFICATION);
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
                double volume=static_cast<float>(wParam);

                // Clamp due to mouse scroll wheel events.
                if (volume<0) {
                    volume=0;
                }
                else if (volume>100) {
                    volume=100;
                }

                // Set the volume gain in dB.
                pbc->set_volume(volume-100.0f);
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
                        char const* path=track->get_path()+sizeof("file://")-1;

                        // Map rating from range [0,5] to [0,10].
                        int rating=atoi(get_rating(info))*2;

                        // TODO: Think about making this an option in the GUI.
                        char const* cover="";

                        // See <http://wiki.hydrogenaudio.org/index.php?title=Foobar2000:ID3_Tag_Mapping>.
                        _snprintf_s(
                            buffer,
                            _TRUNCATE,
                            "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d\t%s\t%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s",
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
                            cover,
                            get_meta(info,"COMPOSER"),
                            get_meta(info,"LYRICIST"),
                            get_meta(info,"PUBLISHER"),
                            get_meta(info,"CONDUCTOR"),
                            get_meta(info,"PRODUCER"),
                            get_meta(info,"COPYRIGHT")
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

                metadb_handle_ptr track;
                if (plm->activeplaylist_get_item_handle(track,static_cast<t_size>(wParam))) {
                    file_info_impl info;
                    if (track->get_info(info)) {
                        int length=static_cast<int>(pbc->playback_get_length());
                        char const* path=track->get_path()+sizeof("file://")-1;

                        // Map rating from range [0,5] to [0,255].
                        int rating=atoi(get_rating(info))*51;

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

            case IPC_AUTOENQUEUE_OPTIONS: {
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

            case IPC_CLOSE_HELIUM: {
                static_api_ptr_t<user_interface>()->shutdown();
                return 1;
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
