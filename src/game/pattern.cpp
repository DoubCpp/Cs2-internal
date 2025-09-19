#include "pattern.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <psapi.h>
#include <cstring>
#include <memory>

// ============================================================================
// Static member definitions
// ============================================================================

// DynamicOffsets static members
std::unordered_map<std::string, uintptr_t> DynamicOffsets::offset_cache;
PatternScanner DynamicOffsets::scanner;
bool DynamicOffsets::initialized = false;

// PatternOffsets static members
std::unordered_map<std::string, uintptr_t> pattern_offsets::PatternOffsets::cached_offsets;
bool pattern_offsets::PatternOffsets::initialized = false;

// ============================================================================
// PatternScanner Implementation
// ============================================================================

PatternScanner::PatternScanner() : initialized(false) {}

PatternScanner::~PatternScanner() {
    // No cleanup needed for internal mode
}

bool PatternScanner::Initialize() {
    if (initialized) {
        return true;
    }

    std::cout << "[PatternScanner] Initializing in internal mode..." << std::endl;
    
    // Since we're injected, we can directly get module handles
    if (!GetModuleInfo("client.dll") || !GetModuleInfo("engine2.dll")) {
        std::cout << "[PatternScanner] Failed to get module information!" << std::endl;
        return false;
    }

    std::cout << "[PatternScanner] Successfully initialized!" << std::endl;
    initialized = true;
    return true;
}

bool PatternScanner::GetModuleInfo(const std::string& module_name) {
    // Since we're injected, we can directly get the module handle
    HMODULE hModule = GetModuleHandleA(module_name.c_str());
    if (!hModule) {
        std::cout << "[PatternScanner] Failed to get module handle for " << module_name << std::endl;
        return false;
    }

    // Get module information
    MODULEINFO moduleInfo;
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo))) {
        std::cout << "[PatternScanner] Failed to get module information for " << module_name << std::endl;
        return false;
    }

    ModuleInfo info;
    info.base = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
    info.size = moduleInfo.SizeOfImage;
    info.name = module_name;
    modules[module_name] = info;
    
    std::cout << "[PatternScanner] Found " << module_name 
              << " at 0x" << std::hex << info.base << std::dec 
              << " (size: 0x" << std::hex << info.size << std::dec << ")" << std::endl;
    
    return true;
}

std::vector<uint8_t> PatternScanner::ReadMemory(uintptr_t address, size_t size) {
    std::vector<uint8_t> buffer(size);
    
    try {
        // Since we're injected, we can directly access memory
        memcpy(buffer.data(), reinterpret_cast<void*>(address), size);
    } catch (...) {
        // If memory access fails, return empty buffer
        buffer.clear();
    }
    
    return buffer;
}

std::vector<int> PatternScanner::PatternToBytes(const std::string& pattern) {
    std::vector<int> bytes;
    std::istringstream iss(pattern);
    std::string token;
    
    while (iss >> token) {
        if (token == "?" || token == "??") {
            bytes.push_back(-1); // Wildcard
        } else {
            bytes.push_back(std::stoi(token, nullptr, 16));
        }
    }
    
    return bytes;
}

uintptr_t PatternScanner::ScanPattern(uintptr_t start, size_t size, const std::vector<int>& pattern) {
    std::vector<uint8_t> memory = ReadMemory(start, size);
    if (memory.empty() || pattern.empty()) {
        return 0;
    }

    for (size_t i = 0; i <= memory.size() - pattern.size(); ++i) {
        bool found = true;
        for (size_t j = 0; j < pattern.size(); ++j) {
            if (pattern[j] != -1 && pattern[j] != memory[i + j]) {
                found = false;
                break;
            }
        }
        if (found) {
            return start + i;
        }
    }
    
    return 0;
}

uintptr_t PatternScanner::FindPattern(const std::string& module_name, const std::string& pattern) {
    auto it = modules.find(module_name);
    if (it == modules.end()) {
        return 0;
    }

    return FindPattern(it->second.base, it->second.size, pattern);
}

uintptr_t PatternScanner::FindPattern(uintptr_t start, size_t size, const std::string& pattern) {
    std::vector<int> pattern_bytes = PatternToBytes(pattern);
    uintptr_t result = ScanPattern(start, size, pattern_bytes);
    
    if (result == 0) {
        return 0;
    }

    // For relative addressing, we need to read the offset and calculate the actual address
    // Most CS2 patterns use RIP-relative addressing (e.g., 48 8B 05 XX XX XX XX)
    if (pattern.find("48 8B 05") == 0 || // mov rax, qword ptr [rip+offset]
        pattern.find("48 89 05") == 0 || // mov qword ptr [rip+offset], rax
        pattern.find("48 8D 05") == 0 || // lea rax, qword ptr [rip+offset]
        pattern.find("48 8D 0D") == 0 || // lea rcx, qword ptr [rip+offset]
        pattern.find("48 89 35") == 0 || // mov qword ptr [rip+offset], rsi
        pattern.find("48 8B 3D") == 0 || // mov rdi, qword ptr [rip+offset]
        pattern.find("48 89 3D") == 0 || // mov qword ptr [rip+offset], rdi
        pattern.find("48 89 1D") == 0 || // mov qword ptr [rip+offset], rbx
        pattern.find("48 89 15") == 0 || // mov qword ptr [rip+offset], rdx
        pattern.find("48 8B 15") == 0 || // mov rdx, qword ptr [rip+offset]
        pattern.find("8B 05") == 0 ||    // mov eax, dword ptr [rip+offset]
        pattern.find("89 05") == 0) {    // mov dword ptr [rip+offset], eax
        
        // Read the 4-byte offset from the instruction
        std::vector<uint8_t> offset_bytes = ReadMemory(result + 3, 4);
        if (offset_bytes.size() != 4) {
            return 0;
        }
        
        int32_t offset = *reinterpret_cast<int32_t*>(offset_bytes.data());
        
        // Calculate RIP-relative address
        // RIP = instruction address + instruction length (7 bytes for most patterns)
        return result + 7 + offset;
    }
    
    return result;
}

// ============================================================================
// DynamicOffsets Implementation
// ============================================================================

bool DynamicOffsets::Initialize() {
    if (initialized) {
        return true;
    }

    if (!DynamicOffsets::scanner.Initialize()) {
        std::cout << "[DynamicOffsets] Failed to initialize pattern scanner!" << std::endl;
        return false;
    }

    InitializePatterns();
    DynamicOffsets::initialized = true;
    std::cout << "[DynamicOffsets] Successfully initialized with " << DynamicOffsets::offset_cache.size() << " offsets!" << std::endl;
    return true;
}

uintptr_t DynamicOffsets::GetOffset(const std::string& name) {
    if (!DynamicOffsets::initialized) {
        std::cout << "[DynamicOffsets] Not initialized! Call Initialize() first." << std::endl;
        return 0;
    }

    auto it = DynamicOffsets::offset_cache.find(name);
    if (it == DynamicOffsets::offset_cache.end()) {
        std::cout << "[DynamicOffsets] Offset '" << name << "' not found!" << std::endl;
        return 0;
    }

    return it->second;
}

void DynamicOffsets::ClearCache() {
    DynamicOffsets::offset_cache.clear();
    DynamicOffsets::initialized = false;
}

void DynamicOffsets::InitializePatterns() {
    // Clear existing cache
    DynamicOffsets::offset_cache.clear();

    std::cout << "[DynamicOffsets] Scanning for essential patterns only..." << std::endl;

    // Only scan patterns that are actually used in the project
    struct PatternData {
        const char* name;
        const char* module;
        const char* pattern;
    };

    PatternData patterns[] = {
        // Essential patterns for Aimbot/Triggerbot/ESP
        {"dwLocalPlayerController", "client.dll", "48 8B 05 ? ? ? ? 41 89 BE"},
        {"dwEntityList", "client.dll", "48 89 35 ? ? ? ? 48 85 F6"},
        {"dwGameEntitySystem", "client.dll", "48 8B 3D ? ? ? ? 48 89 3D"},
        {"dwViewMatrix", "client.dll", "48 8D 0D ? ? ? ? 48 C1 E0 06"},
        {"dwCSGOInput", "client.dll", "48 89 05 ? ? ? ? 0F 57 C0 0F 11 05"},  // Needed for dwViewAngles
        {"dwPrediction", "client.dll", "48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 40 53 56 41 54"},  // Needed for dwLocalPlayerPawn
    };

    int found_count = 0;
    int total_patterns = sizeof(patterns) / sizeof(PatternData);

    for (const auto& pattern_data : patterns) {
        uintptr_t address = DynamicOffsets::scanner.FindPattern(pattern_data.module, pattern_data.pattern);
        
        if (address != 0) {
            DynamicOffsets::offset_cache[pattern_data.name] = address;
            std::cout << "[DynamicOffsets] Found " << pattern_data.name 
                     << " at 0x" << std::hex << address << std::dec << std::endl;
            found_count++;
        } else {
            std::cout << "[DynamicOffsets] Failed to find pattern for " << pattern_data.name << std::endl;
        }
    }

    // Special handling for dwViewAngles and dwLocalPlayerPawn (requires additional scanning)
    DynamicOffsets::HandleSpecialPatterns();

    std::cout << "[DynamicOffsets] Pattern scanning complete: " 
              << found_count << "/" << total_patterns << " patterns found!" << std::endl;
}

void DynamicOffsets::HandleSpecialPatterns() {
    // dwViewAngles - found through dwCSGOInput callback
    auto csgo_input_it = DynamicOffsets::offset_cache.find("dwCSGOInput");
    if (csgo_input_it != DynamicOffsets::offset_cache.end()) {
        uintptr_t csgo_input = csgo_input_it->second;
        // Search for the ViewAngles pattern: F2 42 0F 10 84 28 ? ? ? ?
        uintptr_t view_angles = DynamicOffsets::scanner.FindPattern("client.dll", "F2 42 0F 10 84 28 ? ? ? ?");
        if (view_angles != 0) {
            // Extract the offset from the instruction
            std::vector<uint8_t> offset_bytes = DynamicOffsets::scanner.ReadMemory(view_angles + 6, 4);
            if (offset_bytes.size() == 4) {
                int32_t offset = *reinterpret_cast<int32_t*>(offset_bytes.data());
                DynamicOffsets::offset_cache["dwViewAngles"] = csgo_input + offset;
                std::cout << "[DynamicOffsets] Found dwViewAngles at 0x" 
                         << std::hex << (csgo_input + offset) << std::dec << std::endl;
            }
        }
    }

    // dwLocalPlayerPawn - found through dwPrediction callback  
    auto prediction_it = DynamicOffsets::offset_cache.find("dwPrediction");
    if (prediction_it != DynamicOffsets::offset_cache.end()) {
        uintptr_t prediction = prediction_it->second;
        // Search for the LocalPlayerPawn pattern: 4C 39 B6 ? ? ? ? 74 ? 44 88 BE
        uintptr_t local_pawn = DynamicOffsets::scanner.FindPattern("client.dll", "4C 39 B6 ? ? ? ? 74 ? 44 88 BE");
        if (local_pawn != 0) {
            // Extract the offset from the instruction (at position +3, 4 bytes)
            std::vector<uint8_t> offset_bytes = DynamicOffsets::scanner.ReadMemory(local_pawn + 3, 4);
            if (offset_bytes.size() == 4) {
                int32_t offset = *reinterpret_cast<int32_t*>(offset_bytes.data());
                DynamicOffsets::offset_cache["dwLocalPlayerPawn"] = prediction + offset;
                std::cout << "[DynamicOffsets] Found dwLocalPlayerPawn at 0x" 
                         << std::hex << (prediction + offset) << std::dec << std::endl;
            }
        } else {
            std::cout << "[DynamicOffsets] Failed to find dwLocalPlayerPawn pattern!" << std::endl;
        }
    } else {
        std::cout << "[DynamicOffsets] dwPrediction not found in cache!" << std::endl;
    }
}

// ============================================================================
// PatternOffsets Implementation - High-level interface
// ============================================================================

namespace pattern_offsets {

    bool PatternOffsets::Initialize() {
        if (initialized) {
            return true;
        }
        
        std::cout << "[PatternOffsets] Initializing pattern-based offset system..." << std::endl;
        
        // Clear any existing cache
        cached_offsets.clear();
        
        // Use the existing DynamicOffsets system which is already implemented
        if (!DynamicOffsets::Initialize()) {
            std::cout << "[PatternOffsets] Failed to initialize DynamicOffsets!" << std::endl;
            return false;
        }
        
        // Validate critical offsets are found
        if (!ValidateCriticalOffsets()) {
            std::cout << "[PatternOffsets] Critical offsets missing! Initialization failed." << std::endl;
            return false;
        }
        
        std::cout << "[PatternOffsets] Successfully initialized pattern system!" << std::endl;
        initialized = true;
        return true;
    }

    bool PatternOffsets::ValidateCriticalOffsets() {
        std::cout << "[PatternOffsets] Validating critical offsets..." << std::endl;
        
        // List of essential offsets required for basic cheat functionality
        std::vector<std::string> critical_offsets = {
            "dwLocalPlayerController",
            "dwLocalPlayerPawn", 
            "dwEntityList",
            "dwGameEntitySystem",
            "dwViewMatrix",
            "dwViewAngles"
        };
        
        std::vector<std::string> missing_offsets;
        
        for (const auto& offset_name : critical_offsets) {
            uintptr_t offset = DynamicOffsets::GetOffset(offset_name);
            if (offset == 0) {
                missing_offsets.push_back(offset_name);
                std::cout << "[PatternOffsets] CRITICAL: Missing offset '" << offset_name << "'" << std::endl;
            } else {
                std::cout << "[PatternOffsets] OK: " << offset_name << " = 0x" << std::hex << offset << std::dec << std::endl;
            }
        }
        
        if (!missing_offsets.empty()) {
            std::cout << "[PatternOffsets] VALIDATION FAILED: " << missing_offsets.size() 
                     << " critical offsets missing!" << std::endl;
            return false;
        }
        
        std::cout << "[PatternOffsets] All critical offsets validated successfully!" << std::endl;
        return true;
    }
    
    void PatternOffsets::PrintOffsetStatus() {
        std::cout << "\n[PatternOffsets] ========== OFFSET STATUS ==========" << std::endl;
        
        if (!initialized) {
            std::cout << "[PatternOffsets] System not initialized!" << std::endl;
            return;
        }
        
        const auto& all_offsets = DynamicOffsets::offset_cache;
        
        std::cout << "[PatternOffsets] Total offsets found: " << all_offsets.size() << std::endl;
        
        for (const auto& [name, offset] : all_offsets) {
            std::cout << "[PatternOffsets] " << name << " = 0x" << std::hex << offset << std::dec << std::endl;
        }
        
        std::cout << "[PatternOffsets] ========================================\n" << std::endl;
    }
    
    uintptr_t PatternOffsets::GetGlobalOffset(const std::string& name) {
        // Auto-initialize if not done yet
        if (!initialized) {
            static bool initializing = false;
            if (!initializing) {
                initializing = true;
                Initialize();
                initializing = false;
            } else {
                // Prevent recursion during initialization
                return 0;
            }
        }
        
        // Check cache first
        auto cache_it = cached_offsets.find(name);
        if (cache_it != cached_offsets.end()) {
            return cache_it->second;
        }
        
        // Use DynamicOffsets to get the offset
        uintptr_t offset = DynamicOffsets::GetOffset(name);
        
        if (offset != 0) {
            // Cache the result for future use
            cached_offsets[name] = offset;
            std::cout << "[PatternOffsets] Cached offset '" << name 
                     << "' at 0x" << std::hex << offset << std::dec << std::endl;
        } else {
            std::cout << "[PatternOffsets] Failed to find offset for '" << name << "'" << std::endl;
        }
        
        return offset;
    }
    
    void PatternOffsets::Refresh() {
        std::cout << "[PatternOffsets] Refreshing all offsets..." << std::endl;
        
        // Clear both our cache and the DynamicOffsets cache
        cached_offsets.clear();
        DynamicOffsets::ClearCache();
        
        // Re-initialize
        initialized = false;
        Initialize();
    }
    
} // namespace pattern_offsets