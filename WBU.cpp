#include <windows.h>
#include <string>
#include <psapi.h>
#include <sstream>

#define ID_BUTTON_CHECK 1

// Prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
std::string GetSystemHealth();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register the window class
    const char CLASS_NAME[] = "WBUWindowClass";
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Windows Boost Utils (WBU)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 200,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Run the message loop
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // Add a button
        CreateWindow(
            "BUTTON", "Check System Health",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            100, 50, 200, 50,
            hwnd, (HMENU)ID_BUTTON_CHECK, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
        );
        break;
    }

    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_BUTTON_CHECK) {
            // Display system health information
            std::string healthInfo = GetSystemHealth();
            MessageBox(hwnd, healthInfo.c_str(), "System Health", MB_OK | MB_ICONINFORMATION);
        }
        break;
    }

    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

std::string GetSystemHealth() {
    // Get memory status
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);

    DWORDLONG totalPhysMem = memInfo.ullTotalPhys / (1024 * 1024); // In MB
    DWORDLONG physMemUsed = (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024 * 1024);

    // Get CPU count
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD cpuCount = sysInfo.dwNumberOfProcessors;

    // Format information
    std::ostringstream healthInfo;
    healthInfo << "RAM Usage: " << physMemUsed << " MB / " << totalPhysMem << " MB\n";
    healthInfo << "CPU Cores: " << cpuCount;

    return healthInfo.str();
}
