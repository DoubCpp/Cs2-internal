#include "hook.h"
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <assert.h>

// MinHook includes
#include "../minhook/MinHook.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// Internal types
typedef uint64_t uint150_t;
static RenderType g_renderType = RenderType::None;
static uint150_t* g_methodsTable = nullptr;

namespace Hook {
    
    HookStatus init(RenderType renderType) {
        if (g_renderType != RenderType::None) {
            return HookStatus::AlreadyInitializedError;
        }

        if (renderType != RenderType::D3D11 && renderType != RenderType::Auto) {
            return HookStatus::NotSupportedError;
        }

        // Detect render type if auto
        if (renderType == RenderType::Auto) {
            HMODULE libD3D11 = ::GetModuleHandleA("d3d11.dll");
            if (libD3D11 != NULL) {
                renderType = RenderType::D3D11;
            } else {
                return HookStatus::ModuleNotFoundError;
            }
        }

        // Create window for D3D11 initialization
        WNDCLASSEXA windowClass;
        windowClass.cbSize = sizeof(WNDCLASSEXA);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = DefWindowProc;
        windowClass.cbClsExtra = 0;
        windowClass.cbWndExtra = 0;
        windowClass.hInstance = GetModuleHandle(NULL);
        windowClass.hIcon = NULL;
        windowClass.hCursor = NULL;
        windowClass.hbrBackground = NULL;
        windowClass.lpszMenuName = NULL;
        windowClass.lpszClassName = "Hook";
        windowClass.hIconSm = NULL;

        ::RegisterClassExA(&windowClass);

        HWND window = ::CreateWindowA(windowClass.lpszClassName, "Hook DirectX Window",
                                    WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL,
                                    windowClass.hInstance, NULL);

        // Initialize D3D11
        if (renderType == RenderType::D3D11) {
            HMODULE libD3D11;
            if ((libD3D11 = ::GetModuleHandleA("d3d11.dll")) == NULL) {
                ::DestroyWindow(window);
                ::UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);
                return HookStatus::ModuleNotFoundError;
            }

            void* D3D11CreateDeviceAndSwapChain;
            if ((D3D11CreateDeviceAndSwapChain = ::GetProcAddress(libD3D11, "D3D11CreateDeviceAndSwapChain")) == NULL) {
                ::DestroyWindow(window);
                ::UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);
                return HookStatus::UnknownError;
            }

            D3D_FEATURE_LEVEL featureLevel;
            const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0 };

            DXGI_RATIONAL refreshRate;
            refreshRate.Numerator = 60;
            refreshRate.Denominator = 1;

            DXGI_MODE_DESC bufferDesc;
            bufferDesc.Width = 100;
            bufferDesc.Height = 100;
            bufferDesc.RefreshRate = refreshRate;
            bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

            DXGI_SAMPLE_DESC sampleDesc;
            sampleDesc.Count = 1;
            sampleDesc.Quality = 0;

            DXGI_SWAP_CHAIN_DESC swapChainDesc;
            swapChainDesc.BufferDesc = bufferDesc;
            swapChainDesc.SampleDesc = sampleDesc;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = 1;
            swapChainDesc.OutputWindow = window;
            swapChainDesc.Windowed = 1;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

            IDXGISwapChain* swapChain;
            ID3D11Device* device;
            ID3D11DeviceContext* context;

            if (((long(__stdcall*)(
                IDXGIAdapter*,
                D3D_DRIVER_TYPE,
                HMODULE,
                UINT,
                const D3D_FEATURE_LEVEL*,
                UINT,
                UINT,
                const DXGI_SWAP_CHAIN_DESC*,
                IDXGISwapChain**,
                ID3D11Device**,
                D3D_FEATURE_LEVEL*,
                ID3D11DeviceContext**))(D3D11CreateDeviceAndSwapChain))(
                    NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, 1, 
                    D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, 
                    &featureLevel, &context) < 0) {
                ::DestroyWindow(window);
                ::UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);
                return HookStatus::UnknownError;
            }

            g_methodsTable = (uint150_t*)::calloc(205, sizeof(uint150_t));
            ::memcpy(g_methodsTable, *(uint150_t**)swapChain, 18 * sizeof(uint150_t));
            ::memcpy(g_methodsTable + 18, *(uint150_t**)device, 43 * sizeof(uint150_t));
            ::memcpy(g_methodsTable + 18 + 43, *(uint150_t**)context, 144 * sizeof(uint150_t));

            MH_Initialize();

            swapChain->Release();
            swapChain = NULL;

            device->Release();
            device = NULL;

            context->Release();
            context = NULL;

            ::DestroyWindow(window);
            ::UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);

            g_renderType = RenderType::D3D11;

            return HookStatus::Success;
        }

        ::DestroyWindow(window);
        ::UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);

        return HookStatus::NotSupportedError;
    }

    void shutdown() {
        if (g_renderType != RenderType::None) {
            MH_DisableHook(MH_ALL_HOOKS);
            ::free(g_methodsTable);
            g_methodsTable = NULL;
            g_renderType = RenderType::None;
        }
    }

    HookStatus bind(uint16_t index, void** original, void* function) {
        assert(index >= 0 && original != NULL && function != NULL);

        if (g_renderType != RenderType::None) {
            void* target = (void*)g_methodsTable[index];
            if (MH_CreateHook(target, function, original) != MH_OK || MH_EnableHook(target) != MH_OK) {
                return HookStatus::UnknownError;
            }
            return HookStatus::Success;
        }

        return HookStatus::NotInitializedError;
    }

    void unbind(uint16_t index) {
        assert(index >= 0);

        if (g_renderType != RenderType::None) {
            MH_DisableHook((void*)g_methodsTable[index]);
        }
    }

    uint64_t* getMethodsTable() {
        return (uint64_t*)g_methodsTable;
    }
}