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
        MessageBoxW(NULL, L"الرجاء اختيار اللعبة أولاً!", L"تنبيه", MB_ICONWARNING);
        return;
    }
    SendMessageW(hListBox, LB_RESETCONTENT, 0, 0);
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
            std::wstring msg = L"لوتس عدو في: X=" + std::to_wstring((int)px) + L" Y=" + std::to_wstring((int)py);
            SendMessageW(hListBox, LB_ADDSTRING, 0, (LPARAM)msg.c_str());
            Beep(800, 200);
            foundCount++;
        }
    }
    if(foundCount == 0) SendMessageW(hListBox, LB_ADDSTRING, 0, (LPARAM)L"لم يتم العثور على لوتس حالياً");
}

void RefreshProcesses() {
    SendMessageW(hProcessList, LB_RESETCONTENT, 0, 0);
    pids.clear();
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W entry = {sizeof(entry)};
    if (Process32FirstW(snapshot, &entry)) {
        do {
            std::wstring name = entry.szExeFile;
            SendMessageW(hProcessList, LB_ADDSTRING, 0, (LPARAM)name.c_str());
            pids.push_back(entry.th32ProcessID);
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch(msg) {
        case WM_CREATE:
            CreateWindowW(L"STATIC", L"قائمة العمليات (اختر generals.exe):", WS_VISIBLE|WS_CHILD, 10, 5, 260, 20, hwnd, 0, 0, 0);
            hProcessList = CreateWindowW(L"LISTBOX", L"", WS_VISIBLE|WS_CHILD|WS_BORDER|WS_VSCROLL|LBS_NOTIFY, 10, 25, 260, 150, hwnd, 0, 0, 0);
            CreateWindowW(L"BUTTON", L"ربط اللعبة (Attach)", WS_VISIBLE|WS_CHILD, 10, 180, 260, 35, hwnd, (HMENU)2, 0, 0);
            
            CreateWindowW(L"STATIC", L"نتائج الرادار:", WS_VISIBLE|WS_CHILD, 10, 225, 260, 20, hwnd, 0, 0, 0);
            hListBox = CreateWindowW(L"LISTBOX", L"", WS_VISIBLE|WS_CHILD|WS_BORDER|WS_VSCROLL, 10, 245, 260, 100, hwnd, 0, 0, 0);
            CreateWindowW(L"BUTTON", L"فحص لوتس الآن", WS_VISIBLE|WS_CHILD, 10, 355, 260, 45, hwnd, (HMENU)1, 0, 0);
            RefreshProcesses();
            break;
        case WM_COMMAND:
            if(LOWORD(wp) == 1) ScanUnits();
            if(LOWORD(wp) == 2) {
                int sel = (int)SendMessageW(hProcessList, LB_GETCURSEL, 0, 0);
                if(sel != LB_ERR) {
                    if(hProcess) CloseHandle(hProcess);
                    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pids[sel]);
                    MessageBoxW(hwnd, L"متصل الآن بالعملية المختارة!", L"نجاح", MB_OK | MB_ICONINFORMATION);
                }
            }
            break;
        case WM_DESTROY: PostQuitMessage(0); break;
        default: return DefWindowProcW(hwnd, msg, wp, lp);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR, int s) {
    InitCommonControls();
    WNDCLASSW wc = {0}; wc.lpfnWndProc = WinProc; wc.hInstance = h; wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszClassName = L"MostafaFinal"; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);
    HWND main = CreateWindowW(L"MostafaFinal", L"Mostafa Generals Radar v2.0", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 200, 200, 300, 450, 0, 0, h, 0);
    ShowWindow(main, s);
    MSG m; while(GetMessageW(&m, 0, 0, 0)) { TranslateMessage(&m); DispatchMessageW(&m); }
    return 0;
}
