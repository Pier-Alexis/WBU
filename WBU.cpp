
#include <windows.h>
#include <string>
#include <psapi.h>
#include <sstream>
#include <vector>
#include <tlhelp32.h>
#include <commctrl.h>
#include <commdlg.h>

#define ID_BUTTON_GAME 10
#define ID_BUTTON_MONITOR 11
#define ID_LISTVIEW_PROCESSES 12

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OptimizeForGaming(HWND hwnd);
bool KillUnwantedProcesses(const std::vector<std::string>& processesToKill);
bool SetHighPriority(const std::string& processName);
void MonitorPerformance(HWND hwnd);
void AddProcessListView(HWND hwnd, HWND& listViewProcesses);
void PopulateProcessList(HWND listViewProcesses);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "WBUWindowClass";
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Windows Boost Utils (WBU)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
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

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND listViewProcesses;

    switch (uMsg) {
    case WM_CREATE: {
        HFONT hFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                 DEFAULT_PITCH | FF_SWISS, "Segoe UI");

        HWND btnGame = CreateWindow(
            "BUTTON", "Optimize for Gaming",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 50, 250, 50,
            hwnd, (HMENU)ID_BUTTON_GAME, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
        );
        SendMessage(btnGame, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND btnMonitor = CreateWindow(
            "BUTTON", "Monitor Performance",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 120, 250, 50,
            hwnd, (HMENU)ID_BUTTON_MONITOR, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
        );
        SendMessage(btnMonitor, WM_SETFONT, (WPARAM)hFont, TRUE);

        AddProcessListView(hwnd, listViewProcesses);
        break;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_BUTTON_GAME: {
            OptimizeForGaming(hwnd);
            break;
        }
        case ID_BUTTON_MONITOR: {
            MonitorPerformance(hwnd);
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

// Fonction principale d'optimisation pour le jeu
void OptimizeForGaming(HWND hwnd) {
    std::vector<std::string> unwantedProcesses = { "chrome.exe", "firefox.exe", "Teams.exe" };
    bool killedProcesses = KillUnwantedProcesses(unwantedProcesses);
    bool prioritySet = SetHighPriority("game.exe"); // Remplacez "game.exe" par le processus de votre jeu

    std::ostringstream message;
    if (killedProcesses) {
        message << "Unwanted processes stopped successfully.\n";
    } else {
        message << "Failed to stop some processes.\n";
    }

    if (prioritySet) {
        message << "Game priority set to High.\n";
    } else {
        message << "Failed to set game priority.\n";
    }

    MessageBox(hwnd, message.str().c_str(), "Game Optimization", MB_OK | MB_ICONINFORMATION);
}

// Fonction pour tuer les processus inutiles
bool KillUnwantedProcesses(const std::vector<std::string>& processesToKill) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &processEntry)) {
        do {
            for (const auto& processName : processesToKill) {
                if (std::string(processEntry.szExeFile) == processName) {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processEntry.th32ProcessID);
                    if (hProcess) {
                        TerminateProcess(hProcess, 0);
                        CloseHandle(hProcess);
                    }
                }
            }
        } while (Process32Next(hSnapshot, &processEntry));
    }

    CloseHandle(hSnapshot);
    return true;
}

// Fonction pour attribuer une priorité élevée à un processus
bool SetHighPriority(const std::string& processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &processEntry)) {
        do {
            if (std::string(processEntry.szExeFile) == processName) {
                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, processEntry.th32ProcessID);
                if (hProcess) {
                    bool result = SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS);
                    CloseHandle(hProcess);
                    CloseHandle(hSnapshot);
                    return result;
                }
            }
        } while (Process32Next(hSnapshot, &processEntry));
    }

    CloseHandle(hSnapshot);
    return false;
}

// Fonction pour surveiller les performances
void MonitorPerformance(HWND hwnd) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);

    DWORDLONG totalPhysMem = memInfo.ullTotalPhys / (1024 * 1024);
    DWORDLONG physMemUsed = (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024 * 1024);

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD cpuCount = sysInfo.dwNumberOfProcessors;

    std::ostringstream monitorInfo;
    monitorInfo << "CPU Cores: " << cpuCount << "\n";
    monitorInfo << "RAM Usage: " << physMemUsed << " MB / " << totalPhysMem << " MB";

    MessageBox(hwnd, monitorInfo.str().c_str(), "Performance Monitoring", MB_OK | MB_ICONINFORMATION);
}

// Ajoute un ListView pour afficher les processus
void AddProcessListView(HWND hwnd, HWND& listViewProcesses) {
    listViewProcesses = CreateWindowEx(
        WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
        WS_VISIBLE | WS_CHILD | LVS_REPORT,
        350, 50, 400, 400,
        hwnd, (HMENU)ID_LISTVIEW_PROCESSES, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
    );

    LVCOLUMN lvColumn;
    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;

    lvColumn.cx = 200;
    lvColumn.pszText = "Process Name";
    ListView_InsertColumn(listViewProcesses, 0, &lvColumn);

    lvColumn.cx = 100;
    lvColumn.pszText = "PID";
    ListView_InsertColumn(listViewProcesses, 1, &lvColumn);

    lvColumn.cx = 100;
    lvColumn.pszText = "Memory (MB)";
    ListView_InsertColumn(listViewProcesses, 2, &lvColumn);

    PopulateProcessList(listViewProcesses);
}

// Remplit le ListView avec les processus en cours
void PopulateProcessList(HWND listViewProcesses) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    int index = 0;
    if (Process32First(hSnapshot, &processEntry)) {
        do {
            LVITEM lvItem;
            lvItem.mask = LVIF_TEXT;
            lvItem.iItem = index++;
            lvItem.iSubItem = 0;
            lvItem.pszText = processEntry.szExeFile;
            ListView_InsertItem(listViewProcesses, &lvItem);

            lvItem.iSubItem = 1;
            std::ostringstream pid;
            pid << processEntry.th32ProcessID;
            lvItem.pszText = const_cast<char*>(pid.str().c_str());
            ListView_SetItem(listViewProcesses, &lvItem);

            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processEntry.th32ProcessID);
            if (hProcess) {
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                    lvItem.iSubItem = 2;
                    std::ostringstream memory;
                    memory << pmc.WorkingSetSize / (1024 * 1024);
                    lvItem.pszText = const_cast<char*>(memory.str().c_str());
                    ListView_SetItem(listViewProcesses, &lvItem);
                }
                CloseHandle(hProcess);
            }
        } while (Process32Next(hSnapshot, &processEntry));
    }

    CloseHandle(hSnapshot);
}
