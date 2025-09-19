#pragma once
#include <Windows.h>
#include "../game/pattern.h"

struct ViewMatrix {
    float matrix[4][4];
};

class CGameEntitySystem;
class C_CSPlayerPawn;
class CCSPlayerController;

class Memory {
private:
    static uintptr_t clientBase;
    
public:
    static bool Initialize();
    static void Shutdown();
    
    static uintptr_t GetClientBase() { return clientBase; }
    
    template<typename T>
    static T Read(uintptr_t address) {
        if (!address) return T{};
        __try {
            return *reinterpret_cast<T*>(address);
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            return T{};
        }
    }
    
    template<typename T>
    static void Write(uintptr_t address, const T& value) {
        if (!address) return;
        __try {
            *reinterpret_cast<T*>(address) = value;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
        }
    }
    
    static bool ReadRaw(uintptr_t address, void* buffer, size_t size) {
        if (!address || !buffer || !size) return false;
        __try {
            memcpy(buffer, reinterpret_cast<void*>(address), size);
            return true;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }
    
    // CS2 specific getters - Using Pattern Offsets
    static CGameEntitySystem* GetEntitySystem() {
        if (!clientBase) return nullptr;
        // dwGameEntitySystem est un pointeur vers un pointeur
        uintptr_t entitySystemAddr = GET_GLOBAL_OFFSET("dwGameEntitySystem");
        if (!entitySystemAddr) return nullptr;
        uintptr_t entitySystemPtr = Read<uintptr_t>(entitySystemAddr);
        return reinterpret_cast<CGameEntitySystem*>(entitySystemPtr);
    }
    
    static C_CSPlayerPawn* GetLocalPlayerPawn() {
        if (!clientBase) return nullptr;
        uintptr_t localPlayerAddr = LOCAL_PLAYER_PAWN;
        if (!localPlayerAddr) return nullptr;
        uintptr_t localPlayerPtr = Read<uintptr_t>(localPlayerAddr);
        return reinterpret_cast<C_CSPlayerPawn*>(localPlayerPtr);
    }
    
    static CCSPlayerController* GetLocalPlayerController() {
        if (!clientBase) return nullptr;
        uintptr_t localControllerAddr = LOCAL_PLAYER_CONTROLLER;
        if (!localControllerAddr) return nullptr;
        uintptr_t localControllerPtr = Read<uintptr_t>(localControllerAddr);
        return reinterpret_cast<CCSPlayerController*>(localControllerPtr);
    }
    
    static ViewMatrix GetViewMatrix() {
        if (!clientBase) return {};
        uintptr_t viewMatrixAddr = VIEW_MATRIX;
        if (!viewMatrixAddr) return {};
        return Read<ViewMatrix>(viewMatrixAddr);
    }
};


