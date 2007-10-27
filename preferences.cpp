#include <shlobj.h>

#include "component.h"
#include "resource.h"

#define DEFAULT_CAD_START     true
#define DEFAULT_CAD_PATH      get_registry_string(HKEY_CURRENT_USER,_T("Software\\CD Art Display"))
#define DEFAULT_WRITE_RATING  false

// Returns a registry key's default value in UTF-8.
static char const* get_registry_string(HKEY key,LPCTSTR subkey) {
    TCHAR path[MAX_PATH];
    LONG size=MAX_PATH;
    if (RegQueryValue(key,subkey,path,&size)!=ERROR_SUCCESS) {
        return NULL;
    }

    static pfc::stringcvt::string_utf8_from_wide path_utf8;
    path_utf8.convert(path);
    return path_utf8;
}

// {7cef938b-1b5f-4fe5-a8ca-a1711f6de31c}
static GUID const cfg_cad_start_guid=
{ 0x7cef938b, 0x1b5f, 0x4fe5, { 0xa8, 0xca, 0xa1, 0x71, 0x1f, 0x6d, 0xe3, 0x1c } };
cfg_bool cfg_cad_start(cfg_cad_start_guid,DEFAULT_CAD_START);

// {33b397c7-8e79-4302-b5af-058a269d0e4d}
static GUID const cfg_cad_path_guid=
{ 0x33b397c7, 0x8e79, 0x4302, { 0xb5, 0xaf, 0x05, 0x8a, 0x26, 0x9d, 0x0e, 0x4d } };
cfg_string cfg_cad_path(cfg_cad_path_guid,DEFAULT_CAD_PATH);

// {453c4714-7147-42a2-bae2-da0a6935a707}
static GUID const cfg_write_rating_guid=
{ 0x453c4714, 0x7147, 0x42a2, { 0xba, 0xe2, 0xda, 0x0a, 0x69, 0x35, 0xa7, 0x07 } };
cfg_bool cfg_write_rating(cfg_write_rating_guid,DEFAULT_WRITE_RATING);

class CDArtDisplayPreferences:public preferences_page
{
  public:

    HWND create(HWND p_parent) {
        return uCreateDialog(IDD_CONFIG,p_parent,WindowProc);
    }

    const char * get_name() {
        static char const* name="CD Art Display Interface";
        return name;
    }

    GUID get_guid() {
        // {8ae21164-7867-4fc0-9545-f5fe97dfe896}
        static GUID const guid=
            { 0x8ae21164, 0x7867, 0x4fc0, { 0x95, 0x45, 0xf5, 0xfe, 0x97, 0xdf, 0xe8, 0x96 } };
        return guid;
    }

    GUID get_parent_guid() {
        return guid_tools;
    }

    bool reset_query() {
        return true;
    }

    void reset() {
        cfg_cad_start=DEFAULT_CAD_START;
        cfg_cad_path=DEFAULT_CAD_PATH;
        cfg_write_rating=DEFAULT_WRITE_RATING;
    }

    bool get_help_url(pfc::string_base & p_out) {
        p_out="http://closetosoftware.com/forum/";
        return true;
    }

  private:

    static int CALLBACK WindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
        switch (uMsg) {
            case WM_INITDIALOG: {
                // Set the check box state according to the stored configuration.
                uSendDlgItemMessage(hWnd,IDC_CAD_START,BM_SETCHECK,cfg_cad_start,0);
                EnableWindow(GetDlgItem(hWnd,IDC_CAD_PATH),cfg_cad_start);

                // Get the foobar2000 configuration strings and convert them to
                // the OS' format.
                static pfc::stringcvt::string_os_from_utf8 path2os;
                path2os.convert(cfg_cad_path);

                uSendDlgItemMessage(hWnd,IDC_CAD_PATH,WM_SETTEXT,0,reinterpret_cast<LPARAM>(path2os.get_ptr()));

                // Set the stored configuration for writing the rating to tags.
                uSendDlgItemMessage(hWnd,IDC_WRITE_RATING,BM_SETCHECK,cfg_write_rating,0);

                return TRUE;
            }
            case WM_COMMAND: {
                // Get all edit control string in the OS' format and convert them
                // to UTF-8 for foobar2000.
                static pfc::stringcvt::string_utf8_from_os path2utf8;

                TCHAR path[MAX_PATH];
                LRESULT result;

                switch (LOWORD(wParam)) {
                    case IDC_CAD_START: {
                        // Get the check box state and toggle the edit control accordingly.
                        cfg_cad_start=uSendDlgItemMessage(hWnd,IDC_CAD_START,BM_GETCHECK,0,0)!=0;
                        EnableWindow(GetDlgItem(hWnd,IDC_CAD_PATH),cfg_cad_start);
                        break;
                    }
                    case IDC_CAD_PATH: {
                        if (HIWORD(wParam)==EN_CHANGE) {
                            result=uSendDlgItemMessage(hWnd,IDC_CAD_PATH,WM_GETTEXT,MAX_PATH,reinterpret_cast<LPARAM>(path));
                            if (result==0) {
                                cfg_cad_path.reset();
                            }
                            else if (result>0) {
                                path2utf8.convert(path);
                                cfg_cad_path=path2utf8;
                            }
                        }
                        break;
                    }
                    case IDC_WRITE_RATING: {
                        // Get the check box state and toggle the edit control accordingly.
                        cfg_write_rating=uSendDlgItemMessage(hWnd,IDC_WRITE_RATING,BM_GETCHECK,0,0)!=0;
                        break;
                    }
                    default: {
                        return 1;
                    }
                }

                return 0;
            }
        }
        return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }
};

static preferences_page_factory_t<CDArtDisplayPreferences> foo_preferences;
