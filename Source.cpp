#include "tGui.h"
#define TIMERID 1001
#define HOTKEYID 1234

/*---------------------------------------------------------------------------------------------------------------
Global Defined stuffs
---------------------------------------------------------------------------------------------------------------*/

std::map<int, int> MouseDown = { {0,WM_LBUTTONDOWN}, {1,WM_RBUTTONDOWN}, {2,WM_MBUTTONDOWN} };
std::map<int, int> MouseDouble = { {0,WM_LBUTTONDBLCLK}, {1,WM_RBUTTONDBLCLK}, {2,WM_MBUTTONDBLCLK} };
std::map<int, int> MouseUp = { {0,WM_LBUTTONUP}, {1,WM_RBUTTONUP}, {2,WM_MBUTTONUP} };

void PostMouse(HWND handle, int Button, int ClickType, int x, int y) {
    PostMessage(handle, (ClickType == 0 ? MouseDown[Button] : MouseDouble[Button]), 1, MAKELPARAM(x, y));
    PostMessage(handle, MouseUp[Button], 0, MAKELPARAM(x, y));
}

/*---------------------------------------------------------------------------------------------------------------
overload tDialog (which is an overloaded tMainWindow....)
---------------------------------------------------------------------------------------------------------------*/

class MyApp : public tDialog {
private:
    bool FindApp = false;
    bool GetCoord = false;
    bool SetHotkey = false;
    bool Clicking = false;

    //Declare pointers for controls which we'll create in the constructor...
    tBox *Box1, *Box2, *Box3, *Box4;
    tText *Label1, *Label2, *Label3, *Label4, *Label5, *Label6, *Label7, *Label8, *Label9, *Label10, *Label11, *Label12, *Label13, *Label14;
    tEdit *Editbox1, *Editbox2, *Editbox3, *Editbox4, *Editbox5, *Editbox6, *Editbox7, *Editbox8, *Editbox9, *Editbox10, *Editbox11;
    tButton *Button1, *Button2, *Button3, *Button4, *Button5;
    tCheck *Check1, *Check2;
    tCombo *Combo1, *Combo2;

public:
    UINT ClickCount = 0;
    UINT LimitCount = 0;
    UINT RepeatCount = 0;

    MyApp(
        tMainWindow* Parent,                                 //Owner window, use NULL if it is to be the owner
        int rx,                                              //Requested screen location X
        int ry,                                              //Requested screen location Y
        int rw,                                              //Requested width
        int rh,                                              //Requested height
        std::string Icon,                                    //Window Icon
        std::string Cursor,                                  //Window Mouse Cursor
        std::string Caption,                                 //Window Title
        long stylesEx = WS_EX_CONTROLPARENT | WS_EX_TOPMOST,                 //Default extended styles
        long styles = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, //Default styles
        bool isInternalSized = true,       //Upon creation, is the outer final dimensions (boder window title n all) or the inner dimensions? (window surface)
        bool isMainApplication = true,      //Should we end the entire app if we're destroyed.
        bool isMDIApplication = false,      //Should we make an MDI client area?
        bool isModal = false,               //Are we a modal dialog to some other hwnd? (window)
        bool isThreaded = false,            //Are we using an std::thread alongside our message loop?
        bool isGraphicalGDI = false        //Are we planning on just drawing stuff via GDIPlus?
    ) : tDialog(Parent, rx, ry, rw, rh, Icon, Cursor, Caption, stylesEx, styles, isInternalSized, isMainApplication, isMDIApplication, isModal, isThreaded, isGraphicalGDI) {

        ShowStatus(); //Should we have a status bar?
        SetStatusbarTitle("Stopped.");
         
        //Create controls!
        Box1 = new tBox(this, "Click Interval", WS_VISIBLE, 4, 4, 424, 68);
        Box2 = new tBox(this, "Click Options", WS_VISIBLE, 4, 76, 208, 100);
        Box3 = new tBox(this, "Assign Application", WS_VISIBLE, 220, 76, 208, 100);
        Box4 = new tBox(this, "Cursor Position", WS_VISIBLE, 4, 180, 424, 48);

        Label1 = new tText(this, "hours", WS_VISIBLE, 74, 24, 32, 16);
        Label2 = new tText(this, "mins", WS_VISIBLE, 174, 24, 26, 16);
        Label3 = new tText(this, "secs", WS_VISIBLE, 268, 24, 26, 16);
        Label4 = new tText(this, "milliseconds", WS_VISIBLE, 362, 24, 60, 16);
        Label5 = new tText(this, "milliseconds", WS_VISIBLE, 174, 48, 60, 16);
        Label6 = new tText(this, "Repeat:", WS_VISIBLE, 254, 48, 44, 16);
        Label7 = new tText(this, "times", WS_VISIBLE, 362, 48, 50, 16);
        Label8 = new tText(this, "Mouse Button:", WS_VISIBLE, 12, 94, 76, 16);
        Label9 = new tText(this, "Click Type:", WS_VISIBLE, 12, 122, 62, 16);
        Label10 = new tText(this, "Title:", WS_VISIBLE, 230, 94, 32, 16);
        Label11 = new tText(this, "Hwnd:", WS_VISIBLE, 230, 118, 38, 16);
        Label12 = new tText(this, "X:", WS_VISIBLE, 250, 198, 18, 16);
        Label13 = new tText(this, "Y:", WS_VISIBLE, 336, 198, 18, 16);
        Label14 = new tText(this, "Current Hotkey:", WS_VISIBLE, 4, 236, 80, 16);

        Editbox1 = new tEdit(this, "0", WS_VISIBLE | ES_NUMBER | SS_RIGHT, 10, 22, 60, 20);
        Editbox2 = new tEdit(this, "0", WS_VISIBLE | ES_NUMBER | SS_RIGHT, 110, 22, 60, 20);
        Editbox3 = new tEdit(this, "0", WS_VISIBLE | ES_NUMBER | SS_RIGHT, 204, 22, 60, 20);
        Editbox4 = new tEdit(this, "100", WS_VISIBLE | ES_NUMBER | SS_RIGHT, 298, 22, 60, 20);
        Editbox5 = new tEdit(this, "40", WS_VISIBLE | ES_NUMBER | SS_RIGHT, 110, 46, 60, 20);
        Editbox6 = new tEdit(this, "0", WS_VISIBLE | ES_NUMBER | SS_RIGHT, 298, 46, 60, 20);
        Editbox7 = new tEdit(this, "", WS_VISIBLE | SS_CENTER | ES_READONLY, 276, 92, 140, 20);
        Editbox8 = new tEdit(this, "", WS_VISIBLE | SS_CENTER | ES_READONLY, 276, 116, 140, 20);
        Editbox9 = new tEdit(this, "0", WS_VISIBLE | ES_NUMBER | SS_CENTER, 272, 196, 60, 20);
        Editbox10 = new tEdit(this, "0", WS_VISIBLE | ES_NUMBER | SS_CENTER, 356, 196, 60, 20);
        Editbox11 = new tEdit(this, "", WS_VISIBLE | SS_CENTER | ES_READONLY, 88, 234, 40, 20);

        Check1 = new tCheck(this, "Random Offset", WS_VISIBLE, 12, 46, 96, 20);
        Check2 = new tCheck(this, "Use Defined Location", WS_VISIBLE, 12, 196, 130, 20);

        Button1 = new tButton(this, "Get Location", WS_VISIBLE, 170, 194, 74, 24);
        Button1->SetEnabled(false);
        Button2 = new tButton(this, "Set Target Application", WS_VISIBLE, 286, 140, 120, 24);
        Button3 = new tButton(this, "Assign Key", WS_VISIBLE, 132, 232, 74, 24);
        Button4 = new tButton(this, "Start", WS_VISIBLE, 274, 232, 74, 24);
        Button5 = new tButton(this, "Stop", WS_VISIBLE, 354, 232, 74, 24);
        Button5->SetEnabled(false);

        Combo1 = new tCombo(this, "", WS_VISIBLE, 102, 92, 100, 100);
        Combo2 = new tCombo(this, "", WS_VISIBLE, 102, 120, 100, 100);

        Combo1->AddItem("Left");
        Combo1->AddItem("Middle");
        Combo1->AddItem("Right");
        Combo1->SelectItem(0);

        Combo2->AddItem("Single");
        Combo2->AddItem("Double");
        Combo2->SelectItem(0);
    }
    ~MyApp() { } //Placeholder destructor

    /*---------------------------------------------------------------------------------------------------------------
    Custom Public Stuffs here
    ---------------------------------------------------------------------------------------------------------------*/

    void StartAutomation(bool Offset = false) {
        Clicking = true;
        int Hours = atoi(Editbox1->GetText().c_str());
        int Minutes = atoi(Editbox2->GetText().c_str());
        int Seconds = atoi(Editbox3->GetText().c_str());
        int Milliseconds = atoi(Editbox4->GetText().c_str());
        int Random = rand() % (atoi(Editbox5->GetText().c_str()) + 1);
        int Total = Hours * 3600000 + Minutes * 60000 + Seconds * 1000 + Milliseconds + (Offset == true ? Random : 0);
        SetTimer(hwnd(), TIMERID, Total, (TIMERPROC)NULL);
        Button4->SetEnabled(false);
        Button5->SetEnabled(true);
        SetStatusbarTitle("Running, Clicks: " + std::to_string(ClickCount));
    }
    void StopAutomation() {
        Clicking = false;
        KillTimer(hwnd(), TIMERID);
        Button4->SetEnabled(true);
        Button5->SetEnabled(false);
        SetStatusbarTitle("Stopped, Clicks: " + std::to_string(ClickCount));
    }

    /*---------------------------------------------------------------------------------------------------------------
    Custom Virtual Callback Functions (overrides)
    ---------------------------------------------------------------------------------------------------------------*/

    void OnExit() override { } //Impending DOOM! Die phase... Cleanup what you can!
    bool OnClose() override { return false; } //this event lets us return true to prevent the window from exiting, it instead hides.
    void OnInit() override { } //Read config somewhere and assign values to controls? etc....

    void OnKeyDown(const int32_t& key, const UINT& vk) override {
        if (key == Key::ESCAPE) { Exit(); }
        if (SetHotkey == true) {
            Editbox11->SetText(KeyName[static_cast<Key>(key)]);
            Button3->SetEnabled(true);
            DelHotkey(HOTKEYID);
            AddHotkey(HOTKEYID, 0, vk);
            SetHotkey = false;
        }
    }
    void OntMouseSclick() override {
        if (GetCoord == true) {
            ReleaseCapture();
            RECT wr;
            GetWindowRect((HWND)atoi(Editbox8->GetText().c_str()), &wr);

            Editbox9->SetText(std::to_string(mouse().dx - wr.left));
            Editbox10->SetText(std::to_string(mouse().dy - wr.top));
            GetCoord = false;
            Button1->SetEnabled(true);
        }
        if (FindApp == true) {
            ReleaseCapture();
            POINT CursorPos;
            CursorPos.x = mouse().dx;
            CursorPos.y = mouse().dy;
            HWND MouseOver = WindowFromPoint(CursorPos);
            int szSize = SendMessage(MouseOver, WM_GETTEXTLENGTH, 0, 0L) + 1;
            char* szString = new char[szSize];
            GetWindowText(MouseOver, szString, szSize);
            Editbox7->SetText(szString);
            delete szString;

            Editbox8->SetText(std::to_string((int)MouseOver));
            FindApp = false;
            Button1->SetEnabled(true);
            Button2->SetEnabled(true);
        }
    }
    void OntHotkey(const int& ID) override {
        if (ID == HOTKEYID) {
            if (Clicking == false) { StartAutomation(); }
            else { StopAutomation(); }
        }
    }
    void OntTimer(const int& ID) override {
        POINT CursorPos;
        int Limit = atoi(Editbox6->GetText().c_str());

        if (Check2->State() == BST_CHECKED) {
            //RECT wr;
            //GetWindowRect((HWND)atoi(Editbox8->GetText().c_str()), &wr);
            //SetCursorPos(atoi(Editbox9->GetText().c_str()) + wr.left, atoi(Editbox10->GetText().c_str()) + wr.top);
            CursorPos.x = atoi(Editbox9->GetText().c_str());  // + wr.left
            CursorPos.y = atoi(Editbox10->GetText().c_str()); // + wr.top
        }
        else { GetCursorPos(&CursorPos); }

        PostMouse((HWND)atoi(Editbox8->GetText().c_str()), Combo1->SelectedIndex(), Combo2->SelectedIndex(), CursorPos.x, CursorPos.y);
        SetStatusbarTitle("Running, Clicks: " + std::to_string(++ClickCount));
        if (Limit > 0) {
            LimitCount++;
            if (LimitCount == Limit) { StopAutomation(); LimitCount = 0; }
        }
        if (Check1->State() == BST_CHECKED) { StartAutomation(true); }
    }
    void OnSclick(HWND item, const int& ID) override {
        if (Button1->id() == ID) {
            GetCoord = true;
            Button1->SetEnabled(false);
            SetCapture(hwnd());
        }
        if (Button2->id() == ID) {
            FindApp = true;
            Button2->SetEnabled(false);
            SetCapture(hwnd());
        }
        if (Button3->id() == ID) {
            SetHotkey = true;
            Button3->SetEnabled(false);
        }
        if (Button4->id() == ID) { StartAutomation();  }
        if (Button5->id() == ID) { StopAutomation(); }

        //Reset to normal if we uncheck offset so our timer doesn't run with the last random offset applied
        if (Check1->id() == ID && Check1->State() == BST_UNCHECKED && Clicking == true) { StartAutomation(); }
    }
};

/*---------------------------------------------------------------------------------------------------------------
Run our Application
---------------------------------------------------------------------------------------------------------------*/

int WINAPI WinMain(HINSTANCE currentInstance, HINSTANCE previousInstance, PSTR cmdLine, INT cmdCount) {
    MyApp Clicker(NULL, -1, -1, 432, 324, "talon.ico", "", "Talon's Auto-Clicker v1.0 BETA!");
    Clicker.Run();
}

/* Dialog Studio Table
dialog new_table {
  title "New Project"
  size -1 -1 432 262
  option pixels
  box "Click Interval", 1, 4 4 424 68
  edit "0", 2, 10 22 60 20, right
  edit "0", 3, 110 22 60 20, right
  edit "0", 4, 204 22 60 20, right
  edit "100", 5, 298 22 60 20, right
  check "Random Offset", 6, 12 46 100 20
  edit "100", 7, 110 46 60 20, right
  text "hours", 8, 74 24 32 16
  text "mins", 9, 174 24 26 16
  text "secs", 10, 268 24 26 16
  text "milliseconds", 11, 362 24 60 16
  text "milliseconds", 12, 174 48 60 16
  edit "0", 13, 298 46 60 20, right
  text "Repeat:", 14, 254 48 44 16
  text "times", 15, 362 48 50 16
  box "Click Options", 16, 4 76 208 100
  box "Assign Application", 17, 220 76 208 100
  box "Cursor Position", 18, 4 180 424 48
  text "Mouse Button:", 19, 12 94 76 16
  combo 20, 102 92 100 100, size drop
  combo 21, 102 120 100 100, size drop
  text "Click Type:", 22, 12 122 62 16
  text "Title:", 23, 230 94 32 16
  edit "Minecraft* 1.20.4", 24, 276 92 140 20, center
  text "Hwnd:", 25, 230 118 38 16
  edit "", 26, 276 116 140 20, center
  check "Use Defined Location", 27, 12 196 130 20
  edit "0", 28, 272 196 60 20, center
  edit "0", 29, 356 196 60 20, center
  text "X:", 30, 250 198 18 16
  text "Y:", 31, 336 198 18 16
  button "Get Location", 32, 170 194 74 24
  button "Set Target Application", 33, 286 140 120 24
  text "Current Hotkey:", 34, 4 236 80 16
  edit "`", 35, 88 234 40 20
  button "Assign Key", 36, 132 232 74 24
  button "Start", 37, 274 232 74 24
  button "Stop", 38, 354 232 74 24
}
*/
