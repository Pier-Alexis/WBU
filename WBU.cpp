#include <windows.h>
#include <string>
#include <psapi.h>
#include <sstream>
#include <filesystem>
#include <commdlg.h>

#define ID_BUTTON_CHECK 1
#define ID_BUTTON_CLEAN 2
#define ID_BUTTON_DISK 3
#define ID_MENU_EXIT 4
#define ID_MENU_ABOUT 5
#define ID_PROGRESS_BAR 6

// Prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
std::string GetSystemHealth();
void CleanTemporaryFiles(HWND hwnd, HWND progressBar);
std::string AnalyzeDiskSpace(const std::string& path);
void ShowAboutDialog(HWND hwnd);
std::string SelectDisk(HWND hwnd);

// Point d'entrée principal
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "WBUWindowClass";
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(RGB(240, 240, 240)); // Couleur de fond

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Windows Boost Utils (WBU)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
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
    static HWND progressBar;

    switch (uMsg) {
    case WM_CREATE: {
        // Ajouter les boutons avec des styles personnalisés
        HFONT hFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                 DEFAULT_PITCH | FF_SWISS, "Segoe UI");

        HWND btnCheck = CreateWindow(
            "BUTTON", "Check System Health",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 50, 250, 50,
            hwnd, (HMENU)ID_BUTTON_CHECK, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
        );
        SendMessage(btnCheck, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND btnClean = CreateWindow(
            "BUTTON", "Clean Temporary Files",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 120, 250, 50,
            hwnd, (HMENU)ID_BUTTON_CLEAN, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
        );
        SendMessage(btnClean, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND btnDisk = CreateWindow(
            "BUTTON", "Analyze Disk Space",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 190, 250, 50,
            hwnd, (HMENU)ID_BUTTON_DISK, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
        );
        SendMessage(btnDisk, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Barre de progression
        progressBar = CreateWindowEx(
            0, PROGRESS_CLASS, NULL,
            WS_CHILD | WS_VISIBLE,
            50, 300, 500, 30,
            hwnd, (HMENU)ID_PROGRESS_BAR, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
        );

        SendMessage(progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100)); // Définit la plage à 0-100
        SendMessage(progressBar, PBM_SETPOS, 0, 0); // Position initiale

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
            CleanTemporaryFiles(hwnd, progressBar);
            break;
        }
        case ID_BUTTON_DISK: {
            std::string diskPath = SelectDisk(hwnd);
            if (!diskPath.empty()) {
                std::string diskInfo = AnalyzeDiskSpace(diskPath);
                MessageBox(hwnd, diskInfo.c_str(), "Disk Space Analysis", MB_OK | MB_ICONINFORMATION);
            }
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
void CleanTemporaryFiles(HWND hwnd, HWND progressBar) {
    char tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);

    int totalFiles = 0, deletedFiles = 0;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(tempPath)) {
            totalFiles++;
        }

        int progress = 0;
        for (const auto& entry : std::filesystem::directory_iterator(tempPath)) {
            try {
                std::filesystem::remove_all(entry.path());
                deletedFiles++;
                progress = (deletedFiles * 100) / totalFiles;
                SendMessage(progressBar, PBM_SETPOS, progress, 0);
            } catch (...) {
                // Ignorer les erreurs
            }
        }

        std::ostringstream msg;
        msg << "Deleted " << deletedFiles << " temporary files.";
        MessageBox(hwnd, msg.str().c_str(), "Clean Temporary Files", MB_OK | MB_ICONINFORMATION);
    } catch (...) {
        MessageBox(hwnd, "Error cleaning temporary files.", "Error", MB_OK | MB_ICONERROR);
    }

    SendMessage(progressBar, PBM_SETPOS, 0, 0); // Réinitialise la barre
}

// Fonction pour analyser l'espace disque
std::string AnalyzeDiskSpace(const std::string& path) {
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;

    if (GetDiskFreeSpaceEx(path.c_str(), &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        std::ostringstream diskInfo;
        diskInfo << "Disk: " << path << "\n";
        diskInfo << "Total Disk Space: " << totalBytes.QuadPart / (1024 * 1024 * 1024) << " GB\n";
        diskInfo << "Free Disk Space: " << totalFreeBytes.QuadPart / (1024 * 1024 * 1024) << " GB\n";
        diskInfo << "Used Disk Space: "
                 << (totalBytes.QuadPart - totalFreeBytes.QuadPart) / (1024 * 1024 * 1024) << " GB";
        return diskInfo.str();
    }

    return "Error retrieving disk space information.";
}

// Fonction pour sélectionner un disque
std::string SelectDisk(HWND hwnd) {
    char path[MAX_PATH] = "";
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = "Select a Disk";
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

    if (pidl != 0) {
        SHGetPathFromIDList(pidl, path);
        return std::string(path);
    }
    return "";
}

// Fonction pour afficher une boîte de dialogue À propos
void ShowAboutDialog(HWND hwnd) {
    MessageBox(hwnd,
        "Windows Boost Utils (WBU)\nVersion 3.0\nCreated by You",
        "About WBU", MB_OK | MB_ICONINFORMATION);
}
