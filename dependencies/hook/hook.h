#pragma once

#include <stdint.h>
#include <d3d11.h>

// Hook status enum
enum class HookStatus {
    UnknownError = -1,
    NotSupportedError = -2,
    ModuleNotFoundError = -3,
    AlreadyInitializedError = -4,
    NotInitializedError = -5,
    Success = 0,
};

// Render type enum
enum class RenderType {
    None,
    D3D9,
    D3D10,
    D3D11,
    D3D12,
    OpenGL,
    Vulkan,
    
    Auto = D3D11  // Default to D3D11 for CS2
};

namespace Hook {
    // Initialize the hook system
    HookStatus init(RenderType renderType);
    
    // Shutdown the hook system
    void shutdown();
    
    // Bind a function hook
    HookStatus bind(uint16_t index, void** original, void* function);
    
    // Unbind a function hook
    void unbind(uint16_t index);
    
    // Get method table
    uint64_t* getMethodsTable();
}

// D3D11 Present function index
constexpr uint16_t IDXGISwapChain_Present = 8;
constexpr uint16_t IDXGISwapChain_ResizeBuffers = 13;