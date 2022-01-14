#pragma once

#include "pch.h"
#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "Menu.h"
#include <Windows.h>

#include "ini.h"

#ifdef _WIN32
#include <io.h> 
#define access    _access_s
#else
#include <unistd.h>
#endif

#define INAME L"VALORANT  " 


using namespace std;

typedef int(*pDD_btn)(int btn);
typedef int(*pDD_movR)(int dx, int dy);

pDD_btn      DD_btn;          //Mouse button
pDD_movR   DD_movR;	     //VK to ddcode


int snapValue;
int fovW;
int fovH;

int colorMode = 0;

int get_screen_width(void) {
    return GetSystemMetrics(SM_CXSCREEN);
}

int get_screen_height(void) {
    return GetSystemMetrics(SM_CYSCREEN);
}

struct point {
    double x;
    double y;
    point(double x, double y) : x(x), y(y) {}
};

//to hide console cursor (doesn't work for some reason? it did before.)
void ShowConsoleCursor(bool showFlag)
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO     cursorInfo;

    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = showFlag;
    SetConsoleCursorInfo(out, &cursorInfo);
}

//to avoid system()
void clear() {
    COORD topLeft = { 0, 0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(
        console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    FillConsoleOutputAttribute(
        console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
        screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    SetConsoleCursorPosition(console, topLeft);
}

inline bool is_color(int red, int green, int blue) {
    
    //original color purple
    if (colorMode == 0) {
        if (green >= 190) {
            return false;
        }

        if (green >= 140) {
            return abs(red - blue) <= 8 &&
                red - green >= 50 &&
                blue - green >= 50 &&
                red >= 105 &&
                blue >= 105;
        }

        return abs(red - blue) <= 13 &&
            red - green >= 60 &&
            blue - green >= 60 &&
            red >= 110 &&
            blue >= 100;
    }

    // yellow
    else {
        if (red < 160)
        {
            return false;
        }
        if (red > 161 && red < 255) {
            return green > 150 && green < 255 && blue > 0 && blue < 79;
        }
        return false;
    }
}

BOOL is_treigger(unsigned short red, unsigned short green, unsigned short blue) {

    if (green >= 170) {
        return FALSE;
    }

    if (green >= 120) {
        return abs(red - blue) <= 8 &&
            red - green >= 50 &&
            blue - green >= 50 &&
            red >= 105 &&
            blue >= 105;
    }

    return abs(red - blue) <= 13 &&
        red - green >= 60 &&
        blue - green >= 60 &&
        red >= 110 &&
        blue >= 100;

}

BOOL scan(HDC dc, int area_x, int area_y) {
    COLORREF col;
    for (int y = 2 * (-1); y < 2; ++y) {

        for (int x = 3 * (-1); x < 3; ++x) {

            col = GetPixel(dc, area_x + x, area_y + y);
            if (is_treigger((short)GetRValue(col), (short)GetGValue(col), (short)GetBValue(col))) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

//checking if settings.ini is present
bool FileExists(const std::string& Filename)
{
    return access(Filename.c_str(), 0) == 0;
}

BYTE* screenData = 0;
bool run_threads = true;
const int screen_width = get_screen_width(), screen_height = get_screen_height();

int aim_x = 0;
int aim_y = 0;

//bot with purple (original (again not default))
void bot() {
    int w = fovW, h = fovH;
    auto t_start = std::chrono::high_resolution_clock::now();
    auto t_end = std::chrono::high_resolution_clock::now();

    HDC hScreen = GetDC(NULL);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, w, h);
    screenData = (BYTE*)malloc(5 * screen_width * screen_height);
    HDC hDC = CreateCompatibleDC(hScreen);
    point middle_screen(screen_width / 2, screen_height / 2);

    BITMAPINFOHEADER bmi = { 0 };
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biWidth = w;
    bmi.biHeight = -h;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage = 0;

    while (run_threads) {
        Sleep(6);
        HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
        BOOL bRet = BitBlt(hDC, 0, 0, w, h, hScreen, middle_screen.x - (w/2), middle_screen.y - (h/2), SRCCOPY);
        SelectObject(hDC, old_obj);
        GetDIBits(hDC, hBitmap, 0, h, screenData, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);
        bool stop_loop = false;
        for (int j = 0; j < h; ++j) {
            for (int i = 0; i < w * 4; i += 4) {
                #define red screenData[i + (j*w*4) + 2]
                #define green screenData[i + (j*w*4) + 1]
                #define blue screenData[i + (j*w*4) + 0]

                if (is_color(red, green, blue)) {
                    aim_x = (i / 4) - (w/2);
                    aim_y = j - (h/2) + snapValue;
                    stop_loop = true;
                    break;
                }
            }
            if (stop_loop) {
                break;
            }
        }
        if (!stop_loop) {
            aim_x = 0;
            aim_y = 0;
        }
    }
}

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    HMODULE hDll = LoadLibraryW(L"MouseInput.dll");
    if (hDll == nullptr)
    {
        return -1;
    }
    DD_btn = (pDD_btn)GetProcAddress(hDll, "DD_btn");
    DD_movR = (pDD_movR)GetProcAddress(hDll, "DD_movR");

    if (!(DD_btn && DD_movR))
    {
        return -2;
    }
    int st = DD_btn(0);
    if (st != 1)
    {
        return st;
    }

    //for toggle keys
    bool trigger = false;
    bool toggleKey = false;
    bool toggleActive = true;
    bool aim = false;
    bool changeKey = false;
    bool lClick = false;
    bool mb4 = false;
    bool mb5 = false;
    bool hold = false;

    HWND deeznuts;
    deeznuts = FindWindowW(NULL, INAME);
    HDC nDC = GetDC(deeznuts);
    BOOL netflix_check;

    string color;
    int mode = 0;

    double sensitivity = 0.52;
    double smoothing = 0.5;
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    auto w_f = freopen("CON", "w", stdout);
    auto r_f = freopen("CON", "r", stdin);

    string loadSelect;

    INIReader reader("config.ini");

    if (FileExists("config.ini") == true) {
        cout << "[+] CONFIG FOUNDED [+]" << endl << endl;

        cout << "Do you want to load it (YES OR NO): ";
        cin >> loadSelect;

        if (loadSelect == "YES") {
            cout << "[+] CONFIG LOADED [+]";
            Sleep(2000);
            clear();

            cout << "'config.ini' loaded" << endl << endl;

            cout << "[+] CONFIGURATION [+]" << endl << endl;
            cout << "Sensitivity : " << reader.GetFloat("config", "sensitivity", 0.00) << endl
                << "Smoothing : " << reader.GetFloat("config", "smoothing", 0.00) << endl
                << "Mode : " << reader.GetInteger("config", "mode", 0) << endl
                << "Trigger : " << reader.Get("config", "trigger", "") << endl << endl
                << "[+] SETTINGS [+]" << endl << endl
                << "Aim Color : " << reader.Get("settings", "aim color", "") << endl
                << "Snapping Area : " << reader.Get("settings", "snapping area", "") << endl
                << "Toggle Key : " << reader.Get("settings", "toggle key", "") << endl
                << "Hold / Toggle : " << reader.Get("settings", "hold or toggle", "") << endl
                << "H FOV : " << reader.GetInteger("settings", "hFov", 0) << endl
                << "V FOV : " << reader.GetInteger("settings", "vFov", 0) << endl;

            cout << endl << endl << "[$] SUCCESSFUL LAUNCH YOUR GAME [$]" << endl << "[!] THIS PROMPT WILL CLOSE IN 5 SECONDS [!]";

            Sleep(5000);
            fclose(w_f);
            fclose(r_f);
            FreeConsole();

            sensitivity = reader.GetFloat("config", "sensitivity", 0.00);
            smoothing = reader.GetFloat("config", "smoothing", 0.00);
            mode = reader.GetInteger("config", "mode", 0);

            if (reader.Get("config", "trigger", "") == "TRUE") {
                trigger = true;
            }
            else if (reader.Get("config", "trigger", "") == "FALSE")
            {
                trigger = false;
            }

            color = reader.Get("settings", "aim color", "");
            if (reader.Get("settings", "snapping area", "") == "HEAD") {
                snapValue = 1;
            }
            else if (reader.Get("settings", "snapping area", "") == "NECK") {
                snapValue = 3;
            }
            else if (reader.Get("settings", "snapping area", "") == "CHEST") {
                snapValue = 8;
            }

            if (reader.Get("settings", "toggle key", "") == "LCLICK") {
                lClick = true;
            }
            else if (reader.Get("settings", "toggle key", "") == "MB4") {
                mb4 = true;
            }
            else if (reader.Get("settings", "toggle key", "") == "MB5") {
                mb5 = true;
            }

            if (reader.Get("settings", "hold or toggle", "") == "TOGGLE") {
                hold = false;
            }
            if (reader.Get("settings", "hold or toggle", "") == "HOLD") {
                hold = true;
            }

            fovW = reader.GetInteger("settings", "hFov", 0);
            fovH = reader.GetInteger("settings", "vFov", 0);

            if (color == "PURPLE") {
                // set color mode
                colorMode = 0;
            }

            else if (color == "YELLOW") {
                // set color mode
                colorMode = 1;
            }
            thread(bot).detach();

            auto t_start = std::chrono::high_resolution_clock::now();
            auto t_end = std::chrono::high_resolution_clock::now();
            auto left_start = std::chrono::high_resolution_clock::now();
            auto left_end = std::chrono::high_resolution_clock::now();
            double sensitivity_x = 1.0 / sensitivity / (screen_width / 1920.0) * 1.08;
            double sensitivity_y = 1.0 / sensitivity / (screen_height / 1080.0) * 1.08;
            bool left_down = false;

            while (run_threads) {
                t_end = std::chrono::high_resolution_clock::now();
                double elapsed_time_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();

                if (GetAsyncKeyState(VK_LBUTTON))
                {
                    left_down = true;
                }
                else
                {
                    left_down = false;
                }

                if (toggleActive == true) {

                    if (hold == true) {
                        if (lClick == true) {
                            if (GetAsyncKeyState(VK_LBUTTON))
                            {
                                aim = true;
                            }
                            else
                            {
                                aim = false;
                            }
                        }

                        if (mb4 == true) {
                            if (GetAsyncKeyState(VK_XBUTTON1))
                            {
                                aim = true;
                            }
                            else
                            {
                                aim = false;
                            }
                        }

                        if (mb5 == true) {
                            if (GetAsyncKeyState(VK_XBUTTON2))
                            {
                                aim = true;
                            }
                            else
                            {
                                aim = false;
                            }
                        }

                        if (GetAsyncKeyState(VK_F4) & 1)
                        {
                            trigger = !trigger;
                            if (trigger) {
                                Beep(500, 100);
                            }
                            else if (!trigger) {
                                Beep(300, 300);
                            }

                        }

                    }

                    if (hold == false) {
                        if (lClick == true) {
                            if (GetAsyncKeyState(VK_LBUTTON) & 1)
                            {
                                aim = !aim;

                            }
                        }

                        if (mb4 == true) {
                            if (GetAsyncKeyState(VK_XBUTTON1) & 1)
                            {
                                aim = !aim;
                                if (aim) {
                                    Beep(500, 100);
                                }
                                else if (!aim) {
                                    Beep(300, 300);
                                }
                            }
                        }

                        if (mb5 == true) {
                            if (GetAsyncKeyState(VK_XBUTTON2) & 1)
                            {
                                aim = !aim;
                                if (aim) {
                                    Beep(500, 100);
                                }
                                else if (!aim) {
                                    Beep(300, 300);
                                }
                            }
                        }

                        if (GetAsyncKeyState(VK_F4) & 1)
                        {
                            trigger = !trigger;
                            if (trigger) {
                                Beep(500, 100);
                            }
                            else if (!trigger) {
                                Beep(300, 300);
                            }

                        }
                    }



                    if (aim) {
                        CURSORINFO cursorInfo = { 0 };
                        cursorInfo.cbSize = sizeof(cursorInfo);
                        GetCursorInfo(&cursorInfo);
                        if (cursorInfo.flags != 1) {
                            if (((mode & 1) > 0) && (VK_LBUTTON)) {
                                left_down = true;
                                if (elapsed_time_ms > 7) {
                                    t_start = std::chrono::high_resolution_clock::now();
                                    left_start = std::chrono::high_resolution_clock::now();
                                    if (aim_x != 0 || aim_y != 0) {
                                        DD_movR(double(aim_x) * sensitivity_x, double(aim_y) * sensitivity_y);
                                    }
                                }
                            }
                            else if (((mode & 2) > 0)) {
                                if (elapsed_time_ms > 7) {
                                    t_start = std::chrono::high_resolution_clock::now();
                                    if (aim_x != 0 || aim_y != 0) {
                                        left_end = std::chrono::high_resolution_clock::now();
                                        double recoil_ms = std::chrono::duration<double, std::milli>(left_end - left_start).count();
                                        double extra = 38.0 * (screen_height / 1080.0) * (recoil_ms / 1000.0);
                                        if (!left_down) {
                                            extra = 0;
                                        }
                                        else if (extra > 38.0) {
                                            extra = 38.0;
                                        }
                                        double v_x = double(aim_x) * sensitivity_x * smoothing;
                                        double v_y = double(aim_y + extra) * sensitivity_y * smoothing;
                                        if (fabs(v_x) < 1.0) {
                                            v_x = v_x > 0 ? 1.05 : -1.05;
                                        }
                                        if (fabs(v_y) < 1.0) {
                                            v_y = v_y > 0 ? 1.05 : -1.05;
                                        }
                                        DD_movR(v_x, v_y);
                                    }
                                }
                            }
                        }
                    }

                    if (trigger)
                    {
                        netflix_check = scan(nDC, 960, 540);
                        if (netflix_check == TRUE) {
                            DD_btn(1);
                            Sleep(1);
                            DD_btn(2);
                            Sleep(1);
                            Sleep(100);
                        }
                    }
                }
                //end

            }

        }
    }


    // fix later
    if (loadSelect == "NO" || FileExists("config.ini") == false) {
        ShowCursor(false);

        //for the menu
        bool colorOptions = false;
        bool changeConfig = false;
        bool snappingArea = false;
        bool commit = false;

        //fov
        bool setFov = true;

        //start
        cout << "[+] $$$$$$ [+]" << endl;
        cout << "[-] ENTER YOUR CONFIG [-]" << endl << endl;
        cout << "[-] SENSITIVITY: ";
        cin >> sensitivity;
        cout << "[-] SMOOTHING: ";
        cin >> smoothing;
        int mode = 0;
        cout << "[-] MODE: ";
        cin >> mode;

        clear();
        cout << "CONFIGURATION SUCCESSFULLY LOADED. LOADING CHEAT OPTIONS.";
        Sleep(2000);

        Menu menu = Menu("[~] COLOR AIM MENU [~]", "[!] TO FINISH COMMIT YOUR CHANGES [!]");

        menu.AddOption("[!!] NOTHING IS DEFAULT SET EVERY SETTING HERE [!!]");
        menu.AddOption("[TAB] SET AIM COLOR");
        menu.AddOption("[F1] SET SNAPPING AREA");
        menu.AddOption("[F2] SET TOGGLE KEY");
        menu.AddOption("[F3] SET FOV");
        menu.AddOption("[ALT] CHANGE CONFIGURATION");
        menu.AddOption("[ESCAPE] COMMIT CHANGES");
        menu.AddOption(0, 99, false);
        menu.Print();
        while (true) {
            if (GetAsyncKeyState(VK_TAB)) {
                colorOptions = !colorOptions;

                menu.UpdateOption(1, 1);
                Sleep(1000);
                clear();
                cin.clear();
                fflush(stdin);

                cout << "[-] ENTER COLOR (CASE SENSITIVE - RED, YELLOW, PURPLE) ! RED NOT FUNCTIONAL ! : ";
                cin >> color;
                if (color == "RED") {
                    cout << "[!] RED ISN'T FUNCTIONAL, GO BACK AND CHANGE YOUR CHOICE [!]" << endl;

                    Sleep(1000);

                    menu.UpdateOption(1, 13);
                    menu.Print();
                }
                else if (color == "YELLOW") {
                    cout << "[+] COLOR IS SET TO YELLOW [+]" << endl;

                    Sleep(1000);

                    menu.UpdateOption(1, 4);
                    menu.Print();
                }
                else if (color == "PURPLE") {
                    cout << "[+] COLOR IS SET TO PURPLE [+]" << endl;

                    Sleep(1000);

                    menu.UpdateOption(1, 5);
                    menu.Print();
                }
                else {
                    cout << "[!] INVALID COLOR OPTION [!]";

                    menu.UpdateOption(1, 13);
                    Sleep(1000);

                    menu.Print();
                }
            }

            else if (GetAsyncKeyState(VK_F1)) {

                snappingArea = !snappingArea;

                menu.UpdateOption(2, 1);
                Sleep(1000);
                clear();
                cin.clear();
                fflush(stdin);

                string area;

                cout << "[-] ENTER AIMING AREA (CASE SENSITIVE - HEAD, NECK, CHEST): ";
                cin >> area;

                if (area == "HEAD") {
                    snapValue = 3;
                    cout << "[+] SNAPPING SET TO HEAD [+]";
                    Sleep(2000);

                    clear();

                    menu.UpdateOption(2, 7);
                    menu.Print();
                }
                else if (area == "NECK") {
                    snapValue = 4;
                    cout << "[+] SNAPPING SET TO NECK [+]";
                    Sleep(2000);

                    clear();

                    menu.UpdateOption(2, 8);
                    menu.Print();
                }
                else if (area == "CHEST") {
                    snapValue = 10;
                    cout << "[+] SNAPPING SET TO CHEST [+]";
                    Sleep(2000);

                    clear();

                    menu.UpdateOption(2, 9);
                    menu.Print();
                }
                else {
                    cout << "[!] INVALID CHOICE [!]";
                    menu.UpdateOption(2, 13);
                    Sleep(2000);

                    clear();

                    menu.Print();
                }
            }

            else if (GetAsyncKeyState(VK_F2)) {
                changeKey = !changeKey;

                int choice;

                menu.UpdateOption(3, 1);
                Sleep(1000);
                clear();
                cin.clear();
                fflush(stdin);

                cout << "SET THIS WITH AN INTEGER. ENTER 1, 2 OR 3" << endl << endl;

                cout << "[1] LEFT CLICK" << endl;
                cout << "[2] MB4" << endl;
                cout << "[3] MB5" << endl;

                cout << endl << "[-] ENTER YOUR CHOICE: ";
                cin >> choice;

                if (choice == 1) {
                    lClick = true;

                    clear();
                    cout << "[+] AIM KEY SET TO LEFT CLICK";

                    cin.clear();
                    fflush(stdin);

                    clear();

                    int changeMode;

                    cout << "[1] HOLD" << endl;
                    cout << "[2] TOGGLE" << endl;

                    cout << "[-] ENTER YOUR CHOICE: ";
                    cin >> changeMode;
                    if (changeMode == 1) {
                        hold = true;
                        clear();
                        cout << "[+] AIM KEY SET TO HOLD [+]";
                        Sleep(1000);
                    }
                    else if (changeMode == 2) {
                        clear();
                        cout << "[+] AIM KEY SET TO TOGGLE [+]";
                        Sleep(1000);
                    }


                    clear();
                    Sleep(500);
                    menu.UpdateOption(3, 10);
                    menu.Print();
                }

                else if (choice == 2) {
                    mb4 = true;

                    clear();
                    cout << "[+] AIM KEY SET TO MB4";

                    cin.clear();
                    fflush(stdin);

                    clear();

                    int changeMode;

                    cout << "[1] HOLD" << endl;
                    cout << "[2] TOGGLE" << endl;

                    cout << "[-] ENTER YOUR CHOICE: ";
                    cin >> changeMode;
                    if (changeMode == 1) {
                        hold = true;
                        clear();
                        cout << "[+] AIM KEY SET TO HOLD";
                    }
                    else if (changeMode == 2) {
                        clear();
                        cout << "[+] AIM KEY SET TO TOGGLE [+]";
                    }


                    clear();
                    Sleep(500);

                    menu.UpdateOption(3, 11);
                    clear();

                    menu.Print();
                }

                else if (choice == 3) {
                    mb5 = true;

                    clear();
                    cout << "[+] AIM KEY SET TO MB5";

                    cin.clear();
                    fflush(stdin);

                    clear();

                    int changeMode;

                    cout << "[1] HOLD" << endl;
                    cout << "[2] TOGGLE" << endl;

                    cout << "[-] ENTER YOUR CHOICE: ";
                    cin >> changeMode;
                    if (changeMode == 1) {
                        hold = true;
                        clear();
                        cout << "[+] AIM KEY SET TO HOLD";
                    }
                    else if (changeMode == 2) {
                        clear();
                        cout << "[+] AIM KEY SET TO TOGGLE [+]";
                    }


                    clear();
                    Sleep(500);

                    menu.UpdateOption(3, 12);
                    clear();

                    menu.Print();
                }

                else {
                    cout << "[!] INVALID CHOICE [!]";
                    menu.UpdateOption(3, 13);

                    clear();
                    menu.Print();
                }
            }

            else if (GetAsyncKeyState(VK_F3)) {
                setFov = !setFov;

                menu.UpdateOption(4, 1);
                Sleep(1000);
                clear();
                cin.clear();
                fflush(stdin);

                cout << "TO SET DEFAULT VALUES ENTER 100, 100 (RECOMMENDED VALUES 20-150)" << endl << endl;

                cout << "HORIZONTAL FOV: ";
                cin >> fovW;

                cout << "VERTICAL FOV: ";
                cin >> fovH;

                Sleep(1000);
                clear();

                cin.clear();
                fflush(stdin);
                menu.UpdateOption(4, 6);
                menu.Print();
            }

            else if (GetAsyncKeyState(VK_MENU)) {
                changeConfig = !changeConfig;

                menu.UpdateOption(5, 1);
                Sleep(1000);
                clear();

                cout << "[-] SENSITIVITY: ";
                cin >> sensitivity;
                cout << "[-] SMOOTHING: ";
                cin >> smoothing;
                cout << "[-] MODE: ";
                cin >> mode;
                Sleep(1000);
                clear();

                cout << "[+] CONFIGURATION SAVED [+]";
                Sleep(1000);
                cin.clear();
                fflush(stdin);
                menu.UpdateOption(5, 6);
                menu.Print();
            }

            else if (GetAsyncKeyState(VK_ESCAPE)) {
                commit = !commit;
                menu.UpdateOption(1);
                if (commit) {
                    menu.UpdateProgressBar(0, 0);
                    //the loading bar to indicate that everything worked and the program didn't stall
                    for (int i = 0; i < 99; i += 6) {
                        menu.UpdateProgressBar(0, (float)i);
                        Sleep(20);
                    }
                    menu.UpdateProgressBar(0, -1);
                    menu.UpdateOption(3, 6);

                    clear();
                    cout << endl << endl << endl;
                    cout << "[$] SUCCESS - LAUNCH YOUR GAME [$]" << endl;
                    cout << "[!] THIS PROMPT WILL CLOSE IN 5 SECONDS [!]";
                    Sleep(5000);
                    fclose(w_f);
                    fclose(r_f);
                    FreeConsole(); // had to free console here because running the aim (below) would stall the program

                    //end

                    if (color == "PURPLE") {
                        //aim start
                        colorMode = 0;
                    }

                    else if (color == "YELLOW") {
                        //aim start
                        colorMode = 1;
                    }
                    thread(bot).detach();

                    auto t_start = std::chrono::high_resolution_clock::now();
                    auto t_end = std::chrono::high_resolution_clock::now();
                    auto left_start = std::chrono::high_resolution_clock::now();
                    auto left_end = std::chrono::high_resolution_clock::now();
                    double sensitivity_x = 1.0 / sensitivity / (screen_width / 1920.0) * 1.08;
                    double sensitivity_y = 1.0 / sensitivity / (screen_height / 1080.0) * 1.08;
                    bool left_down = false;


                    while (run_threads) {
                        t_end = std::chrono::high_resolution_clock::now();
                        double elapsed_time_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();

                        if (GetAsyncKeyState(VK_LBUTTON)) {
                            left_down = true;
                        }
                        else
                        {
                            left_down = false;
                        }

                        if (toggleActive == true) {

                            if (hold == true) {
                                if (lClick == true) {
                                    if (GetAsyncKeyState(VK_LBUTTON))
                                    {
                                        aim = true;
                                    }
                                    else
                                    {
                                        aim = false;
                                    }
                                }

                                if (mb4 == true) {
                                    if (GetAsyncKeyState(VK_XBUTTON1))
                                    {
                                        aim = true;
                                    }
                                    else
                                    {
                                        aim = false;
                                    }
                                }

                                if (mb5 == true) {
                                    if (GetAsyncKeyState(VK_XBUTTON2))
                                    {
                                        aim = true;
                                    }
                                    else
                                    {
                                        aim = false;
                                    }
                                }
                            }

                            if (hold == false) {
                                if (lClick == true) {
                                    if (GetAsyncKeyState(VK_LBUTTON) & 1)
                                    {
                                        aim = !aim;

                                    }
                                }

                                if (mb4 == true) {
                                    if (GetAsyncKeyState(VK_XBUTTON1) & 1)
                                    {
                                        aim = !aim;
                                    }
                                }

                                if (mb5 == true) {
                                    if (GetAsyncKeyState(VK_XBUTTON2) & 1)
                                    {
                                        aim = !aim;
                                    }
                                }
                            }


                            if (aim) {
                                CURSORINFO cursorInfo = { 0 };
                                cursorInfo.cbSize = sizeof(cursorInfo);
                                GetCursorInfo(&cursorInfo);
                                if (cursorInfo.flags != 1) {
                                    if (((mode & 1) > 0) && (VK_LBUTTON)) {
                                        left_down = true;
                                        if (elapsed_time_ms > 7) {
                                            t_start = std::chrono::high_resolution_clock::now();
                                            left_start = std::chrono::high_resolution_clock::now();
                                            if (aim_x != 0 || aim_y != 0) {
                                                DD_movR(double(aim_x) * sensitivity_x, double(aim_y) * sensitivity_y);
                                            }
                                        }
                                    }
                                    else if (((mode & 2) > 0)) {
                                        if (elapsed_time_ms > 7) {
                                            t_start = std::chrono::high_resolution_clock::now();
                                            if (aim_x != 0 || aim_y != 0) {
                                                left_end = std::chrono::high_resolution_clock::now();
                                                double recoil_ms = std::chrono::duration<double, std::milli>(left_end - left_start).count();
                                                double extra = 38.0 * (screen_height / 1080.0) * (recoil_ms / 1000.0);
                                                if (!left_down) {
                                                    extra = 0;
                                                }
                                                else if (extra > 38.0) {
                                                    extra = 38.0;
                                                }
                                                double v_x = double(aim_x) * sensitivity_x * smoothing;
                                                double v_y = double(aim_y + extra) * sensitivity_y * smoothing;
                                                if (fabs(v_x) < 1.0) {
                                                    v_x = v_x > 0 ? 1.05 : -1.05;
                                                }
                                                if (fabs(v_y) < 1.0) {
                                                    v_y = v_y > 0 ? 1.05 : -1.05;
                                                }
                                                DD_movR(v_x, v_y);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        //end
                    }

                }
            }
        }
    }
    return 0;
}
