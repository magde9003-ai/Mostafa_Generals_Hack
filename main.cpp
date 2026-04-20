#include <windows.h>
#include <string>

#define ADDR_UNIT_ARRAY_START   0x007E8A6C
#define ADDR_UNIT_COUNT         0x007E8A68
#define ADDR_LOCAL_TEAM         0x00A32C44
#define HASH_BLACK_LOTUS        0x00A8B2D8
#define UNIT_STRUCT_SIZE        0x5A8

HANDLE hProcess = NULL;
HWND hListBox;

void ScanUnits() {
    if(!hProcess) return;
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    DWORD count = 0, start = 0, myTeam = 0;
    ReadProcessMemory(hProcess, (LPCVOID)ADDR_UNIT_COUNT, &count, 4, NULL);
    ReadProcessMemory(hProcess, (LPCVOID)ADDR_UNIT_ARRAY_START, &start, 4, NULL);
    ReadProcessMemory(hProcess, (LPCVOID)ADDR_LOCAL_TEAM, &myTeam, 4, NULL);

    for(DWORD i = 0; i < count && i < 1500; i++) {
        DWORD addr = start + (i * UNIT_STRUCT_SIZE);
        DWORD hash = 0, team = 0;
        float hp = 0, px = 0, py = 0;
        ReadProcessMemory(hProcess, (LPCVOID)(addr + 0x1C), &hash, 4, NULL);
        ReadProcessMemory(hProcess, (LPCVOID)(addr + 0x44), &team, 4, NULL);
        ReadProcessMemory(hProcess, (LPCVOID)(addr + 0xC0), &hp, 4, NULL);

        if(team != myTeam && hp > 0 && hash == HASH_BLACK_LOTUS) {
            ReadProcessMemory(hProcess, (LPCVOID)(addr + 0x3C), &px, 4, NULL);
            ReadProcessMemory(hProcess, (LPCVOID)(addr + 0x40), &py, 4, NULL);
            std::string msg = "LOTUS Found at: " + std::to_string((int)px) + " , " + std::to_string((int)py);
            SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)msg.c_str());
            Beep(750, 300);
        }
    }
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if(msg == WM_CREATE) {
        CreateWindowA("BUTTON", "SCAN LOTUS", WS_VISIBLE|WS_CHILD, 10, 10, 130, 35, hwnd, (HMENU)1, 0, 0);
        hListBox = CreateWindowA("LISTBOX", "", WS_VISIBLE|WS_CHILD|WS_BORDER|WS_VSCROLL, 10, 55, 260, 150, hwnd, 0, 0, 0);
    } else if(msg == WM_COMMAND && LOWORD(wp) == 1) { ScanUnits(); }
    else if(msg == WM_DESTROY) { PostQuitMessage(0); }
    return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR, int s) {
    // محاولة إيجاد اللعبة بأكثر من طريقة
    HWND gWnd = FindWindowA("GameWindow", NULL);
    if(!gWnd) gWnd = FindWindowA(NULL, "Command & Conquer (TM) Generals Zero Hour");
    if(!gWnd) gWnd = FindWindowA(NULL, "Command & Conquer™ Generals Zero Hour");

    if(!gWnd) {
        MessageBoxA(0, "Game NOT Found! Open the Game First.", "Error", 0);
        return 0;
    }

    DWORD pid; GetWindowThreadProcessId(gWnd, &pid);
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    WNDCLASSA wc = {0}; wc.lpfnWndProc = WinProc; wc.hInstance = h; wc.lpszClassName = "Mostafa_Hack";
    RegisterClassA(&wc);
    HWND main = CreateWindowA("Mostafa_Hack", "Mostafa ZH Scanner", WS_OVERLAPPEDWINDOW, 200, 200, 300, 260, 0, 0, h, 0);
    ShowWindow(main, s);
    MSG m; while(GetMessage(&m, 0, 0, 0)) { TranslateMessage(&m); DispatchMessage(&m); }
    return 0;
}
