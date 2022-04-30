const u64 MIT_COOKIE_SIZE = 16;
CStringView MIT_COOKIE_PROTOCOL_NAME = "MIT-MAGIC-COOKIE-1";

enum X11Endianness : char
{
    X11EndiannessLittle = 'l',
    X11EndiannessBig = 'B',
};

struct X11ConnectionRequest
{
    X11Endianness order;
    byte padding_1;
    u16 protocol_major_version;
    u16 protocol_minor_version;
    u16 authorization_protocol_name_size;
    u16 authorization_protocol_data_size;
    char padding_2[2];
    char authorization_protocol_name[20];
    char authorization_protocol_data[MIT_COOKIE_SIZE]; // this isn't exactly right, but it's simple
};

enum X11ConnectionStatus : u8
{
    X11ConnectionStatusFailed = 0,
    X11ConnectionStatusSuccess = 1,
    X11ConnectionStatusAuthenticate = 2,
};

struct X11ConnectionResponseHeader
{
    X11ConnectionStatus status;
    byte padding;
    u16 major_version;
    u16 minor_version;
    u16 body_size_in_dwords; // multiply by 4 to get size in bytes
};

struct X11ConnectionResponseBodyInitial
{
    // TODO: better names + what do these fields even mean???
    u32 release;
    u32 base_id;
    u32 id_mask;
    u32 motion_buffer_size;
    u16 vendor_len;
    u16 request_max;
    u8 num_screens;
    u8 num_pixmap_formats;
    u8 image_byte_order;
    u8 bitmap_bit_order;
    u8 scanline_unit;
    u8 scanline_pad;
    u8 keycode_min;
    u8 keycode_max;
    byte padding[4];
};

enum X11RequestType : s8
{
    X11RequestTypeCreateWindow = 1,
    X11RequestTypeMapWindow = 8,
    X11RequestTypeCreateGraphicsContext = 55,
    X11RequestTypePutImage = 72,
};

// not to be confused with event type, this is used when setting window attributes
enum X11EventMark : u32
{
    X11EventMarkKeyPress = 0x00000001,
    X11EventMarkKeyRelease = 0x00000002,
    X11EventMarkButtonPress = 0x00000004,
    X11EventMarkButtonRelease = 0x00000008,
    X11EventMarkEnterWindow = 0x00000010,
    X11EventMarkLeaveWindow = 0x00000020,
    X11EventMarkPointerMotion = 0x00000040,
    X11EventMarkPointerMotionHint = 0x00000080,
    X11EventMarkButton1Motion = 0x00000100,
    X11EventMarkButton2Motion = 0x00000200,
    X11EventMarkButton3Motion = 0x00000400,
    X11EventMarkButton4Motion = 0x00000800,
    X11EventMarkButton5Motion = 0x00001000,
    X11EventMarkButtonMotion = 0x00002000,
    X11EventMarkKeymapState = 0x00004000,
    X11EventMarkExposure = 0x00008000,
    X11EventMarkVisibilityChange = 0x00010000,
    X11EventMarkStructureNotify = 0x00020000,
    X11EventMarkResizeRedirect = 0x00040000,
    X11EventMarkSubstructureNotify = 0x00080000,
    X11EventMarkSubstructureRedirect = 0x00100000,
    X11EventMarkFocusChange = 0x00200000,
    X11EventMarkPropertyChange = 0x00400000,
    X11EventMarkColormapChange = 0x00800000,
    X11EventMarkOwnerGrabButton = 0x01000000,
    // the other bits must be set to zero
};

static inline X11EventMark operator|(X11EventMark left, X11EventMark right)
{
    return (X11EventMark)((u32)left | (u32)right);
}

const u32 X11_VISUAL_ID_COPY_FROM_PARENT = 0;

enum X11WindowClass : u16
{
    X11WindowClassCopyFromParent = 0,
    X11WindowClassInputOutput = 1,
    X11WindowClassInputOnly = 2,
};

enum X11WindowAttribute : u32
{
    X11WindowAttributeBackgroundPixmap = 0x00000001,
    X11WindowAttributeBackgroundPixel = 0x00000002,
    X11WindowAttributeBorderPixmap = 0x00000004,
    X11WindowAttributeBorderPixel = 0x00000008,
    X11WindowAttributeBitGravity = 0x00000010,
    X11WindowAttributeWinGravity = 0x00000020,
    X11WindowAttributeBackingStore = 0x00000040,
    X11WindowAttributeBackingPlanes = 0x00000080,
    X11WindowAttributeBackingPixel = 0x00000100,
    X11WindowAttributeOverrideRedirect = 0x00000200,
    X11WindowAttributeSaveUnder = 0x00000400,
    X11WindowAttributeEventMask = 0x00000800,
    X11WindowAttributeDoNotPropagateMask = 0x00001000,
    X11WindowAttributeColormap = 0x00002000,
    X11WindowAttributeCursor = 0x00004000,
};

static inline X11WindowAttribute operator|(X11WindowAttribute left, X11WindowAttribute right)
{
    return (X11WindowAttribute)((u32)left | (u32)right);
}

struct X11CreateWindowRequestHeader
{
    X11RequestType type;
    u8 depth;
    u16 request_size_in_dwords;
    u32 window_id;
    u32 parent_id;
    s16 position_x;
    s16 position_y;
    u16 width;
    u16 height;
    u16 border_width;
    X11WindowClass window_class;
    u32 visual_id;
    X11WindowAttribute value_mask;
};

struct X11MapWindowRequest
{
    X11RequestType type;
    byte UNUSED;
    u16 request_size_in_dwords;
    u32 window_id;
};

struct X11CreateGraphicsContextRequest
{
    X11RequestType type;
    byte UNUSED = 0xd0;
    u16 request_size_in_dwords;
    u32 graphics_context_id;
    u32 drawable_id;
    u32 value_mask;
};

enum X11ImageFormat : u8
{
    X11ImageFormatBitmap = 0,
    X11ImageFormatXYPixmap = 1,
    X11ImageFormatZPixmap = 2,
};

struct X11PutImageRequestHeader
{
    X11RequestType type;
    X11ImageFormat format;
    u16 request_size_in_dwords;
    u32 drawable_id;
    u32 graphics_context_id;
    u16 width;
    u16 height;
    s16 position_x;
    s16 position_y;
    u8 left_pad;
    u8 depth;
    byte UNUSED[2];
};

enum X11EventType : u8
{
    X11EventTypeButtonPress = 4,
    X11EventTypeExpose = 12,
};

struct X11EventExpose
{
    X11EventType type;
    byte unused1;
    u16 sequence_number;
    u32 window_id;
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    u16 count;
    byte unused2[14];
};

u64 x11_calculate_padding(u64 value)
{
    return (4 - (value % 4)) % 4;
}
