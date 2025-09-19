#include <Windows.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <TlHelp32.h>
#include <cstring>

class DLLInjector {
private:
    std::string m_processName;
    std::string m_dllPath;
    DWORD m_processId;
    HANDLE m_processHandle;

public:
    DLLInjector(const std::string& processName, const std::string& dllPath) 
        : m_processName(processName), m_dllPath(dllPath), m_processId(0), m_processHandle(nullptr) {}

    ~DLLInjector() {
        if (m_processHandle) {
            CloseHandle(m_processHandle);
        }
    }

    bool findProcess() {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            std::cout << "[ERROR] Failed to create process snapshot!\n";
            return false;
        }

        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(processEntry);

        if (Process32First(snapshot, &processEntry)) {
            do {
                // Convert process name to string using MultiByteToWideChar if needed
                std::string processNameFromEntry;
                
#ifdef UNICODE
                // Unicode version - convert to string
                int size = WideCharToMultiByte(CP_UTF8, 0, processEntry.szExeFile, -1, nullptr, 0, nullptr, nullptr);
                if (size > 0) {
                    processNameFromEntry.resize(size - 1);
                    WideCharToMultiByte(CP_UTF8, 0, processEntry.szExeFile, -1, &processNameFromEntry[0], size, nullptr, nullptr);
                }
#else
                // ASCII version - direct assignment
                processNameFromEntry = processEntry.szExeFile;
#endif
                
                if (m_processName == processNameFromEntry) {
                    m_processId = processEntry.th32ProcessID;
                    CloseHandle(snapshot);
                    std::cout << "[INFO] Found " << m_processName << " (PID: " << m_processId << ")\n";
                    return true;
                }
            } while (Process32Next(snapshot, &processEntry));
        }

        CloseHandle(snapshot);
        std::cout << "[ERROR] Process " << m_processName << " not found!\n";
        return false;
    }

    bool openProcess() {
        m_processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_processId);
        if (!m_processHandle) {
            std::cout << "[ERROR] Failed to open process! Error: " << GetLastError() << "\n";
            return false;
        }
        std::cout << "[INFO] Process opened successfully\n";
        return true;
    }

    bool injectDLL() {
        if (!std::filesystem::exists(m_dllPath)) {
            std::cout << "[ERROR] DLL file not found: " << m_dllPath << "\n";
            return false;
        }

        // Get full path
        std::string fullDllPath = std::filesystem::absolute(m_dllPath).string();
        std::cout << "[INFO] Injecting DLL: " << fullDllPath << "\n";

        // Allocate memory in target process
        void* allocatedMemory = VirtualAllocEx(m_processHandle, nullptr, fullDllPath.length() + 1, 
                                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        
        if (!allocatedMemory) {
            std::cout << "[ERROR] Failed to allocate memory in target process! Error: " << GetLastError() << "\n";
            return false;
        }

        // Write DLL path to allocated memory
        if (!WriteProcessMemory(m_processHandle, allocatedMemory, fullDllPath.c_str(), 
                               fullDllPath.length() + 1, nullptr)) {
            std::cout << "[ERROR] Failed to write DLL path to process memory! Error: " << GetLastError() << "\n";
            VirtualFreeEx(m_processHandle, allocatedMemory, 0, MEM_RELEASE);
            return false;
        }

        // Get LoadLibraryA address
        HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
        if (!kernel32) {
            std::cout << "[ERROR] Failed to get kernel32.dll handle!\n";
            VirtualFreeEx(m_processHandle, allocatedMemory, 0, MEM_RELEASE);
            return false;
        }

        void* loadLibraryAddr = GetProcAddress(kernel32, "LoadLibraryA");
        if (!loadLibraryAddr) {
            std::cout << "[ERROR] Failed to get LoadLibraryA address!\n";
            VirtualFreeEx(m_processHandle, allocatedMemory, 0, MEM_RELEASE);
            return false;
        }

        // Create remote thread to execute LoadLibraryA
        HANDLE remoteThread = CreateRemoteThread(m_processHandle, nullptr, 0, 
                                                 (LPTHREAD_START_ROUTINE)loadLibraryAddr, 
                                                 allocatedMemory, 0, nullptr);

        if (!remoteThread) {
            std::cout << "[ERROR] Failed to create remote thread! Error: " << GetLastError() << "\n";
            VirtualFreeEx(m_processHandle, allocatedMemory, 0, MEM_RELEASE);
            return false;
        }

        std::cout << "[INFO] Remote thread created, waiting for injection...\n";

        // Wait for thread completion
        WaitForSingleObject(remoteThread, INFINITE);

        // Get exit code to check if injection was successful
        DWORD exitCode;
        GetExitCodeThread(remoteThread, &exitCode);

        // Clean up
        CloseHandle(remoteThread);
        VirtualFreeEx(m_processHandle, allocatedMemory, 0, MEM_RELEASE);

        if (exitCode == 0) {
            std::cout << "[ERROR] DLL injection failed!\n";
            return false;
        }

        std::cout << "[SUCCESS] DLL injected successfully! Module handle: 0x" << std::hex << exitCode << std::dec << "\n";
        return true;
    }

    bool inject() {
        if (!findProcess()) return false;
        if (!openProcess()) return false;
        return injectDLL();
    }
};

void showBanner() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║                       CS2 Cheat Injector                     ║
║                      Doub-Cheat v1.0                        ║
╚══════════════════════════════════════════════════════════════╝
    )" << "\n\n";
}

void showUsage() {
    std::cout << "Usage: CS2-Injector.exe [options]\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help     Show this help message\n";
    std::cout << "  -d, --dll      Specify DLL path (default: Internal-Menu-Cs2.dll)\n";
    std::cout << "  -p, --process  Specify process name (default: cs2.exe)\n";
    std::cout << "\nExample:\n";
    std::cout << "  CS2-Injector.exe -d \"path/to/cheat.dll\" -p cs2.exe\n\n";
}

int main(int argc, char* argv[]) {
    showBanner();

    std::string processName = "cs2.exe";
    std::string dllPath = "Internal-Menu-Cs2.dll";

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            showUsage();
            return 0;
        }
        else if ((arg == "-d" || arg == "--dll") && i + 1 < argc) {
            dllPath = argv[++i];
        }
        else if ((arg == "-p" || arg == "--process") && i + 1 < argc) {
            processName = argv[++i];
        }
    }

    std::cout << "[INFO] Target Process: " << processName << "\n";
    std::cout << "[INFO] DLL Path: " << dllPath << "\n\n";

    // Check if running as administrator
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID adminGroup;

    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, 
                                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }

    if (!isAdmin) {
        std::cout << "[WARNING] Not running as administrator! Injection may fail.\n";
        std::cout << "[INFO] Please run as administrator for best results.\n\n";
    }

    // Wait for user confirmation
    std::cout << "[INFO] Make sure CS2 is running and press Enter to inject...\n";
    std::cin.get();

    // Create injector and attempt injection
    DLLInjector injector(processName, dllPath);
    
    if (injector.inject()) {
        std::cout << "\n[SUCCESS] Injection completed successfully!\n";
        std::cout << "[INFO] Check CS2 console for cheat initialization messages.\n";
        std::cout << "[INFO] Press INSERT to open the cheat menu in-game.\n\n";
        
        std::cout << "Press Enter to exit...\n";
        std::cin.get();
        return 0;
    } else {
        std::cout << "\n[FAILED] Injection failed!\n";
        std::cout << "[INFO] Please ensure:\n";
        std::cout << "  1. CS2 is running\n";
        std::cout << "  2. The DLL file exists\n";
        std::cout << "  3. Running as administrator\n";
        std::cout << "  4. Anti-virus is not blocking the injection\n\n";
        
        std::cout << "Press Enter to exit...\n";
        std::cin.get();
        return 1;
    }
}