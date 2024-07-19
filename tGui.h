#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include <commctrl.h>
#if defined(__MINGW32__) // Code::Blocks
#include <gdiplus/gdiplus.h>
#else // msvc++ 20xx?? (2019 for sure...) Not sure what msvc++'s pi constant is.....
#define M_PI 3.1415926535897932384
#include <gdiplus.h>
#endif
#include <uxtheme.h>
#include <GL/gl.h>
#include <mmsystem.h>
#include <thread>
#include <atomic>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <chrono>
#include "resource.h"

/*=======================================================================================================================
* Welcome to Talon's Quick n Dirty wrapper to win32 api in terms of game development!
  Everything's hacked together snowballed into wrapper classes and somehow all works!
=======================================================================================================================
* Useful functions and enums that just don't fit inside any wrapper...
=======================================================================================================================*/
std::wstring toWideString(const std::string& in) { return std::wstring(in.begin(), in.end()); }

std::string ExePath() {
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
}

std::string ReadIni(const std::string& File, const std::string& Section, const std::string& Item) {
    char buffer[MAX_PATH];
    GetPrivateProfileStringA(Section.c_str(), Item.c_str(), "", buffer, MAX_PATH, File.c_str());
    return std::string(buffer);
}
bool WriteIni(LPCTSTR File, LPCTSTR Section, LPCTSTR Item, LPCTSTR Value) { return WritePrivateProfileStringA(Section, Item, Value, File); }
bool RemIni(LPCTSTR File, LPCTSTR Section, LPCTSTR Item) { return WritePrivateProfileStringA(Section, Item, 0, File); }

enum Key
{
    NONE,
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    K0, K1, K2, K3, K4, K5, K6, K7, K8, K9,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    UP, DOWN, LEFT, RIGHT,
    SPACE, TAB, SHIFT, CTRL, INS, DEL, HOME, END, PGUP, PGDN,
    BACK, ESCAPE, RETURN, ENTER, PAUSE, SCROLL,
    NP0, NP1, NP2, NP3, NP4, NP5, NP6, NP7, NP8, NP9,
    NP_MUL, NP_DIV, NP_ADD, NP_SUB, NP_DECIMAL, PERIOD,
    EQUALS, COMMA, MINUS,
    OEM_1, OEM_2, OEM_3, OEM_4, OEM_5, OEM_6, OEM_7, OEM_8,
    CAPS_LOCK, ENUM_END
};
std::map<Key, std::string> KeyName = {
    {NONE, ""},
    {A, "A"}, {B, "B"}, {C, "C"}, {D, "D"}, {E, "E"}, {F,"F"}, {G,"G"}, {H,"H"}, {I,"I"}, {J,"J"}, {K,"K"}, {L,"L"}, {M,"M"}, {N,"N"}, {O,"O"}, {P,"P"}, {Q,"Q"}, {R,"R"}, {S,"S"}, {T,"T"}, {U,"U"}, {V,"V"}, {W,"W"}, {X,"X"}, {Y,"Y"}, {Z,"Z"},
    {K0,"0"}, {K1,"1"}, {K2,"2"}, {K3,"3"}, {K4,"4"}, {K5,"5"}, {K6,"6"}, {K7,"7"}, {K8,"8"}, {K9,"9"},
    {F1,"F1"}, {F2,"F2"}, {F3,"F3"}, {F4,"F4"}, {F5,"F5"}, {F6,"F6"}, {F7,"F7"}, {F8,"F8"}, {F9,"F9"}, {F10,"F10"}, {F11,"F11"}, {F12,"F12"},
    {UP,"Up"}, {DOWN,"Down"}, {LEFT,"Left"}, {RIGHT,"Right"},
    {SPACE,"Space"}, {TAB,"Tab"}, {SHIFT,"Shift"}, {CTRL,"Ctrl"}, {INS,"Insert"}, {DEL,"Delete"}, {HOME,"Home"}, {END,"End"}, {PGUP,"PgUp"}, {PGDN,"PgDn"},
    {BACK,"Back"}, {ESCAPE,"Escape"}, {RETURN,"Return"}, {ENTER,"Enter"}, {PAUSE,"Pause"}, {SCROLL,"Scroll"},
    {NP0,"Num0"}, {NP1,"Num1"}, {NP2,"Num2"}, {NP3,"Num3"}, {NP4,"Num4"}, {NP5,"Num5"}, {NP6,"Num6"}, {NP7,"Num7"}, {NP8,"Num8"}, {NP9,"Num9"},
    {NP_MUL,"Num*"}, {NP_DIV,"Num/"}, {NP_ADD,"Num+"}, {NP_SUB,"Num-"}, {NP_DECIMAL,"Num."},
    {PERIOD,"."}, {EQUALS,"="}, {COMMA,","}, {MINUS,"-"},
    {OEM_1,"OEM1"}, {OEM_2,"OEM2"}, {OEM_3,"OEM3"}, {OEM_4,"OEM4"}, {OEM_5,"OEM5"}, {OEM_6,"OEM6"}, {OEM_7,"OEM7"}, {OEM_8,"OEM8"},
    {CAPS_LOCK,"CapsLock"},
    {ENUM_END,"End"}
};

/*=======================================================================================================================
* CHEAP AND DIRTY Audio Mixer Wrapper class............
=======================================================================================================================*/

struct LazyAudioSound {
    std::string Alias;
    std::string Location;
};
class LazyAudioMixer {
protected:
    static int LoadedSounds;
private:
    std::vector<LazyAudioSound> Sounds;
public:
    LazyAudioMixer() { }
    ~LazyAudioMixer() { }
    void Add(std::string Alias, std::string Location, bool local = true) {
        LazyAudioSound tmp;
        tmp.Alias = Alias;
        tmp.Location = Location;
        std::string music_cmd = "open " + ExePath() + (local ? "\\" + Location : "") + " alias " + Alias;
        mciSendString(music_cmd.c_str(), 0, 0, 0);
        Sounds.push_back(tmp);
    }
    void AddLoop(std::string Alias, std::string Location, bool local = true) {
        LazyAudioSound tmp;
        tmp.Alias = Alias;
        tmp.Location = Location;
        std::string music_cmd = "open " + ExePath() + (local ? "\\" + Location : "") + " alias " + Alias;
        mciSendString(music_cmd.c_str(), 0, 0, 0);
        music_cmd = "play " + Alias + " repeat";
        mciSendString(music_cmd.c_str(), 0, 0, 0);
        Sounds.push_back(tmp);
    }
    void Play(std::string Alias) {
        for (auto& tmp : Sounds) {
            if (tmp.Alias == Alias) {
                std::string music_cmd = "play " + Alias + " from 0";
                mciSendString(music_cmd.c_str(), 0, 0, 0);
            }
        }
    }
    void Delete(std::string Alias) {
        for (auto& tmp : Sounds) {
            if (tmp.Alias == Alias) {
                //PlayedSound++;
                std::string music_cmd = "close " + Alias;
                mciSendString(music_cmd.c_str(), 0, 0, 0);
            }
        }
    }
};
int LazyAudioMixer::LoadedSounds = 0;

/*=======================================================================================================================
* Base tMainWindow Class (Basically everything inherits from this window wise, even dialogs... controls dont tho....)
=======================================================================================================================*/

typedef struct {
    int x, y, dx, dy, key;
}tMouseStruct;

struct tTimerStruct {
    UINT Index;
    std::string Alias;
};

class tMainWindow {
protected:
    static bool HasRegisteredWindowClass;
private:
    NOTIFYICONDATA NotifyIconData;                //Our Tray Icon if we're asked to produce one
    HICON AppIcon = nullptr;                      //Our Program Icon
    HCURSOR AppCursor = nullptr;                  //Our Mouse Cursor
    HINSTANCE hInstC = nullptr;                   //hInstance assigned to whatever we made with CreateWindow(EX) as our main app
    HWND hwndC = nullptr;                         //window itself handle
    HWND hwndMDI = nullptr;                       //window itself handle
    HWND hStatusC = nullptr;                      //our status bar handle
    LPCSTR szClassName = "tMainWindow"; //The "class" windows assigns our "object" with createwindow(Ex)
    LPCSTR szChildClassName = "tMainSubWindow"; //The "class" windows assigns our "object" with createwindow(Ex)
    HWND ModalParentHWND = nullptr;               //Placeholder to the window we belong to that should be "locked" while we're modal.
    int TimerIDCount = 1;               //Rolling counter of child created timers.
    int ChildIDCount = 1;               //Rolling counter of child created "windows" (buttons/lists/editboxes/etc..)
    std::map<size_t, uint8_t> mapKeys;  //Custom defined keyref map to our enum of keys.
    bool KeyboardState[256] = { 0 };    //Array of keys as booleans to flag when pressed/released (keylog)

    bool cursorvisible = true;          //internal state switch for when we set the cursor to nullptr (none)
    bool initialized = false;           //Did we trigger our virtual to derived clasess an "OnInit()" yet?

    bool IsInternalSized = false;             //Should we end the entire app if we're destroyed.
    bool IsMainApplication = false;             //Should we end the entire app if we're destroyed.
    bool IsMDIApplication = false;              //Should we make a mdi client area?
    bool IsModal = false;                       //Are we a modal dialog to some other hwnd? (window)
    bool IsThreaded = false;                    //Are we using an std::thread alongside our message loop?
    bool IsGraphicalGDI = false;                //Are we planning on just drawing stuff via GDIPlus?

    int wX = 0, wY = 0, wiX = 0, wiY = 0, Width = 0, Height = 0, iWidth = 0, iHeight = 0;  //Our window overall position/size, and the inner (content surface) position/size.
    tMouseStruct Mouse;                 //Mouse tracking data.
    std::vector<tTimerStruct> Timers;   //Vector of created timers.

    //Timers time tracking... ETC!
    UINT rTimerID = 0;                  //Windows multimedia timer callback, emits WM_TIMER with it's ID.
    std::atomic<bool> bAtomActive;      //The boolean we monitor to die gracefully between message loop and thread.
    std::chrono::system_clock::time_point fStart, fCheck, lStart, lCheck; //thread execution time, and multimedia timer execution time trackers
    float fElapsedTime, lElapsedTime;   //Results of both

    //GDI Plus startup and pointer to the "graphics" object (AKA what we're drawing to: the Device Context)
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::Graphics* gf;
    ULONG_PTR gdiplusToken;

    //Backbuffering DC, compatible bitmap, and 1 dimensional pointer to each pixel in said bitmap.
    HDC backbuffDC = nullptr;
    HBITMAP backbuffer = nullptr;
    BITMAP buffSz;
    unsigned char* buff1d;

    //Internal Private Methods
    static LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM); //What we report to windows when we "register" our "class", we'll have to cast to the actual object here....
    static LRESULT CALLBACK MDIWindowProc(HWND, UINT, WPARAM, LPARAM); //What we report to windows when we "register" our "class", we'll have to cast to the actual object here....
    void EngineThread() { //callback for our std::thread
        while (bAtomActive) {
            fCheck = std::chrono::system_clock::now();
            std::chrono::duration<float> elapsedTime = fCheck - fStart;
            fStart = fCheck;
            fElapsedTime = elapsedTime.count();
            OnUpdate(fElapsedTime);
        }
    }
    void MapKeyboardKeys() { // Create Keyboard Mapping
        mapKeys[0x00] = Key::NONE;
        mapKeys[0x41] = Key::A; mapKeys[0x42] = Key::B; mapKeys[0x43] = Key::C; mapKeys[0x44] = Key::D; mapKeys[0x45] = Key::E;
        mapKeys[0x46] = Key::F; mapKeys[0x47] = Key::G; mapKeys[0x48] = Key::H; mapKeys[0x49] = Key::I; mapKeys[0x4A] = Key::J;
        mapKeys[0x4B] = Key::K; mapKeys[0x4C] = Key::L; mapKeys[0x4D] = Key::M; mapKeys[0x4E] = Key::N; mapKeys[0x4F] = Key::O;
        mapKeys[0x50] = Key::P; mapKeys[0x51] = Key::Q; mapKeys[0x52] = Key::R; mapKeys[0x53] = Key::S; mapKeys[0x54] = Key::T;
        mapKeys[0x55] = Key::U; mapKeys[0x56] = Key::V; mapKeys[0x57] = Key::W; mapKeys[0x58] = Key::X; mapKeys[0x59] = Key::Y;
        mapKeys[0x5A] = Key::Z;

        mapKeys[VK_F1] = Key::F1; mapKeys[VK_F2] = Key::F2; mapKeys[VK_F3] = Key::F3; mapKeys[VK_F4] = Key::F4;
        mapKeys[VK_F5] = Key::F5; mapKeys[VK_F6] = Key::F6; mapKeys[VK_F7] = Key::F7; mapKeys[VK_F8] = Key::F8;
        mapKeys[VK_F9] = Key::F9; mapKeys[VK_F10] = Key::F10; mapKeys[VK_F11] = Key::F11; mapKeys[VK_F12] = Key::F12;

        mapKeys[VK_DOWN] = Key::DOWN; mapKeys[VK_LEFT] = Key::LEFT; mapKeys[VK_RIGHT] = Key::RIGHT; mapKeys[VK_UP] = Key::UP;
        //mapKeys[VK_RETURN] = Key::ENTER;// mapKeys[VK_RETURN] = Key::RETURN;

        mapKeys[VK_BACK] = Key::BACK; mapKeys[VK_ESCAPE] = Key::ESCAPE; mapKeys[VK_RETURN] = Key::ENTER; mapKeys[VK_PAUSE] = Key::PAUSE;
        mapKeys[VK_SCROLL] = Key::SCROLL; mapKeys[VK_TAB] = Key::TAB; mapKeys[VK_DELETE] = Key::DEL; mapKeys[VK_HOME] = Key::HOME;
        mapKeys[VK_END] = Key::END; mapKeys[VK_PRIOR] = Key::PGUP; mapKeys[VK_NEXT] = Key::PGDN; mapKeys[VK_INSERT] = Key::INS;
        mapKeys[VK_SHIFT] = Key::SHIFT; mapKeys[VK_CONTROL] = Key::CTRL;
        mapKeys[VK_SPACE] = Key::SPACE;

        mapKeys[0x30] = Key::K0; mapKeys[0x31] = Key::K1; mapKeys[0x32] = Key::K2; mapKeys[0x33] = Key::K3; mapKeys[0x34] = Key::K4;
        mapKeys[0x35] = Key::K5; mapKeys[0x36] = Key::K6; mapKeys[0x37] = Key::K7; mapKeys[0x38] = Key::K8; mapKeys[0x39] = Key::K9;

        mapKeys[VK_NUMPAD0] = Key::NP0; mapKeys[VK_NUMPAD1] = Key::NP1; mapKeys[VK_NUMPAD2] = Key::NP2; mapKeys[VK_NUMPAD3] = Key::NP3; mapKeys[VK_NUMPAD4] = Key::NP4;
        mapKeys[VK_NUMPAD5] = Key::NP5; mapKeys[VK_NUMPAD6] = Key::NP6; mapKeys[VK_NUMPAD7] = Key::NP7; mapKeys[VK_NUMPAD8] = Key::NP8; mapKeys[VK_NUMPAD9] = Key::NP9;
        mapKeys[VK_MULTIPLY] = Key::NP_MUL; mapKeys[VK_ADD] = Key::NP_ADD; mapKeys[VK_DIVIDE] = Key::NP_DIV; mapKeys[VK_SUBTRACT] = Key::NP_SUB; mapKeys[VK_DECIMAL] = Key::NP_DECIMAL;

        mapKeys[VK_OEM_1] = Key::OEM_1;			// On US and UK keyboards this is the ';:' key
        mapKeys[VK_OEM_2] = Key::OEM_2;			// On US and UK keyboards this is the '/?' key
        mapKeys[VK_OEM_3] = Key::OEM_3;			// On US keyboard this is the '~' key
        mapKeys[VK_OEM_4] = Key::OEM_4;			// On US and UK keyboards this is the '[{' key
        mapKeys[VK_OEM_5] = Key::OEM_5;			// On US keyboard this is '\|' key.
        mapKeys[VK_OEM_6] = Key::OEM_6;			// On US and UK keyboards this is the ']}' key
        mapKeys[VK_OEM_7] = Key::OEM_7;			// On US keyboard this is the single/double quote key. On UK, this is the single quote/@ symbol key
        mapKeys[VK_OEM_8] = Key::OEM_8;			// miscellaneous characters. Varies by keyboard
        mapKeys[VK_OEM_PLUS] = Key::EQUALS;		// the '+' key on any keyboard
        mapKeys[VK_OEM_COMMA] = Key::COMMA;		// the comma key on any keyboard
        mapKeys[VK_OEM_MINUS] = Key::MINUS;		// the minus key on any keyboard
        mapKeys[VK_OEM_PERIOD] = Key::PERIOD;	// the period key on any keyboard
        mapKeys[VK_CAPITAL] = Key::CAPS_LOCK;        //center window
    }
    void InitStatusBar(bool visible = false) { // Create Status bar. TODO: allow modifiers to add/remove/insert and adjust panels, for now revert to just one.
        hStatusC = CreateWindowEx(
            0, // no extended styles
            STATUSCLASSNAME, // name of status bar class
            (PCTSTR)NULL, // no text when first created
            WS_CHILD | SBARS_SIZEGRIP | (visible ? WS_VISIBLE : 0), // styles, TODO: should look at owner window styles to decide on size grip or not..
            0, 0, 0, 0, // ignores size and position
            hwndC, // handle to parent window
            (HMENU)0, // child window identifier
            hInstC, // handle to application instance
            NULL);
    }
    void InitTrayIcon(const std::string& TrayCaption) { // Initialize and Show the notification icon
        ZeroMemory(&NotifyIconData, sizeof(NotifyIconData));
        NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
        NotifyIconData.hWnd = hwndC;
        NotifyIconData.uID = 0;
        NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

        NotifyIconData.uCallbackMessage = WM_USER;

        //Subject to change....
        NotifyIconData.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));

        lstrcpy(NotifyIconData.szTip, TrayCaption.c_str());
        Shell_NotifyIcon(NIM_ADD, &NotifyIconData);
    }
    void CenterDlgToParent(HWND hwndParent) { // Useful function to quickly center a given hwnd onto another.
        RECT rectParent;
        GetWindowRect(hwndParent, &rectParent);
        // Calculate the width and height of the parent
        int nParentWidth = rectParent.right - rectParent.left;
        int nParentHeight = rectParent.bottom - rectParent.top;
        // Get window rect of the dialog
        RECT rectDlg;
        GetWindowRect(hwndC, &rectDlg);
        // Calculate the width and height of the dialog
        int nDlgWidth = rectDlg.right - rectDlg.left;
        int nDlgHeight = rectDlg.bottom - rectDlg.top;
        // Calculate the center of the dialog
        // Note: Subtracting the dialogs calculated center from the its parent
        // left and top makes sure that the dialog box is centered even when
        // the parent is smaller in dimension as compared to the dialog.
        rectDlg.left = rectParent.left - ((nDlgWidth / 2) - (nParentWidth / 2));
        rectDlg.top = rectParent.top - ((nDlgHeight / 2) - (nParentHeight / 2));
        // Get the width and height of the screen
        int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
        int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
        // Calculate dialog's left and top
        int nDlgLeft = rectDlg.left < 0 ? 0 : rectDlg.left;
        int nDlgTop = rectDlg.top < 0 ? 0 : rectDlg.top;
        // If dialog's right goes beyond the right boundary of the screen,
        // subtract the amount by which it goes beyond from the dialog's left.
        int nDlgRight = rectDlg.left + nDlgWidth;
        if (nDlgRight > nScreenWidth) nDlgLeft -= (nDlgRight - nScreenWidth);
        // If dialog's bottom goes beyond the bottom boundary of the screen,
        // subtract the amount by which it goes beyond from the dialog's right.
        int nDlgBottom = rectDlg.top + nDlgHeight;
        if (nDlgBottom > nScreenHeight) nDlgTop -= (nDlgBottom - nScreenHeight);
        // Move window to center
        MoveWindow(hwndC, nDlgLeft, nDlgTop, nDlgWidth, nDlgHeight, TRUE);
    }
    HWND CreateNewMDIChild(HWND hMDIClient) { //Generic MDI window generation if we're an MDI application
        MDICREATESTRUCT mcs;
        HWND hChild;

        mcs.szTitle = "New Window";
        mcs.szClass = szChildClassName;
        mcs.hOwner = GetModuleHandle(NULL);
        mcs.x = mcs.cx = CW_USEDEFAULT;
        mcs.y = mcs.cy = CW_USEDEFAULT;
        mcs.style = MDIS_ALLCHILDSTYLES;

        BOOL bMaximized = FALSE;
        HWND hWndOld = (HWND)SendMessage(hMDIClient, WM_MDIGETACTIVE, 0, (LPARAM)&bMaximized);

        if (bMaximized == TRUE) { SendMessage(hMDIClient, WM_SETREDRAW, FALSE, 0); }
        hChild = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LONG)&mcs);

        if (!hChild) {
            MessageBox(hMDIClient, "MDI Child creation failed.", "Oh Oh...", MB_ICONEXCLAMATION | MB_OK);
        }
        if (bMaximized == TRUE) {
            ShowWindow(hChild, SW_SHOWMAXIMIZED);
            SendMessage(hMDIClient, WM_SETREDRAW, TRUE, 0);
            RedrawWindow(hMDIClient, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
        }
        //TODO: FIX the damn fallback for Dwm not working in MDI windows (Falls back to Vista aero.msstyles)
        //long styles = GetWindowLong(hChild, GWL_EXSTYLE);
        //styles |= WS_EX_TOOLWINDOW;
        //SetWindowLong(hChild, GWL_EXSTYLE, styles);
        return hChild;
    }

public:
    tMainWindow(
        const int& rx,                                              //Requested screen location X
        const int& ry,                                              //Requested screen location Y
        const int& rw,                                              //Requested width
        const int& rh,                                              //Requested height
        const std::string& Icon,                                         //Window Icon
        const std::string& Cursor,                                       //Window Mouse Cursor
        const std::string& Caption,                                      //Window Title
        const long stylesEx = WS_EX_CONTROLPARENT,                 //Default extended styles
        const long styles = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, //Default styles
        const bool isInternalSized = false,       //Upon creation, is the outer final dimensions (boder window title n all) or the inner dimensions? (window surface)
        const bool isMainApplication = true,      //Should we end the entire app if we're destroyed.
        const bool isMDIApplication = false,       //Should we make a mdi child client area?
        const bool isModal = false,               //Are we a modal dialog to some other hwnd? (window)
        const bool isThreaded = false,            //Are we using an std::thread alongside our message loop?
        const bool isGraphicalGDI = false        //Are we planning on just drawing stuff via GDIPlus?
    ) {

        //Assign these values on init.
        IsInternalSized = isInternalSized;
        IsMainApplication = isMainApplication;
        IsMDIApplication = isMDIApplication;
        IsModal = isModal;
        IsThreaded = isThreaded;
        IsGraphicalGDI = isGraphicalGDI;

        //Initialize keyboard mappings
        MapKeyboardKeys();

        //For now use the resource icon instead of the one passed... subject to change...
        AppIcon = (HICON)LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ICON1));
        //AppCursor = LoadCursor(nullptr, IDC_ARROW);

        //Register our "class" with windows
        if (!HasRegisteredWindowClass) {
            if (IsGraphicalGDI) { Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr); }

            // Commented out testing for bugs, was unrelated to the bug but seems to work without ever using this so left out for now...
            //INITCOMMONCONTROLSEX icc;
            //icc.dwSize = sizeof(icc);
            //icc.dwICC = ICC_WIN95_CLASSES;
            //InitCommonControlsEx(&icc);
            WNDCLASS wc;
            //wc.cbSize = sizeof(WNDCLASSEX);
            wc.style = CS_HREDRAW | CS_VREDRAW;
            wc.lpfnWndProc = WindowProc;
            wc.cbClsExtra = 0;
            wc.cbWndExtra = DLGWINDOWEXTRA;
            wc.hInstance = hInstC;
            wc.hIcon = AppIcon;
            wc.hCursor = LoadCursor(NULL, IDC_ARROW);
            wc.hbrBackground = (HBRUSH)COLOR_BTNFACE; //COLOR_BTNFACE+1 == old-school XP dialog, COLOR_BACKGROUND retro 98 and below (or mdi)
            wc.lpszMenuName = NULL;
            wc.lpszClassName = szClassName;
            HasRegisteredWindowClass = RegisterClass(&wc);
            if (!HasRegisteredWindowClass) { MessageBox(hwndC, "Registration failed!", "Oh Oh...", MB_ICONEXCLAMATION | MB_OK); }

            if (IsMDIApplication) {
                WNDCLASS wcMDI;
                //wcMDI.cbSize = sizeof(WNDCLASSEX);
                wcMDI.style = CS_HREDRAW | CS_VREDRAW;
                wcMDI.lpfnWndProc = MDIWindowProc;
                wcMDI.cbClsExtra = 0;
                wcMDI.cbWndExtra = DLGWINDOWEXTRA;
                wcMDI.hInstance = hInstC;
                wcMDI.hIcon = AppIcon;
                wcMDI.hCursor = LoadCursor(NULL, IDC_ARROW);
                wcMDI.hbrBackground = (HBRUSH)(COLOR_WINDOW);
                wcMDI.lpszMenuName = NULL;
                wcMDI.lpszClassName = szChildClassName;
                //wcMDI.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

                if (!RegisterClass(&wcMDI)) { MessageBox(0, "Could Not Register Child Window", "Oh Oh...", MB_ICONEXCLAMATION | MB_OK); }
            }
        }

        //If we want our specified internal width to be a fixed dimension we must compute this...
        RECT wr = { 0, 0, rw, rh }; // set the size, but not the position
        if (isInternalSized) { AdjustWindowRect(&wr, styles, false); }// adjust (grow) the size based on the styles (window border, frame size, etc..)
        Width = wr.right - wr.left;
        Height = wr.bottom - wr.top;
        iWidth = Width - rw;
        iHeight = Height = rh;

        //Special case, -1 for center me, CW_USEDEFAULT for windows assign, >= 0 for actual positioning.
        int xp = (rx == -1 ? (GetSystemMetrics(SM_CXSCREEN) - Width) / 2 : rx);
        int yp = (ry == -1 ? (GetSystemMetrics(SM_CYSCREEN) - Height) / 2 : ry);

        //Needed for the extended stuff, toolbars, statusbars, etc... must be called before making your window
        InitCommonControls();

        //Create the bloody thing via win32 api calls... one of which sets our hinstance var, the other gives windows the pointer to our class/
        hwndC = CreateWindowEx(stylesEx, szClassName, Caption.c_str(), styles, xp, yp, Width, Height, HWND_DESKTOP, NULL, hInstC, (LPVOID)this);
        //SetWindowTheme(hwndC, L"", L"");

        //assign a statusbar even if user doesn't want one, it'll just be hidden instead of visible
        InitStatusBar();

        //Make the MDI child "container" for the child windows to reside within
        if (IsMDIApplication) {
            CLIENTCREATESTRUCT ccs;
            hwndMDI = CreateWindowEx(0, "mdiclient", NULL, WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndC, NULL, GetModuleHandle(NULL), (LPVOID)&ccs);
        }

        //Initialize mouse values. TODO: change to struct instead of typedef struct so i can initialize these already...
        Mouse.x = 0;
        Mouse.y = 0;
        Mouse.dx = 0;
        Mouse.dy = 0;
        Mouse.key = 0;

        //Initialize Chrono for elapsed time tracking even if we never need them....
        fStart = std::chrono::system_clock::now();
        fCheck = std::chrono::system_clock::now();
        fElapsedTime = 0;
        lStart = std::chrono::system_clock::now();
        lCheck = std::chrono::system_clock::now();
        lElapsedTime = 0;

        //in case we didn't adjust our size needing a fixed internal dimension we need to now figure out what this dimension is.
        if (!IsInternalSized) {
            RECT wr;
            GetClientRect(hwnd(), &wr);
            iWidth = wr.right - wr.left;
            iHeight = wr.bottom - wr.top;
        }

        //Initialize GDI stuff if we're graphical based
        if (IsGraphicalGDI) {
            LoadNewGDIBackBuffer();
            //start the windows multimedia timer (used to request a render)
            //Request 15ms 1k/15 ~= 66 FPS ... Could go lower but seems windows
            //either can't or won't honor below 5ms. and Achieving anything better
            //than 15ms reliably is a gamble anyways even doing nothing....
            SetTimer(hwnd(), rTimerID, 15, (TIMERPROC)NULL);
        }

        //Expiramental test to be removed....
        if (IsMDIApplication) {
            HWND child = CreateNewMDIChild(hwndMDI);
            //SetWindowTheme(child, L"Window", L"DwmWindow");
            //SendMessage(child, WM_NCPAINT, NULL, NULL);
            //SetWindowLong(child, GWL_EXSTYLE, WS_EX_MDICHILD);
        }
    }
    virtual ~tMainWindow() {
        Exit(); //We're dying... Inform the derived about it so hopefully they can cleanup beforehand.
        if (gf) { DeleteObject(&gf); } //Cleanup Graphics draw object
        if (backbuffer) { DeleteObject(&backbuffer); } //Cleanup backbuffer
        if (backbuffDC) { DeleteDC(backbuffDC); }     //release the DC to the backbuffer
        if (IsGraphicalGDI) { Gdiplus::GdiplusShutdown(gdiplusToken); } //Gracefully end GDIPlus
        PostQuitMessage(0); //If the windows still alive, send it a kill
    }

    //The magic handler! The callback for each instance of a tMainWindow
    virtual LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    //inside windows callbacks events spawned by children are relayed in form of an ID. As far as I can tell
    //this is just some unsigned int. This is just a rolling count every time something gets added to the main hwnd
    //for tracking purposes to be able to get back to the proper hwnd that caused the event.
    int GetNexTAvaialiableChildID() { return ++ChildIDCount; }
    int GetNexTAvaialiableTimerID() { return ++TimerIDCount; }
    WPARAM Run() { //Make me go!
        MSG msgs;
        ShowWindow(hwndC, SW_SHOW);
        if (!initialized) { OnInit(); }
        if (IsThreaded) {
            bAtomActive = true;
            std::thread t = std::thread(&tMainWindow::EngineThread, this);
            while (GetMessage(&msgs, NULL, 0, 0)) {
                if (!IsMDIApplication || !TranslateMDISysAccel(hwndMDI, &msgs)) {
                    if ((IsGraphicalGDI && msgs.hwnd == hwndC) || !IsDialogMessage(GetActiveWindow(), &msgs)) {
                        TranslateMessage(&msgs);
                        DispatchMessage(&msgs);
                    }
                }
            }
            bAtomActive = false;
            t.join();
            return msgs.wParam;
        }
        else {
            while (GetMessage(&msgs, NULL, 0, 0)) {
                if (!IsMDIApplication || !TranslateMDISysAccel(hwndMDI, &msgs)) {
                    if ((IsGraphicalGDI && msgs.hwnd == hwndC) || !IsDialogMessage(GetActiveWindow(), &msgs)) {
                        TranslateMessage(&msgs);
                        DispatchMessage(&msgs);
                    }
                }
            }
            return msgs.wParam;
        }
    }
    void Exit() {
        bAtomActive = false;    //Stop thread if it exists...
        OnExit();               //Emit our exit event.
        PostQuitMessage(0);     //tell windows to close our message loop.
    }
    void Show(const bool visible = true) {
        HWND Parent = (HWND)GetActiveWindow(); //Find active window
        if (Parent != hwnd()) { //We are not the current active window..
            if (IsWindowVisible(hwnd())) { //We're already visible
                if (visible) { //We're requested visible
                    SetFocus(hwnd());
                }
                else { //we've requested hide }
                    if (IsModal) { EnableWindow(ModalParentHWND, true); ShowWindow(hwnd(), SW_HIDE); }
                    else { ShowWindow(hwnd(), SW_HIDE); }
                }
            }
            else { //We're not currently visible
                if (visible) { //We're requested visible
                    CenterDlgToParent(Parent);    //Center me to active window
                    ModalParentHWND = Parent;     //Log who i centered to just incase I am Modal so we can lock it.
                    //Order matters if we "locked" a window... reason for these..
                    if (IsModal) { ShowWindow(hwnd(), SW_SHOW); EnableWindow(ModalParentHWND, false); }
                    else { ShowWindow(hwnd(), SW_SHOW); }
                }
                else { //we've requested hide }
                    if (IsModal) { EnableWindow(ModalParentHWND, true); ShowWindow(hwnd(), SW_HIDE); }
                    else { ShowWindow(hwnd(), SW_HIDE); }
                }
            }
        }
        //Maybe we were asked to hide and we're not the parent visible window?
        else if (!visible) { //we've requested hide }
            if (IsModal) { EnableWindow(ModalParentHWND, true); ShowWindow(hwnd(), SW_HIDE); }
            else { ShowWindow(hwnd(), SW_HIDE); }
        }
    }
    void ShowStatus(const bool visible = true) { if (hStatusC) { ShowWindow(hStatusC, (visible ? SW_SHOW : SW_HIDE)); } }
    void ShowTrayIcon(std::string Caption) { InitTrayIcon(Caption); }
    //void SetWindowIcon(std::string Title) { }
    void SetWindowTitle(const std::string Title = "") { SetWindowText(hwndC, Title.c_str()); }
    void SetStatusbarTitle(const std::string Caption = "") { if (hStatusC) { SendMessage(hStatusC, SB_SETTEXT, 0, (LPARAM)Caption.c_str()); } }
    void SetMouseCursorVisible(bool visible) { cursorvisible = visible; SetCursor(visible ? AppCursor : nullptr); }
    //void SetMouseCursor(std::string ???) { SetCursor(); }
    void AddHotkey(const int& ID, const UINT& fsModifiers, const UINT& VirtualKey) { RegisterHotKey(hwndC, ID, fsModifiers, VirtualKey);  }
    void DelHotkey(const int& ID) { UnregisterHotKey(hwndC,ID); }

    HINSTANCE hinst() { return hInstC; }
    HWND hwnd() { return hwndC; }
    HWND hwndmdi() { return hwndMDI; }
    bool GetKey(const Key k) { return KeyboardState[k]; }
    int width() { return iWidth; }
    int height() { return iHeight; }
    int dwidth() { return Width; }
    int dheight() { return Height; }
    int x() { return wX; }
    int y() { return wY; }
    int ix() { return wiX; } //internal relative position of our surface (might be shifted from docks/toolbars/etc..)
    int iy() { return wiY; }
    tMouseStruct mouse() { return Mouse; }
    float ElapsedTime() { return fElapsedTime; }


    //Callbacks for various things, mainly used when we inherit from this...
    virtual void OnInit() { } //We should load things BEFORE we create the window..
    virtual void OnOpen() { } //We can init things here AFTER the window gets created...
    virtual bool OnClose() { //this event lets us return true to prevent the window from exiting, it instead hides.
        return false; //override me in your derived class!!! this is just default....
    }
    virtual void OnExit() { } //Impending DOOM! Die phase... Cleanup what you can!
    virtual void OnSclick(HWND item, const int& ID) { }
    virtual void OnDclick(HWND item, const int& ID) { }
    virtual void OnEdit(HWND item, const int& ID) { }
    //virtual void OnMenu(HWND item, int ID) { } Handled already in sclick due to ID assignment...
    virtual void OnScroll(HWND item, const int& ID) { }
    //Do complex maths about a game?
    virtual void OnUpdate(const float& fElapsedTime) {  }
    virtual void OnKeyDown(const int32_t& key, const UINT& vk) { }
    virtual void OnKeyUp(const int32_t& key, const UINT& vk) { }
    virtual void OntMouseLeave() { }
    virtual void OntMouseMove() { }
    virtual void OntMouseSclick() { }
    virtual void OntMouseDclick() { }
    virtual void OntMouseRclick() { }
    virtual void OntMouseUclick() { }
    virtual void OntLostFocus() { }
    virtual void OntHotkey(const int& ID) { }
    virtual void OntTimer(const int& ID) { }
    //Request we draw something.... GDI+ related?
    virtual void Render(const float fElapsedTime = 0) { }

    //Graphical window related stuffs
    Gdiplus::Graphics* Graphics() { return gf; }
    //Routine for double buffer windows when starting or window resizes
    void LoadNewGDIBackBuffer(const bool invalidate = true) {
        RECT SurfaceRect;
        GetClientRect(hwnd(), &SurfaceRect);
        //Release old data if it existed...
        if (backbuffer || backbuffDC) {
            DeleteObject(&gf);
            DeleteObject(&backbuffer);
            DeleteDC(backbuffDC);
        }

        //Initialize our drawing DC and where it points to, a memory HBITMAP.
        backbuffDC = CreateCompatibleDC(GetWindowDC(hwnd()));
        backbuffer = CreateCompatibleBitmap(GetWindowDC(hwnd()), SurfaceRect.right, SurfaceRect.bottom);

        //Initialize our 1D array to the pixel data (sometimes faster than GDI drawing a pixel)
        GetObject(backbuffer, sizeof(buffSz), &buffSz);
        buff1d = reinterpret_cast<unsigned char*>(buffSz.bmBits);

        SelectObject(backbuffDC, backbuffer);
        //Doesn't seem to matter, tried commenting out just to see if you get any gains never nulling out what should
        //technically start out already zero'd out....
        //Invalidate this surface with null brush
        //if (invalidate) {
        //    HBRUSH hBrush = (HBRUSH)::GetStockObject(NULL_BRUSH); //CreateSolidBrush(RGB(0, 0, 0));
        //    FillRect(backbuffDC, &SurfaceRect, hBrush);
        //    DeleteObject(hBrush);
       // }

        //Create GDI+ Graphics object to backbuffer
        gf = new Gdiplus::Graphics(backbuffDC);

        //Should we have this be per call set or global? static? There's lots of modes but this ones pretty.
        gf->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias); //HighSpeed
    }
    Gdiplus::ARGB aRgb(const uint32_t& argb) {
        uint32_t  a = ((uint8_t*)argb)[0];
        uint32_t  r = ((uint8_t*)argb)[1];
        uint32_t  g = ((uint8_t*)argb)[2];
        uint32_t  b = ((uint8_t*)argb)[3];
        return Gdiplus::Color::MakeARGB(a, r, g, b);
    }
    Gdiplus::ARGB aRgb(const uint8_t& a, const uint8_t& r, const uint8_t& g, const uint8_t& b) { return Gdiplus::Color::MakeARGB(a, r, g, b); }
    Gdiplus::ARGB Rgb(const uint32_t& argb) {
        uint32_t  r = ((uint8_t*)argb)[1];
        uint32_t  g = ((uint8_t*)argb)[2];
        uint32_t  b = ((uint8_t*)argb)[3];
        return Gdiplus::Color::MakeARGB(255, r, g, b);
    }
    Gdiplus::ARGB Rgb(const uint8_t& r, const uint8_t& g, const uint8_t& b) { return Gdiplus::Color::MakeARGB(255, r, g, b); }
    void pDrawDot(const Gdiplus::ARGB& color, const int& size, const int& x, const int& y, const bool fill = true) {
        if (gf) {
            Gdiplus::Color clr = Gdiplus::Color(color);
            if (fill) {
                Gdiplus::SolidBrush brush(clr);
                gf->FillEllipse(&brush, x - size, y - size, size * 2, size * 2);
                DeleteObject(&brush);
            }
            else {
                Gdiplus::Pen pen(clr, 0);       // For lines, rectangles and curves
                gf->DrawEllipse(&pen, x - size, y - size, size * 2, size * 2);
                DeleteObject(&pen);
            }
        }
    }
    void pDrawRadialDot(const Gdiplus::ARGB& centercolor, const Gdiplus::ARGB& outercolor, const int& size, const int& x, const int& y) {
        if (gf) {
            Gdiplus::GraphicsPath gp;
            Gdiplus::Rect rct;
            rct.X = x - size;
            rct.Y = y - size;
            rct.Width = size * 2;
            rct.Height = size * 2;
            gp.AddEllipse(rct);
            Gdiplus::PathGradientBrush pgb(&gp);
            pgb.SetCenterPoint(Gdiplus::PointF(x, y));
            pgb.SetCenterColor(centercolor);
            Gdiplus::Color cols[] = {
                outercolor,
            };
            INT count = 1;
            pgb.SetSurroundColors(cols, &count);
            //pgb.SetFocusScales(.5f, 1.0f);
            gf->FillPath(&pgb, &gp);
            DeleteObject(&pgb);
            DeleteObject(&gp);
        }
    }
    //prettier drawdot at the expense of drawing the filled 1st then the outline.
    //only useful if in fast smoothing mode as a filled only ellipse is damn ugly around the edges
    void pDrawPrettyDot(const Gdiplus::ARGB& color, const int& size, const int& x, const int& y, const bool fill = true) {
        if (gf) {
            Gdiplus::Color clr = Gdiplus::Color(color);
            if (fill) {
                Gdiplus::SolidBrush brush(clr);
                Graphics()->FillEllipse(&brush, x - size, y - size, size * 2, size * 2);
                DeleteObject(&brush);
            }
            Gdiplus::Pen pen(clr, 0);       // For lines, rectangles and curves
            gf->DrawEllipse(&pen, x - size, y - size, size * 2, size * 2);
            DeleteObject(&pen);
        }
    }
    void pDrawLine(const Gdiplus::ARGB& color, const int& size, const int& x1, const int& y1, const int& x2, const int& y2) {
        if (gf) {
            int Sz = size - 1;
            pDrawDot(color, size / 2, x1, y1);
            pDrawDot(color, size / 2, x2, y2);
            Gdiplus::Color clr = Gdiplus::Color(color);
            Gdiplus::Pen pen(clr, Sz);      // For lines, rectangles and curves
            gf->DrawLine(&pen, x1, y1, x2, y2);
            DeleteObject(&pen);
        }
    }
    void pDrawArrow(const Gdiplus::ARGB& color, const int& size, const int& headlen, const int& x1, const int& y1, const int& x2, const int& y2) {
        if (gf) {
            pDrawLine(color, size, x1, y1, x2, y2);
            float angle = atan2f(y2 - y1, x2 - x1);
            pDrawLine(color, size, x2, y2, x2 - headlen * cosf(angle - 3.14159 / 6), y2 - headlen * sinf(angle - 3.14159 / 6));
            pDrawLine(color, size, x2, y2, x2 - headlen * cosf(angle + 3.14159 / 6), y2 - headlen * sinf(angle + 3.14159 / 6));
        }
    }
    void pDrawRect(const Gdiplus::ARGB& color, const int& size, const int& x, const int& y, const int& w, const int& h, const bool fill = true) {
        if (gf) {
            int Sz = size - 1;
            Gdiplus::Color clr = Gdiplus::Color(color);
            Gdiplus::Pen pen(clr, (fill ? Sz : 0));      // For lines, rectangles and curves
            gf->DrawRectangle(&pen, x, y, w, h);
            DeleteObject(&pen);
        }
    }
    //TODO
    void pDrawCopy() {
        if (gf) {
        }
    }
    void pDrawPic(const std::string& File, const int& x, const int& y) {
        if (gf) {
            Gdiplus::Bitmap bmp(toWideString(File).c_str());
            gf->DrawImage(&bmp, x, y);
            DeleteObject(&bmp);
        }
    }
    void pDrawRotPic(const std::string& File, const float& rad, int x, int y) {
        if (gf) {
            Gdiplus::Bitmap bmp(toWideString(File).c_str());
            int bw = bmp.GetWidth() / 2;
            int bh = bmp.GetHeight() / 2;
            x -= bw;
            y -= bh;
            Gdiplus::PointF center(x + bw, y + bh);
            Gdiplus::Matrix matrix;
            matrix.RotateAt(rad, center);

            gf->SetTransform(&matrix);
            gf->DrawImage(&bmp, x, y);
            gf->ResetTransform();
            DeleteObject(&bmp);
        }
    }
    //TODO: Flush HBITMAP backbuffer to file
    void pDrawSave() {
        if (gf) {
        }
    }
    void pDrawText(const Gdiplus::ARGB& color, const std::string& Name, const int& size, int x, int y, const std::string& Text, const bool centered = false) {
        if (gf) {
            std::wstring name = std::wstring(Name.begin(), Name.end());
            std::wstring text = std::wstring(Text.begin(), Text.end());
            Gdiplus::Color clr = Gdiplus::Color(color);
            Gdiplus::SolidBrush brush(clr);  // For filled shapes
            Gdiplus::Font fnt(name.c_str(), size);
            Gdiplus::RectF Bounds;
            gf->MeasureString(text.c_str(), text.length(), &fnt, Gdiplus::PointF(0, 0), &Bounds);
            if (centered) {
                x -= Bounds.Width / 2 - 1;
                y -= Bounds.Height / 2 - 1;
            }

            gf->DrawString(text.c_str(), -1, &fnt, Gdiplus::PointF(x, y), &brush);
            DeleteObject(&brush);
            DeleteObject(&fnt);
        }
    }
    void pClear(const Gdiplus::ARGB color = Gdiplus::Color::MakeARGB(255, 0, 0, 0)) {
        if (gf) {
            Gdiplus::Color clr = Gdiplus::Color(color);
            gf->Clear(clr);
        }
    }
    void pFlip() {
        if (gf) {
            //force-redraw window!
            RedrawWindow(hwndC, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
        }
    }

    //Handy dandy Messagebox routine... We'll leave it public for now...
    void MsgBox(const char* Prompt, const char* Options, const char* Title, const char* Text) {
        long styles = MB_APPLMODAL;
        if (!lstrcmp(Prompt, "a")) { styles |= MB_ABORTRETRYIGNORE; }
        else if (!lstrcmp(Prompt, "c")) { styles |= MB_OKCANCEL; }
        else if (!lstrcmp(Prompt, "o")) { styles |= MB_OK; }
        else if (!lstrcmp(Prompt, "r")) { styles |= MB_RETRYCANCEL; }
        if (!lstrcmp(Options, "a")) { styles |= MB_ICONASTERISK; }
        else if (!lstrcmp(Options, "e")) { styles |= MB_ICONEXCLAMATION; }
        else if (!lstrcmp(Options, "w")) { styles |= MB_ICONWARNING; }
        else if (!lstrcmp(Options, "r")) { styles |= MB_ICONERROR; }
        else if (!lstrcmp(Options, "h")) { styles |= MB_ICONHAND; }
        else if (!lstrcmp(Options, "q")) { styles |= MB_ICONQUESTION; }
        else if (!lstrcmp(Options, "i")) { styles |= MB_ICONINFORMATION; }
        else if (!lstrcmp(Options, "s")) { styles |= MB_ICONSTOP; }
        MessageBox(hwndC, Text, Title, styles);
    }
};
bool tMainWindow::HasRegisteredWindowClass = false;
LRESULT CALLBACK tMainWindow::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_NCCREATE) { SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT(lParam))->lpCreateParams)); }
    tMainWindow* Window = (tMainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (Window && Window->hwnd() == hwnd) { return Window->WinProc(hwnd, message, wParam, lParam); }
    return DefWindowProc(hwnd, message, wParam, lParam);
}
LRESULT CALLBACK tMainWindow::MDIWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCPAINT:
        break;
    default:
        break;
    }

    return DefMDIChildProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK tMainWindow::WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    PAINTSTRUCT ps;
    POINT pnt;
    RECT inside, outside;
    int om, nm;
    bool flagrebuffer = false;

    if (GetCursorPos(&pnt)) {
        Mouse.dx = pnt.x; Mouse.dy = pnt.y;
        ScreenToClient(hwndC, &pnt);
        Mouse.x = pnt.x; Mouse.y = pnt.y;
    }
    switch (msg) {
    case MM_MCINOTIFY:
        break;
    case MM_MCISIGNAL:
        break;
    case WM_TIMER:
        if (hwnd == hwndC) {
            if (wParam == rTimerID) {
                if (IsThreaded || IsGraphicalGDI) {
                    lCheck = std::chrono::system_clock::now();
                    std::chrono::duration<float> elapsedTime = lCheck - lStart;
                    lStart = lCheck;
                    lElapsedTime = elapsedTime.count();
                    if (IsGraphicalGDI) {
                        //thread dbl buffers another dbl buffer lol, it does all it's magic to a hbitmap and relies on a windows
                        // multimedia timer on the main thread to decide to blit the image across where as unthreaded just renders as soon as it computes the frame
                        if (!IsThreaded) { OnUpdate(lElapsedTime); Render(lElapsedTime); }
                        else { Render(lElapsedTime); }
                    }
                }
            }
            else { OntTimer(wParam); }
        }
        break;
        //Tray icon behavior tests
    case WM_USER:
        switch (lParam) {
        case WM_LBUTTONDBLCLK:
            ShowWindow(hwnd, SW_SHOWNA);
            SetActiveWindow(hwnd);
            SetForegroundWindow(hwnd);
            return true;
        case WM_RBUTTONUP:
            HMENU myMenu = NULL;
            myMenu = CreatePopupMenu();
            AppendMenu(myMenu, MF_STRING, 0, "Exit");
            POINT mp;
            GetCursorPos(&mp);
            UINT clicked = TrackPopupMenu(myMenu, TPM_RETURNCMD | TPM_NONOTIFY, mp.x, mp.y, 0, hwnd, NULL);
            SendMessage(hwnd, WM_NULL, 0, 0);
            if (clicked == 0) SendMessage(hwnd, WM_DESTROY, 0, 0);
            break;
        }
        break;
    case WM_NCCREATE:
    case WM_CREATE:
        OnInit();
        break;
    case WM_ACTIVATEAPP:
        if (wParam == false) { OntLostFocus(); }
        break;
    case WM_CTLCOLORSTATIC:
        //Fix to be transparent, to conform with more modern windows versions, defaults to btn_face...
        if ((GetWindowLong((HWND)lParam, GWL_STYLE) & SS_NOTIFY)) {
            hdc = GetDC((HWND)lParam);
            SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 0, 156));
            ReleaseDC(nullptr, hdc);
            return (LRESULT)(HBRUSH)GetStockObject(NULL_BRUSH);
        }
        break;
    case WM_CLOSE:
        if (!OnClose()) { DestroyWindow(hwnd); }
        else {
            Show(false);
            return 0;
        }
        break;
    case WM_DESTROY:
        //There's no hope! Attempt to cleanup and exit gracefully
        if (hwnd == hwndC) {
            bAtomActive = false;
            Show(false);
            OnExit();
            Shell_NotifyIcon(NIM_DELETE, &NotifyIconData);
            DeleteObject(&gf);
            DeleteObject(&backbuffer);
            DeleteDC(backbuffDC);
            Gdiplus::GdiplusShutdown(gdiplusToken);
            PostQuitMessage(0);
        }
        break;
    case WM_SIZING:
    case WM_SIZE:
    case WM_MOVE:
        if (hwnd == hwndC) { //Is it the main window? we don't wanna muck things up from children events
            GetWindowRect(hwnd, &outside);
            if (outside.right - outside.left != Width) { flagrebuffer = true; }
            if (outside.bottom - outside.top != Height) { flagrebuffer = true; }
            GetClientRect(hwnd, &inside);

            wX = outside.left;
            wY = outside.top;
            wiX = inside.left;
            wiY = inside.top;
            Width = outside.right - outside.left;
            iWidth = inside.right - inside.left;
            Height = outside.bottom - outside.top;
            iHeight = inside.bottom - inside.top;
            if (hStatusC) { //Do we have a statusbar and is it even visible?
                SendMessage(hStatusC, WM_SIZE, 0, 0);
                GetClientRect(hStatusC, &inside);
                if (IsWindowVisible(hStatusC)) { iHeight -= inside.bottom - inside.top; }
            }
            //Fix MDI container size if we're an MDI app
            if (hwndMDI) { SetWindowPos(hwndMDI, NULL, 0, 0, iWidth, iHeight, SWP_NOZORDER); }

            //Destroy and re-create a new HBITMAP to be the same size as the new window size
            if (flagrebuffer) {
                if (IsGraphicalGDI) { LoadNewGDIBackBuffer(false); Render(0); }
                return 1;
            }
        }
        break;
    case WM_ERASEBKGND:
        if (hwnd == hwndC && IsGraphicalGDI) { return 1; }
        break;
    case WM_PAINT:
        if (hwnd == hwndC && IsGraphicalGDI) {
            hdc = BeginPaint(hwnd, &ps);
            BitBlt(hdc, 0, 0, width(), height(), backbuffDC, 0, 0, SRCCOPY);
            EndPaint(hwnd, &ps);
        }
        break;
    case WM_HOTKEY:
        OntHotkey(wParam);
        break;
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        KeyboardState[mapKeys[wParam]] = !KeyboardState[mapKeys[wParam]];
        if (!KeyboardState[mapKeys[wParam]]) { OnKeyUp(mapKeys[wParam],wParam); }
        else { OnKeyDown(mapKeys[wParam],wParam); }
        break;
    case WM_NCMOUSELEAVE:
    case WM_MOUSELEAVE:
        OntMouseLeave();
    case WM_MOUSEMOVE:
        OntMouseMove();
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        om = Mouse.key;
        nm = 0;
        //wParam mouse structure isn't the same as my own so I can't just assign what's here over...
        //gotta do some comparisons and XOR my own stuff when things change.
        if ((wParam & MK_LBUTTON) != 0) { nm ^= 1; }
        if ((wParam & MK_RBUTTON) != 0) { nm ^= 16; }
        if ((wParam & MK_CONTROL) != 0) { nm ^= 2; }
        if ((wParam & MK_SHIFT) != 0) { nm ^= 4; }
        if ((wParam & MK_ALT) != 0) { nm ^= 8; }
        Mouse.key = nm;
        //Determine what's different between the last state and trigger events accordingly
        if (nm & 1 && !(om & 1)) { SetCapture(hwndC); OntMouseSclick(); }
        if (nm & 16 && !(om & 16)) { SetCapture(hwndC); OntMouseRclick(); }
        if (!(nm & 1) && (om & 1)) { ReleaseCapture();  OntMouseUclick(); }
        if (!(nm & 16) && (om & 16)) { ReleaseCapture(); OntMouseUclick(); }
        break;
    case WM_SYSCOMMAND:
        switch (HIWORD(wParam)) {
        case SC_MINIMIZE:
            Show(false);
            break;
        default:
            break;
        }
        break;
    case WM_COMMAND:
        switch (HIWORD(wParam)) {
        case BN_CLICKED:
            OnSclick((HWND)lParam, LOWORD(wParam));
            break;
        case BN_DOUBLECLICKED:
            OnDclick((HWND)lParam, LOWORD(wParam));
            break;
        case LBN_SELCHANGE:
            OnSclick((HWND)lParam, LOWORD(wParam));
            break;
        case LBN_DBLCLK:
            OnDclick((HWND)lParam, LOWORD(wParam));
            break;
        case EN_UPDATE:
            OnEdit((HWND)lParam, LOWORD(wParam));
            break;
        default:
            break;
        }
        //menu id stuffs?? handled differently now due do ID scheme... left for a placeholder.
        //switch (LOWORD(wParam)) {
        //default:
        //    break;
        //}
        break;
    default:
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/*=======================================================================================================================
* Base tDialog Class
=======================================================================================================================*/

class tDialog : public tMainWindow {
private:
    tMainWindow* Parent;
public:
    tDialog(
        tMainWindow* parent,                                 //Who do I belong to?
        int rx,                                              //Requested screen location X
        int ry,                                              //Requested screen location Y
        int rw,                                              //Requested width
        int rh,                                              //Requested height
        std::string Icon,                                         //Window Icon
        std::string Cursor,                                       //Window Mouse Cursor
        std::string Caption,                                      //Window Title
        long stylesEx = 0 | DS_CONTROL,                 //Default extended styles
        long styles = WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, //Default styles
        bool isInternalSized = false,       //Upon creation, is the outer final dimensions (boder window title n all) or the inner dimensions? (window surface)
        bool isMainApplication = false,      //Should we end the entire app if we're destroyed.
        bool isMDIApplication = false,       //Should we make a mdi child area?
        bool isModal = false,               //Are we a modal dialog to some other hwnd? (window)
        bool isThreaded = false,            //Are we using an std::thread alongside our message loop?
        bool isGraphicalGDI = false        //Are we planning on just drawing stuff via GDIPlus?
    ) : tMainWindow(rx, ry, rw, rh, Icon, Cursor, Caption, stylesEx, styles, isInternalSized, isMainApplication, isMDIApplication, isModal, isThreaded, isGraphicalGDI) {
        //Placeholder constructor
        Parent = parent;
    }
    virtual ~tDialog() {
        //Placeholder destructor
    }
    tMainWindow* parent() { return Parent; }
};

/*=======================================================================================================================
* Base tDialog Menu Class
=======================================================================================================================*/

class tMenuBase {
private:
    tDialog* Parent;
    HMENU hmenuC;
    HMENU hmenuP;
    int ID;

public:
    int id() { return ID; }
    tMenuBase(tDialog* parent, std::string Caption, long styles = MF_STRING | MF_POPUP) {
        Parent = parent;
        ID = Parent->GetNexTAvaialiableChildID();

        if (!IsMenu(hmenuC)) { hmenuC = CreateMenu(); SetMenu(Parent->hwnd(), hmenuC); }
        if (ID > 0) { styles &= ~MF_POPUP; AppendMenu(hmenuC, styles, ID, Caption.c_str()); }
        else {
            hmenuP = CreatePopupMenu();
            if (!lstrcmp(Caption.c_str(), "-")) { AppendMenu(hmenuP, MF_SEPARATOR, 0, ""); }
            //else { AppendMenu(hmenuC, styles, (int)hmenuP, Caption.c_str()); }
        }
        DrawMenuBar(Parent->hwnd());
    }
    virtual ~tMenuBase() { }

};

class tMenu : public tMenuBase {
private:
public:
    tMenu(tDialog* parent, std::string Caption, long styles = MF_STRING) : tMenuBase(parent, Caption, styles) { }
    virtual ~tMenu() { }
};

/*=======================================================================================================================
* Base tDialog Widget Class
=======================================================================================================================*/

class tWidget {
private:
    tDialog* Parent;
    HWND hwndC;
    int ID;

public:
    int id() { return ID; }
    tWidget(tDialog* parent, LPCSTR type, std::string Caption, long styles, int x, int y, int w, int h) {
        Parent = parent;
        ID = Parent->GetNexTAvaialiableChildID();
        if (!lstrcmp("Edit", type)) { hwndC = CreateWindowEx(WS_EX_CLIENTEDGE, type, Caption.c_str(), styles, x, y, w, h, Parent->hwnd(), (HMENU)ID, NULL, NULL); }
        //else { hwndC = CreateWindowEx(WS_EX_TRANSPARENT, type, Caption.c_str(), styles, x, y, w, h, Parent->hwnd(), (HMENU)ID, NULL, NULL); }
        else { hwndC = CreateWindowEx(0, type, Caption.c_str(), styles, x, y, w, h, Parent->hwnd(), (HMENU)ID, NULL, NULL); }
        // The font for objects is ugly (FixedSys).. Use windows default GUI font.
        SendMessage(hwndC, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)TRUE);
        ShowWindow(hwndC, SW_SHOW);
    }
    virtual ~tWidget() { }
    HWND hwnd() { return hwndC; }
    tDialog* parent() { return Parent; }
    void Focus() { SetFocus(hwndC); }
    void Show() { ShowWindow(hwndC, SW_SHOW); }
    void Hide() { ShowWindow(hwndC, SW_HIDE); }
    void SetEnabled(bool enable) { EnableWindow(hwndC, enable); }
    void SetText(std::string text) { SendMessage(hwndC, WM_SETTEXT, 0, (LPARAM)TEXT(text.c_str())); }
    std::string GetText() {
        int szSize = SendMessage(hwndC, WM_GETTEXTLENGTH, 0, 0L) + 1;
        char* szString = new char[szSize];
        GetDlgItemText(Parent->hwnd(), id(), szString, szSize);
        std::string ret = szString;
        delete szString;
        return ret;
    }
};


/*=======================================================================================================================
* tDialog Widget Check Class
=======================================================================================================================*/

class tCheck : public tWidget {
private:
public:
    tCheck(tDialog* parent, std::string Caption, long styles, int x, int y, int w, int h) : tWidget(parent, "Button", Caption, styles | WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, x, y, w, h) { }
    UINT State() { return IsDlgButtonChecked(this->parent()->hwnd(), this->id()); }
    virtual ~tCheck() { }
};

/*=======================================================================================================================
* tDialog Widget Combo Class
=======================================================================================================================*/

class tCombo : public tWidget {
private:
public:
    tCombo(tDialog* parent, std::string Caption, long styles, int x, int y, int w, int h) : tWidget(parent, "ComboBox", Caption, styles | WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST, x, y, w, h) { }
    virtual ~tCombo() { }
    void AddItem(std::string Caption) { SendMessage(hwnd(), CB_ADDSTRING, 0, (LPARAM)TEXT(Caption.c_str())); }
    void InsertItem(int Index, std::string Caption) { SendMessage(hwnd(), CB_INSERTSTRING, Index, (LPARAM)TEXT(Caption.c_str())); }
    void DeleteItem(int Index) { SendMessage(hwnd(), CB_DELETESTRING, Index, 0); }
    void SelectItem(int Index) { SendMessage(hwnd(), CB_SETCURSEL, Index, 0); }
    int SelectedIndex() { return SendMessage(hwnd(), CB_GETCURSEL, NULL, NULL); }
    void Clear() { SendMessage(hwnd(), CB_RESETCONTENT, 0, 0); }
    void OverwriteItem(int Index, std::string Caption) { DeleteItem(Index); InsertItem(Index, Caption); }
};

/*=======================================================================================================================
* tDialog Widget List Class
=======================================================================================================================*/

class tList : public tWidget {
private:
public:
    tList(tDialog* parent, std::string Caption, long styles, int x, int y, int w, int h) : tWidget(parent, "Static", Caption, styles | WS_CHILD | LBS_NOTIFY, x, y, w, h) { }
    virtual ~tList() { }
    void AddItem(std::string Caption) { SendMessage(hwnd(), LB_ADDSTRING, 0, (LPARAM)TEXT(Caption.c_str())); }
    void InsertItem(int Index, std::string Caption) { SendMessage(hwnd(), LB_INSERTSTRING, Index, (LPARAM)TEXT(Caption.c_str())); }
    void DeleteItem(int Index) { SendMessage(hwnd(), LB_DELETESTRING, Index, 0); }
    void SelectItem(int Index) { SendMessage(hwnd(), LB_SETCURSEL, Index, 0); }
    int SelectedIndex() { return SendMessage(hwnd(), LB_GETCURSEL, NULL, NULL); }
    void Clear() { SendMessage(hwnd(), LB_RESETCONTENT, 0, 0); }
    void OverwriteItem(int Index, std::string Caption) { DeleteItem(Index); InsertItem(Index, Caption); }
};

/*=======================================================================================================================
* tDialog Widget Button Class
=======================================================================================================================*/

class tButton : public tWidget {
private:
public:
    tButton(tDialog* parent, std::string Caption, long styles, int x, int y, int w, int h, bool defpb = false) : tWidget(parent, "Button", Caption, styles | WS_CHILD | WS_TABSTOP | (defpb ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON), x, y, w, h) { }
    virtual ~tButton() { }
};

/*=======================================================================================================================
* tDialog Widget Box Class
=======================================================================================================================*/

class tBox : public tWidget {
private:
public:
    tBox(tDialog* parent, std::string Caption, long styles, int x, int y, int w, int h) : tWidget(parent, "Button", Caption, styles | WS_CHILD | BS_GROUPBOX | SS_NOTIFY, x, y, w, h) { }
    virtual ~tBox() { }
};

/*=======================================================================================================================
* tDialog Widget Edit Box Class
=======================================================================================================================*/

class tEdit : public tWidget {
private:
public:
    tEdit(tDialog* parent, std::string Caption, long styles, int x, int y, int w, int h) : tWidget(parent, "Edit", Caption, styles | WS_CHILD | WS_TABSTOP | DS_CONTROL, x, y, w, h) { }
    virtual ~tEdit() { }
};

/*=======================================================================================================================
* tDialog Widget Icon Class
=======================================================================================================================*/

class tIcon : public tWidget {
private:
public:
    tIcon(tDialog* parent, std::string Caption, long styles, int x, int y, int w, int h) : tWidget(parent, "Static", Caption, styles | WS_CHILD | SS_BITMAP, x, y, w, h) {
        HBITMAP TheBmp = (HBITMAP)LoadImage(0, Caption.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        SendMessage(hwnd(), (UINT)STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)TheBmp);
    }
    virtual ~tIcon() { }
};


/*=======================================================================================================================
* tDialog Widget Text Label Class
=======================================================================================================================*/

class tRadio : public tWidget {
private:
public:
    tRadio(tDialog* parent, std::string Caption, long styles, int x, int y, int w, int h) : tWidget(parent, "Button", Caption, styles | WS_CHILD | WS_TABSTOP | BS_AUTORADIOBUTTON, x, y, w, h) { }
    UINT State() { return IsDlgButtonChecked(this->parent()->hwnd(), this->id()); }
    virtual ~tRadio() { }
};

/*=======================================================================================================================
* tDialog Widget Text Label Class
=======================================================================================================================*/

class tText : public tWidget {
private:
public:
    tText(tDialog* parent, std::string Caption, long styles, int x, int y, int w, int h) : tWidget(parent, "Static", Caption, styles | WS_CHILD | SS_NOTIFY, x, y, w, h) { }
    virtual ~tText() { }
};