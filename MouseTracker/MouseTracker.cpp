#include <windows.h>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>

// Global variables
HWND hButton;
bool isTracking = false;
std::ofstream outFile;

// Function declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void StartTracking(HWND hwnd);
void StopTracking();
void TrackMouse(HWND hwnd);

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Create the main window class
    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"FullScreenApp";
    wc.hbrBackground = (HBRUSH)(COLOR_GRAYTEXT + 1);

    RegisterClass(&wc);

    // Create the main window
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles
        L"FullScreenApp",                     // Window class
        L"Mouse Tracker",                // Window text
        WS_OVERLAPPEDWINDOW,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,                           // Parent window
        NULL,                           // Menu
        hInstance,                      // Instance handle
        NULL                            // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    // Maximize window
    ShowWindow(hwnd, SW_SHOWMAXIMIZED);

    // Create a button in the middle of the screen
    RECT rect;
    GetClientRect(hwnd, &rect);
    int btnWidth = 200;
    int btnHeight = 50;
    hButton = CreateWindow(
        L"BUTTON",
        L"Start Tracking",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        (rect.right - btnWidth) / 2,
        (rect.bottom - btnHeight) / 2,
        btnWidth,
        btnHeight,
        hwnd,
        (HMENU)1,
        hInstance,
        NULL
    );

    // Show the window
    UpdateWindow(hwnd);

    // Run the message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {  // Button clicked
            if (!isTracking) {
                StartTracking(hwnd);
                SetWindowText(hButton, L"Stop Tracking");
            }
            else {
                StopTracking();
                SetWindowText(hButton, L"Start Tracking");
            }
        }
        break;
    case WM_DESTROY:
        StopTracking();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Start tracking mouse position
void StartTracking(HWND hwnd) {
    isTracking = true;
    outFile.open("mouse_tracking.csv");
    outFile << "Time, X, Y\n";  // CSV header

    std::thread tracker(TrackMouse, hwnd);
    tracker.detach();
}

// Stop tracking mouse position
void StopTracking() {
    if (isTracking) {
        isTracking = false;
        if (outFile.is_open()) {
            outFile.close();
        }
    }
}

// Track mouse position in a separate thread
void TrackMouse(HWND hwnd) {
    while (isTracking) {
        POINT point;
        if (GetCursorPos(&point)) {
            // Write mouse coordinates to file
            outFile << std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
                << ", " << point.x << ", " << point.y << "\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
