# Keylogger with Email Log Functionality

This project implements a Windows-based **keylogger** that captures keyboard input and saves it to a log file (`keylogs.txt`). It uses the Windows API and a low-level keyboard hook for monitoring keystrokes and the MAPI library to send the log file via email.

## Features

- **Keyboard Monitoring**: Records all key presses, including special keys like Enter, Tab, and Space.
- **Real-Time Timestamping**: Logs are segmented by minute to ensure readability.
- **Secure Exit**: Press `Ctrl + Shift + Q` to safely stop the keylogger.
- **Email Integration**: Automatically sends the log file to a specified email address when the application is closed.

## How It Works

1. **Key Logging**: Hooks into the keyboard using the `WH_KEYBOARD_LL` Windows hook to capture keystrokes.
2. **Log File Creation**: Writes keystrokes to `keylogs.txt` with timestamps for each minute.
3. **Email Sending**: Uses MAPI to send the log file as an email attachment to the specified recipient.

## Usage

1. Clone the repository and compile the code with a Windows-compatible C++ compiler.
2. Replace the `recipientEmail` and `senderEmail` variables with your desired email addresses.
3. Run the executable.
4. To stop the keylogger, press `Ctrl + Shift + Q`.
5. Upon exiting, the log file will be emailed to the specified recipient.

## Disclaimer

This project is for **educational purposes only**. Unauthorized use of keylogging software is illegal and unethical. Always obtain proper consent before using this software.
