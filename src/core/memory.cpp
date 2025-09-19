#include "memory.h"
#include <iostream>

uintptr_t Memory::clientBase = 0;

bool Memory::Initialize() {
    // Get client.dll base address
    clientBase = reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll"));
    if (!clientBase) {
        std::cout << "[Memory] Failed to get client.dll base address\n";
        return false;
    }
    
    std::cout << "[Memory] client.dll: 0x" << std::hex << clientBase << std::dec << "\n";
    std::cout << "[Memory] Memory system initialized successfully!\n";
    
    return true;
}

void Memory::Shutdown() {
    clientBase = 0;
    std::cout << "[Memory] Memory system shutdown complete\n";
}
