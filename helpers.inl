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
    return static_cast<t_size>(infinite);
}

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
