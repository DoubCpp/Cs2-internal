#include <Windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <d3d11.h>
#include <dxgi.h>

// Includes
#include "gui/menu.h"
#include "core/memory.h"
#include "game/pattern.h"
#include "features/visuals/visuals.h"
#include "features/legit/legit.h"
#include "../dependencies/hook/hook.h"
#include "../dependencies/imgui/imgui.h"
#include "../dependencies/imgui/imgui_impl_dx11.h"
#include "../dependencies/imgui/imgui_impl_win32.h"

bool InitializeHooks();
bool SafeInitialize();
DWORD WINAPI MenuThread(LPVOID lpParam);

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Globals
typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WndProc_t)(HWND, UINT, WPARAM, LPARAM);

Present_t oPresent = nullptr;
WndProc_t oWndProc = nullptr;
HWND g_hwnd = nullptr;
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
bool g_imguiInitialized = false;

std::atomic<bool> g_running(false);
std::atomic<bool> g_unloadRequested(false);
HANDLE g_mainThread = nullptr;
auto g_initTime = std::chrono::steady_clock::now();

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (gui::open && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return 1L;
    
    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!g_imguiInitialized) {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pd3dDevice))) {
            g_pd3dDevice->GetImmediateContext(&g_pd3dDeviceContext);
            
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            g_hwnd = sd.OutputWindow;
            
            ID3D11Texture2D* pBackBuffer;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
            pBackBuffer->Release();
            
            oWndProc = (WndProc_t)SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
            
            ImGui::CreateContext();
            ImGui::StyleColorsDark();
            ImGui_ImplWin32_Init(g_hwnd);
            ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
            
            gui::SetupMenu(g_pd3dDevice, g_pd3dDeviceContext, pSwapChain);
            Visuals::UpdateScreenSize(sd.BufferDesc.Width, sd.BufferDesc.Height);
            
            g_imguiInitialized = true;
            std::cout << "[ImGui] Initialized successfully!" << std::endl;
        }
    }
    
    // Render ESP after 3 seconds delay
    if (g_imguiInitialized && g_running && Memory::GetClientBase()) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - g_initTime).count();
        
        if (elapsed >= 3) {
            __try {
                uintptr_t testRead = Memory::Read<uintptr_t>(Memory::GetClientBase() + 0x10);
                Visuals::Render();
                Legit::Update();
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
            }
        }
    }
    
    if (g_imguiInitialized) {
        if (GetAsyncKeyState(VK_INSERT) & 1)
            gui::open = !gui::open;
        
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        Visuals::Render();
        Legit::Render();
        
        if (gui::open) {
            gui::Render();
        }
        
        ImGui::Render();
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
    
    return oPresent(pSwapChain, SyncInterval, Flags);
}

LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* exceptionInfo) {
    std::cout << "[ERROR] Exception caught! Code: 0x" << std::hex << exceptionInfo->ExceptionRecord->ExceptionCode << std::dec << std::endl;
    std::cout << "[ERROR] Exception at address: 0x" << std::hex << exceptionInfo->ExceptionRecord->ExceptionAddress << std::dec << std::endl;
    
    g_running = false;
    g_unloadRequested = true;
    
    return EXCEPTION_EXECUTE_HANDLER;
}

bool SafeInitialize() {
    std::cout << "[Memory] Initializing..." << std::endl;
    
    HMODULE clientDll = GetModuleHandleA("client.dll");
    if (!clientDll) {
        std::cout << "[ERROR] client.dll not found" << std::endl;
        return false;
    }
    
    if (!Memory::Initialize()) {
        std::cout << "[ERROR] Failed to initialize memory system!" << std::endl;
        return false;
    }
    
    // Initialize patterns with retry
    std::cout << "[PatternOffsets] Initializing..." << std::endl;
    
    bool pattern_init_success = false;
    int retry_count = 0;
    const int max_retries = 3;
    
    while (!pattern_init_success && retry_count < max_retries) {
        if (retry_count > 0) {
            std::cout << "[PatternOffsets] Retry attempt " << retry_count << "/" << max_retries << std::endl;
            Sleep(1000); // Wait 1 second before retry
        }
        
        pattern_init_success = pattern_offsets::PatternOffsets::Initialize();
        
        if (!pattern_init_success) {
            retry_count++;
            if (retry_count < max_retries) {
                std::cout << "[PatternOffsets] Initialization failed, retrying..." << std::endl;
            }
        }
    }
    
    if (!pattern_init_success) {
        std::cout << "[ERROR] Pattern initialization failed after " << max_retries << " attempts!" << std::endl;
        return false;
    } else {
        std::cout << "[PatternOffsets] Initialized successfully!" << std::endl;
        pattern_offsets::PatternOffsets::PrintOffsetStatus();
    }
    
    Visuals::Initialize();
    Legit::Initialize();
    
    std::cout << "[Visuals] Ready" << std::endl;
    std::cout << "[Menu] Ready" << std::endl;
    
    return true;
}

bool InitializeHooks() {
    std::cout << "[Hook] Initializing..." << std::endl;
    
    if (Hook::init(RenderType::D3D11) == HookStatus::Success) {
        std::cout << "[Hook] Initialized successfully!" << std::endl;
        
        if (Hook::bind(IDXGISwapChain_Present, (void**)&oPresent, hkPresent) == HookStatus::Success) {
            std::cout << "[Hook] Present hooked successfully!" << std::endl;
            return true;
        } else {
            std::cout << "[ERROR] Failed to hook Present!" << std::endl;
        }
    } else {
        std::cout << "[ERROR] Failed to initialize Hook!" << std::endl;
    }
    
    return false;
}

DWORD WINAPI MenuThread(LPVOID lpParam) {
    // Initialize console
    if (!AllocConsole()) {
        return 1;
    }
    
    FILE* pCout = nullptr;
    freopen_s(&pCout, "CONOUT$", "w", stdout);
    if (!pCout) {
        FreeConsole();
        return 1;
    }
    
    SetUnhandledExceptionFilter(ExceptionHandler);
    std::cout << "[Internal-Menu-Cs2] Starting..." << std::endl;
    std::cout << "[Internal-Menu-Cs2] INTERNAL MENU v1.0 - CS2" << std::endl;
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Initialize hooks first
    if (!InitializeHooks()) {
        std::cout << "[ERROR] Failed to initialize hooks!" << std::endl;
        FreeConsole();
        return 1;
    }
    
    // Initialize menu system
    bool initSuccess = SafeInitialize();
    
    if (!initSuccess) {
        std::cout << "[Internal-Menu-Cs2] Initialization failed! Cleaning up..." << std::endl;
        Hook::shutdown();
        FreeConsole();
        return 1;
    }
    
    std::cout << "[Internal-Menu-Cs2] Ready! Press INSERT to toggle menu, END to unload" << std::endl;
    
    g_running = true;
    
    while (g_running && !g_unloadRequested) {
        if (GetAsyncKeyState(VK_END) & 0x8000) {
            g_unloadRequested = true;
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::cout << "[CS2-Cheat] Unloading..." << std::endl;
    g_running = false;
    
    Memory::Shutdown();
    
    if (g_imguiInitialized) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        
        if (g_mainRenderTargetView) { 
            g_mainRenderTargetView->Release(); 
            g_mainRenderTargetView = nullptr; 
        }
        if (g_pd3dDeviceContext) { 
            g_pd3dDeviceContext->Release(); 
            g_pd3dDeviceContext = nullptr; 
        }
        if (g_pd3dDevice) { 
            g_pd3dDevice->Release(); 
            g_pd3dDevice = nullptr; 
        }
    }
    
    if (oWndProc && g_hwnd) {
        SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);
    }
    
    Hook::shutdown();
    
    std::cout << "[Internal-Menu-Cs2] Unloaded successfully" << std::endl;
    
    if (pCout) fclose(pCout);
    FreeConsole();
    
    if (g_unloadRequested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(lpParam), 0);
    }
    
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    DisableThreadLibraryCalls(hModule);
    
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        g_mainThread = CreateThread(nullptr, 0, MenuThread, hModule, 0, nullptr);
        if (!g_mainThread) return FALSE;
        break;
        
    case DLL_PROCESS_DETACH:
        g_unloadRequested = true;
        g_running = false;
        if (g_mainThread) {
            WaitForSingleObject(g_mainThread, 2000);
            CloseHandle(g_mainThread);
        }
        break;
    }
    
    return TRUE;
}
