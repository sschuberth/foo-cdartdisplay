#include <shlobj.h>

#include "component.h"
#include "resource.h"

#define DEFAULT_CAD_START   true
#define DEFAULT_CAD_PATH    get_registry_string(HKEY_CURRENT_USER,_T("Software\\CD Art Display"))
#define DEFAULT_COVER_PATH  get_folder_path(CSIDL_MYPICTURES)

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

static char const* get_folder_path(int folder) {
    TCHAR path[MAX_PATH];
    SHGetFolderPath(NULL,folder,NULL,SHGFP_TYPE_CURRENT,path);

    static pfc::stringcvt::string_utf8_from_wide path_utf8;
    path_utf8.convert(path);
    return path_utf8;
}

// {7cef938b-1b5f-4fe5-a8ca-a1711f6de31c}
static GUID const cfg_cad_start_guid=
{ 0x7cef938b, 0x1b5f, 0x4fe5, { 0xa8, 0xca, 0xa1, 0x71, 0x1f, 0x6d, 0xe3, 0x1c } };
static cfg_bool cfg_cad_start(cfg_cad_start_guid,DEFAULT_CAD_START);

// {33b397c7-8e79-4302-b5af-058a269d0e4d}
static GUID const cfg_cad_path_guid=
{ 0x33b397c7, 0x8e79, 0x4302, { 0xb5, 0xaf, 0x05, 0x8a, 0x26, 0x9d, 0x0e, 0x4d } };
static cfg_string cfg_cad_path(cfg_cad_path_guid,DEFAULT_CAD_PATH);

// {68fb33a2-261f-4280-9029-3b02fe2d75f8}
static GUID const cfg_cover_path_guid=
{ 0x68fb33a2, 0x261f, 0x4280, { 0x90, 0x29, 0x3b, 0x02, 0xfe, 0x2d, 0x75, 0xf8 } };
static cfg_string cfg_cover_path(cfg_cover_path_guid,DEFAULT_COVER_PATH);

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
        cfg_cover_path=DEFAULT_COVER_PATH;
    }

    bool get_help_url(pfc::string_base & p_out) {
        p_out="http://closetosoftware.com/forum/";
        return true;
    }

  private:

    static int CALLBACK WindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
        switch (uMsg) {
            case WM_INITDIALOG: {
                HWND item;

                item=GetDlgItem(hWnd,IDC_CAD_START);
                uSendMessage(item,BM_SETCHECK,cfg_cad_start,0);

                static pfc::stringcvt::string_os_from_utf8 path2os;

                path2os.convert(cfg_cad_path);
                item=GetDlgItem(hWnd,IDC_CAD_PATH);
                uSendMessage(item,WM_SETTEXT,0,reinterpret_cast<LPARAM>(path2os.get_ptr()));

                path2os.convert(cfg_cover_path);
                item=GetDlgItem(hWnd,IDC_COVER_PATH);
                uSendMessage(item,WM_SETTEXT,0,reinterpret_cast<LPARAM>(path2os.get_ptr()));

                return 1;
            }
            case WM_NOTIFY: {
                cfg_cad_start=uSendDlgItemMessage(hWnd,IDC_CAD_START,BM_GETCHECK,0,0)!=0;

                EnableWindow(GetDlgItem(hWnd,IDC_CAD_PATH),cfg_cad_start);
                // TODO: Fix edit control redraw issue.

                return 0;
            }
            case WM_COMMAND: {
                static pfc::stringcvt::string_utf8_from_os path2utf8;
                TCHAR path[MAX_PATH];

                if (uSendDlgItemMessage(hWnd,IDC_CAD_PATH,WM_GETTEXT,MAX_PATH,reinterpret_cast<LPARAM>(path))>0) {
                    path2utf8.convert(path);
                    cfg_cad_path=path2utf8;
                }

                if (uSendDlgItemMessage(hWnd,IDC_COVER_PATH,WM_GETTEXT,MAX_PATH,reinterpret_cast<LPARAM>(path))>0) {
                    path2utf8.convert(path);
                    cfg_cover_path=path2utf8;
                }

                return 0;
            }
        }
        return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }
};

static preferences_page_factory_t<CDArtDisplayPreferences> foo_preferences;
