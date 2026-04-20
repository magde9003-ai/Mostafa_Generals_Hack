#include <windows.h>
#include <string>
#include <tlhelp32.h>
#include <vector>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

#define ADDR_UNIT_ARRAY_START   0x007E8A6C
#define ADDR_UNIT_COUNT         0x007E8A68
#define ADDR_LOCAL_TEAM         0x00A32C44
#define HASH_BLACK_LOTUS        0x00A8B2D8
#define UNIT_STRUCT_SIZE        0x5A8

HANDLE hProcess = NULL;
HWND hListBox, hProcessList;
std::vector<DWORD> pids;

void ScanUnits() {
    if(!hProcess) {
        MessageBoxA(NULL, "Select Game (generals.exe) First!", "Warning", MB_ICONWARNING);
        return;
    }
    SendMessageA(hListBox, LB_RESETCONTENT, 0, 0);
    DWORD count = 0, start = 0, myTeam = 0;
    ReadProcessMemory(hProcess, (LPCVOID)ADDR_UNIT_COUNT, &count, 4, NULL);
    ReadProcessMemory(hProcess, (LPCVOID)ADDR_UNIT_ARRAY_START, &start, 4, NULL);
    ReadProcessMemory(hProcess, (LPCVOID)ADDR_LOCAL_TEAM, &myTeam, 4, NULL);

    int foundCount = 0;
    for(DWORD i = 0; i < count && i < 2000; i++) {
        DWORD addr = start + (i * UNIT_STRUCT_SIZE);
        DWORD hash = 0, team = 0;
        float hp = 0, px = 0, py = 0;
        if(!ReadProcessMemory(hProcess, (LPCVOID)(addr + 0x1C), &hash, 4, NULL)) continue;
        ReadProcessMemory(hProcess, (LPCVOID)(addr + 0x44), &team, 4, NULL);
        ReadProcessMemory(hProcess, (LPCVOID)(addr + 0xC0), &hp, 4, NULL);

        if(hash == HASH_BLACK_LOTUS && team != myTeam && hp > 0) {
            ReadProcessMemory(hProcess, (LPCVOID)(addr + 0x3C), &px, 4, NULL);
            ReadProcessMemory(hProcess, (LPCVOID)(addr + 0x40), &py, 4, NULL);
            std::string msg = "LOTUS AT: X=" + std::to_string((int)px) + " Y=" + std::to_string((int)py);
            SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)msg.c_str());
            Beep(800, 200);
            foundCount++;
        }
    }
    if(foundCount == 0) SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)"Nothing Found.");
}

void RefreshProcesses() {
    SendMessageA(hProcessList, LB_RESETCONTENT, 0, 0);
    pids.clear();
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(entry);
    if (Process32First(snapshot, &entry)) {
        do {
            SendMessageA(hProcessList, LB_ADDSTRING, 0, (LPARAM)entry.szExeFile);
            pids.push_back(entry.th32ProcessID);
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if(msg == WM_CREATE) {
        hProcessList = CreateWindowA("LISTBOX", "", WS_VISIBLE|WS_CHILD|WS_BORDER|WS_VSCROLL, 10, 10, 260, 150, hwnd, 0, 0, 0);
        CreateWindowA("BUTTON", "1. ATTACH GAME", WS_VISIBLE|WS_CHILD, 10, 170, 260, 35, hwnd, (HMENU)2, 0, 0);
        hListBox = CreateWindowA("LISTBOX", "", WS_VISIBLE|WS_CHILD|WS_BORDER|WS_VSCROLL, 10, 215, 260, 100, hwnd, 0, 0, 0);
        CreateWindowA("BUTTON", "2. SCAN LOTUS", WS_VISIBLE|WS_CHILD, 10, 325, 260, 40, hwnd, (HMENU)1, 0, 0);
        RefreshProcesses();
    } else if(msg == WM_COMMAND) {
        if(LOWORD(wp) == 1) ScanUnits();
        if(LOWORD(wp) == 2) {
            int sel = (int)SendMessageA(hProcessList, LB_GETCURSEL, 0, 0);
            if(sel != LB_ERR) {
                if(hProcess) CloseHandle(hProcess);
                hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pids[sel]);
                MessageBoxA(hwnd, "Connected!", "Done", MB_OK);
            }
        }
    } else if(msg == WM_DESTROY) { PostQuitMessage(0); }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR, int s) {
    InitCommonControls();
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WinProc;
    wc.hInstance = h;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszClassName = "MostafaRadarV3";
    RegisterClassA(&wc);
    HWND main = CreateWindowA("MostafaRadarV3", "Mostafa Radar V3", WS_OVERLAPPEDWINDOW, 200, 200, 300, 420, 0, 0, h, 0);
    ShowWindow(main, s);
    MSG m;
    while(GetMessageA(&m, 0, 0, 0)) { TranslateMessage(&m); DispatchMessageA(&m); }
    return 0;
}
