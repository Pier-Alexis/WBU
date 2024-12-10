#include <windows.h>
#include <string>
#include <psapi.h>
#include <sstream>
#include <filesystem>

#define ID_BUTTON_CHECK 1
#define ID_BUTTON_CLEAN 2
#define ID_MENU_EXIT 3
#define ID_MENU_ABOUT 4

// Prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
std::string GetSystemHealth();
void CleanTemporaryFiles(HWND hwnd);
void ShowAboutDialog(HWND hwnd);

// Point d'entrée principal
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "WBUWindowClass";
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Windows Boost Utils (WBU)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// Gestionnaire de la fenêtre principale
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // Ajouter les boutons
        CreateWindow(
            "BUTTON", "Check System Health",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 50, 200, 50,
            hwnd, (HMENU)ID_BUTTON_CHECK, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
        );

        CreateWindow(
            "BUTTON", "Clean Temporary Files",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 120, 200, 50,
            hwnd, (HMENU)ID_BUTTON_CLEAN, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
        );

        // Ajouter un menu
        HMENU hMenu = CreateMenu();
        HMENU hFileMenu = CreateMenu();
        HMENU hHelpMenu = CreateMenu();

        AppendMenu(hFileMenu, MF_STRING, ID_MENU_EXIT, "Exit");
        AppendMenu(hHelpMenu, MF_STRING, ID_MENU_ABOUT, "About");

        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, "File");
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, "Help");

        SetMenu(hwnd, hMenu);
        break;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_BUTTON_CHECK: {
            std::string healthInfo = GetSystemHealth();
            MessageBox(hwnd, healthInfo.c_str(), "System Health", MB_OK | MB_ICONINFORMATION);
            break;
        }
        case ID_BUTTON_CLEAN: {
            CleanTemporaryFiles(hwnd);
            break;
        }
        case ID_MENU_EXIT: {
            PostQuitMessage(0);
            break;
        }
        case ID_MENU_ABOUT: {
            ShowAboutDialog(hwnd);
            break;
        }
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

// Fonction pour récupérer l'état du système
std::string GetSystemHealth() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);

    DWORDLONG totalPhysMem = memInfo.ullTotalPhys / (1024 * 1024); // En MB
    DWORDLONG physMemUsed = (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024 * 1024);

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD cpuCount = sysInfo.dwNumberOfProcessors;

    std::ostringstream healthInfo;
    healthInfo << "RAM Usage: " << physMemUsed << " MB / " << totalPhysMem << " MB\n";
    healthInfo << "CPU Cores: " << cpuCount;

    return healthInfo.str();
}

// Fonction pour nettoyer les fichiers temporaires
void CleanTemporaryFiles(HWND hwnd) {
    char tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);

    int deletedCount = 0;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(tempPath)) {
            try {
                std::filesystem::remove_all(entry.path());
                deletedCount++;
            } catch (...) {
                // Ignorer les erreurs
            }
        }
        std::ostringstream msg;
        msg << "Deleted " << deletedCount << " temporary files.";
        MessageBox(hwnd, msg.str().c_str(), "Clean Temporary Files", MB_OK | MB_ICONINFORMATION);
    } catch (...) {
        MessageBox(hwnd, "Error cleaning temporary files.", "Error", MB_OK | MB_ICONERROR);
    }
}

// Fonction pour afficher une boîte de dialogue À propos
void ShowAboutDialog(HWND hwnd) {
    MessageBox(hwnd,
        "Windows Boost Utils (WBU)\nVersion 1.0\nCreated by You",
        "About WBU", MB_OK | MB_ICONINFORMATION);
}
