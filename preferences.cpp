#include "component.h"
#include "resource.h"

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
        // {8AE21164-7867-4fc0-9545-F5FE97DFE896}
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
        // TODO: Write default values to settings.
    }

    bool get_help_url(pfc::string_base & p_out) {
        p_out="http://closetosoftware.com/forum/";
        return true;
    }

  private:

    static int CALLBACK WindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
        return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }
};

static preferences_page_factory_t<CDArtDisplayPreferences> foo_preferences;
