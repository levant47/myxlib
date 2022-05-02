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
    X11EventTypeKeyPress = 2,
    X11EventTypeButtonPress = 4,
    X11EventTypeExpose = 12,
};

struct X11Event
{
    X11EventType type;
    byte data[31];

    void print_debug()
    {
        for (u64 k = 0; k < sizeof(*this); k++)
        {
            print((u64)((byte*)this)[k], " ");
        }
        print("\n");
    }
};

// https://gist.github.com/rickyzhang82/8581a762c9f9fc6ddb8390872552c250
enum X11KeyCode : u8
{
    X11KeyCodeEsc = 9,
    X11KeyCodeF1 = 67,
    X11KeyCodeF2 = 68,
    X11KeyCodeF3 = 69,
    X11KeyCodeF4 = 70,
    X11KeyCodeF5 = 71,
    X11KeyCodeF6 = 72,
    X11KeyCodeF7 = 73,
    X11KeyCodeF8 = 74,
    X11KeyCodeF9 = 75,
    X11KeyCodeF10 = 76,
    X11KeyCodeF11 = 95,
    X11KeyCodeF12 = 96,
    X11KeyCodePrintScrn = 111,
    X11KeyCodeScrollLock = 78,
    X11KeyCodePause = 110,
    X11KeyCodeBacktick = 49,
    X11KeyCode1 = 10,
    X11KeyCode2 = 11,
    X11KeyCode3 = 12,
    X11KeyCode4 = 13,
    X11KeyCode5 = 14,
    X11KeyCode6 = 15,
    X11KeyCode7 = 16,
    X11KeyCode8 = 17,
    X11KeyCode9 = 18,
    X11KeyCode0 = 19,
    X11KeyCodeDash = 20,
    X11KeyCodeEquals = 21,
    X11KeyCodeBackspace = 22,
    X11KeyCodeInsert = 106,
    X11KeyCodeHome = 97,
    X11KeyCodePageUp = 99,
    X11KeyCodeNumLock = 77,
    X11KeyCodeTab = 23,
    X11KeyCodeQ = 24,
    X11KeyCodeW = 25,
    X11KeyCodeE = 26,
    X11KeyCodeR = 27,
    X11KeyCodeT = 28,
    X11KeyCodeY = 29,
    X11KeyCodeU = 30,
    X11KeyCodeI = 31,
    X11KeyCodeO = 32,
    X11KeyCodeP = 33,
    X11KeyCodeOpenBracket = 34,
    X11KeyCodeCloseBracket = 35,
    X11KeyCodeReturn = 36,
    X11KeyCodeDelete = 107,
    X11KeyCodeEnd = 103,
    X11KeyCodePageDown = 105,
    X11KeyCodeA = 38,
    X11KeyCodeS = 39,
    X11KeyCodeD = 40,
    X11KeyCodeF = 41,
    X11KeyCodeG = 42,
    X11KeyCodeH = 43,
    X11KeyCodeJ = 44,
    X11KeyCodeK = 45,
    X11KeyCodeL = 46,
    X11KeyCodeSemicolon = 47,
    X11KeyCodeSingleQuote = 48,
    X11KeyCodeShiftLeft = 50,
    X11KeyCodeZ = 52,
    X11KeyCodeX = 53,
    X11KeyCodeC = 54,
    X11KeyCodeV = 55,
    X11KeyCodeB = 56,
    X11KeyCodeN = 57,
    X11KeyCodeM = 58,
    X11KeyCodeComma = 59,
    X11KeyCodeDot = 60,
    X11KeyCodeSlash = 61,
    X11KeyCodeShiftRight = 62,
    X11KeyCodeBackslash = 51,
    X11KeyCodeCtrlLeft = 37,
    X11KeyCodeSpace = 65,
    X11KeyCodeCtrlRight = 109,
};

enum X11ModifierKey : u16
{
    X11ModifierKeyShift = 0x0001,
    X11ModifierKeyLock = 0x0002,
    X11ModifierKeyControl = 0x0004,
    X11ModifierKeyMod1 = 0x0008,
    X11ModifierKeyMod2 = 0x0010,
    X11ModifierKeyMod3 = 0x0020,
    X11ModifierKeyMod4 = 0x0040,
    X11ModifierKeyMod5 = 0x0080,
    X11ModifierKeyButton1 = 0x0100,
    X11ModifierKeyButton2 = 0x0200,
    X11ModifierKeyButton3 = 0x0400,
    X11ModifierKeyButton4 = 0x0800,
    X11ModifierKeyButton5 = 0x1000,
};

struct X11EventKeyPress
{
    X11EventType type;
    X11KeyCode key_code;
    u16 sequence_number;
    u32 time;
    u32 window_id;
    u32 event;
    u32 child;
    s16 root_x;
    s16 root_y;
    s16 event_x;
    s16 event_y;
    X11ModifierKey state; // SETofKEYBUTMASK
    bool same_screen;
    byte unused;

    Option<char> to_char()
    {
        if (state == 0)
        {
            switch (key_code)
            {
                case X11KeyCodeA: return Option<char>::construct('a');
                case X11KeyCodeB: return Option<char>::construct('b');
                case X11KeyCodeC: return Option<char>::construct('c');
                case X11KeyCodeD: return Option<char>::construct('d');
                case X11KeyCodeE: return Option<char>::construct('e');
                case X11KeyCodeF: return Option<char>::construct('f');
                case X11KeyCodeG: return Option<char>::construct('g');
                case X11KeyCodeH: return Option<char>::construct('h');
                case X11KeyCodeI: return Option<char>::construct('i');
                case X11KeyCodeJ: return Option<char>::construct('j');
                case X11KeyCodeK: return Option<char>::construct('k');
                case X11KeyCodeL: return Option<char>::construct('l');
                case X11KeyCodeM: return Option<char>::construct('m');
                case X11KeyCodeN: return Option<char>::construct('n');
                case X11KeyCodeO: return Option<char>::construct('o');
                case X11KeyCodeP: return Option<char>::construct('p');
                case X11KeyCodeQ: return Option<char>::construct('q');
                case X11KeyCodeR: return Option<char>::construct('r');
                case X11KeyCodeS: return Option<char>::construct('s');
                case X11KeyCodeT: return Option<char>::construct('t');
                case X11KeyCodeU: return Option<char>::construct('u');
                case X11KeyCodeV: return Option<char>::construct('v');
                case X11KeyCodeW: return Option<char>::construct('w');
                case X11KeyCodeX: return Option<char>::construct('x');
                case X11KeyCodeY: return Option<char>::construct('y');
                case X11KeyCodeZ: return Option<char>::construct('z');
                case X11KeyCodeSpace: return Option<char>::construct(' ');
                case X11KeyCodeBacktick: return Option<char>::construct('`');
                case X11KeyCode1: return Option<char>::construct('1');
                case X11KeyCode2: return Option<char>::construct('2');
                case X11KeyCode3: return Option<char>::construct('3');
                case X11KeyCode4: return Option<char>::construct('4');
                case X11KeyCode5: return Option<char>::construct('5');
                case X11KeyCode6: return Option<char>::construct('6');
                case X11KeyCode7: return Option<char>::construct('7');
                case X11KeyCode8: return Option<char>::construct('8');
                case X11KeyCode9: return Option<char>::construct('9');
                case X11KeyCode0: return Option<char>::construct('0');
                case X11KeyCodeDash: return Option<char>::construct('-');
                case X11KeyCodeEquals: return Option<char>::construct('=');
                case X11KeyCodeOpenBracket: return Option<char>::construct('(');
                case X11KeyCodeCloseBracket: return Option<char>::construct(')');
                case X11KeyCodeSemicolon: return Option<char>::construct(';');
                case X11KeyCodeSingleQuote: return Option<char>::construct('\'');
                case X11KeyCodeComma: return Option<char>::construct(',');
                case X11KeyCodeDot: return Option<char>::construct('.');
                case X11KeyCodeSlash: return Option<char>::construct('/');
                case X11KeyCodeBackslash: return Option<char>::construct('\\');
                default: return Option<char>::empty();
            }
        }
        else if (state & X11ModifierKeyShift)
        {
            switch (key_code)
            {
                case X11KeyCodeA: return Option<char>::construct('A');
                case X11KeyCodeB: return Option<char>::construct('B');
                case X11KeyCodeC: return Option<char>::construct('C');
                case X11KeyCodeD: return Option<char>::construct('D');
                case X11KeyCodeE: return Option<char>::construct('E');
                case X11KeyCodeF: return Option<char>::construct('F');
                case X11KeyCodeG: return Option<char>::construct('G');
                case X11KeyCodeH: return Option<char>::construct('H');
                case X11KeyCodeI: return Option<char>::construct('I');
                case X11KeyCodeJ: return Option<char>::construct('J');
                case X11KeyCodeK: return Option<char>::construct('K');
                case X11KeyCodeL: return Option<char>::construct('L');
                case X11KeyCodeM: return Option<char>::construct('M');
                case X11KeyCodeN: return Option<char>::construct('N');
                case X11KeyCodeO: return Option<char>::construct('O');
                case X11KeyCodeP: return Option<char>::construct('P');
                case X11KeyCodeQ: return Option<char>::construct('Q');
                case X11KeyCodeR: return Option<char>::construct('R');
                case X11KeyCodeS: return Option<char>::construct('S');
                case X11KeyCodeT: return Option<char>::construct('T');
                case X11KeyCodeU: return Option<char>::construct('U');
                case X11KeyCodeV: return Option<char>::construct('V');
                case X11KeyCodeW: return Option<char>::construct('W');
                case X11KeyCodeX: return Option<char>::construct('X');
                case X11KeyCodeY: return Option<char>::construct('Y');
                case X11KeyCodeZ: return Option<char>::construct('Z');
                case X11KeyCode1: return Option<char>::construct('!');
                case X11KeyCode2: return Option<char>::construct('@');
                case X11KeyCode3: return Option<char>::construct('#');
                case X11KeyCode4: return Option<char>::construct('$');
                case X11KeyCode5: return Option<char>::construct('%');
                case X11KeyCode6: return Option<char>::construct('^');
                case X11KeyCode7: return Option<char>::construct('&');
                case X11KeyCode8: return Option<char>::construct('*');
                case X11KeyCode9: return Option<char>::construct('(');
                case X11KeyCode0: return Option<char>::construct(')');
                case X11KeyCodeDash: return Option<char>::construct('_');
                case X11KeyCodeEquals: return Option<char>::construct('+');
                case X11KeyCodeSemicolon: return Option<char>::construct(':');
                case X11KeyCodeSingleQuote: return Option<char>::construct('"');
                case X11KeyCodeComma: return Option<char>::construct('<');
                case X11KeyCodeDot: return Option<char>::construct('>');
                case X11KeyCodeSlash: return Option<char>::construct('?');
                case X11KeyCodeBackslash: return Option<char>::construct('|');
                default: return Option<char>::empty();
            }
        }
        return Option<char>::empty();
    }
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
