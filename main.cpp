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
        MessageBoxA(NULL, "Please Select Game (generals.exe) First!", "Warning", MB_ICONWARNING);
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

        if(team != myTeam && hp > 0 && hash == HASH_BLACK_LOTUS) {
            ReadProcessMemory(hProcess, (LPCVOID)(addr + 0x3C), &px, 4, NULL);
            ReadProcessMemory(hProcess, (LPCVOID)(addr + 0x40), &py, 4, NULL);
            std::string msg = "LOTUS FOUND! AT: X=" + std::to_string((int)px) + " Y=" + std::to_string((int)py);
            SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)msg.c_str());
            Beep(800, 200);
            foundCount++;
        }
    }
    if(foundCount == 0) SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)"No Black Lotus detected yet.");
}

void RefreshProcesses() {
    SendMessageA(hProcessList, LB_RESETCONTENT, 0, 0);
    pids.clear();
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 entry = {sizeof(entry)};
    if (Process32First(snapshot, &entry)) {
        do {
            SendMessageA(hProcessList, LB_ADDSTRING, 0, (LPARAM)entry.szExeFile);
            pids.push_back(entry.th32ProcessID);
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch(msg) {
        case WM_CREATE:
            CreateWindowA("STATIC", "1. Select 'generals.exe':", WS_VISIBLE|WS_CHILD, 10, 5, 260, 20, hwnd, 0, 0, 0);
            hProcessList = CreateWindowA("LISTBOX", "", WS_VISIBLE|WS_CHILD|WS_BORDER|WS_VSCROLL|LBS_NOTIFY, 10, 25, 260, 150, hwnd, 0, 0, 0);
            CreateWindowA("BUTTON", "STEP 1: ATTACH GAME", WS_VISIBLE|WS_CHILD, 10, 180, 260, 35, hwnd, (HMENU)2, 0, 0);
            
            CreateWindowA("STATIC", "2. Radar Status:", WS_VISIBLE|WS_CHILD, 10, 225, 260, 20, hwnd, 0, 0, 0);
            hListBox = CreateWindowA("LISTBOX", "", WS_VISIBLE|WS_CHILD|WS_BORDER|WS_VSCROLL, 10, 245, 260, 100, hwnd, 0, 0, 0);
            CreateWindowA("BUTTON", "STEP 2: SCAN FOR LOTUS", WS_VISIBLE|WS_CHILD, 10, 355, 260, 45, hwnd, (HMENU)1, 0, 0);
            RefreshProcesses();
            break;
        case WM_COMMAND:
            if(LOWORD(wp) == 1) ScanUnits();
            if(LOWORD(wp) == 2) {
                int sel = (int)SendMessageA(hProcessList, LB_GETCURSEL, 0, 0);
                if(sel != LB_ERR) {
                    if(hProcess) CloseHandle(hProcess);
                    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pids[sel]);
                    MessageBoxA(hwnd, "Connected Successfully!", "Success", MB_OK);
                }
            }
            break;
        case WM_DESTROY: PostQuitMessage(0); break;
        default: return DefWindowProcA(hwnd, msg, wp, lp);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR, int s) {
    InitCommonControls();
    WNDCLASSA wc = {0}; wc.lpfnWndProc = WinProc; wc.hInstance = h; wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszClassName = "MostafaFinalEN"; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc);
    HWND main = CreateWindowA("MostafaFinalEN", "Mostafa Generals Scanner v2.1", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 200, 200, 300, 450, 0, 0, h, 0);
    ShowWindow(main, s);
    MSG m; while(GetMessage
