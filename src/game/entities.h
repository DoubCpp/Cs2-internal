#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <cmath>
#include <cstdint>
#include "pattern.h"
#include "../core/memory.h"

class Memory;

struct Vector3 {
    float x, y, z;
    
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }
    
    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }
    
    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }
    
    float Distance(const Vector3& other) const {
        Vector3 diff = *this - other;
        return sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
    }
};

struct Vector2 {
    float x, y;
    
    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}
};

struct Vector4 {
    float x, y, z, w;
    
    Vector4() : x(0), y(0), z(0), w(0) {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

template<typename T>
struct CHandle {
    uint32_t handle;
    
    bool IsValid() const {
        return handle != 0xFFFFFFFF;
    }
    
    int GetIndex() const {
        return handle & 0x7FF;
    }
};

class C_BaseEntity {
public:
    template<typename T>
    T GetValue(std::ptrdiff_t offset) {
        return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + offset);
    }
    
    template<typename T>
    void SetValue(std::ptrdiff_t offset, T value) {
        *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + offset) = value;
    }
    
    Vector3 GetOrigin() {
        // Note: This method will be implemented differently in ESP code
        // using the proper GameSceneNode approach
        return GetValue<Vector3>(pattern_offsets::entity_offsets::m_vecOrigin);
    }
    
    int GetHealth() {
        return GetValue<int>(pattern_offsets::entity_offsets::m_iHealth);
    }
    
    int GetTeamNum() {
        return GetValue<int>(pattern_offsets::entity_offsets::m_iTeamNum);
    }
    
    bool IsDormant() {
        // Use GetValue to avoid direct Memory dependency here
        // This will be handled by derived classes that properly include memory.h
        uintptr_t sceneNode = GetValue<uintptr_t>(pattern_offsets::entity_offsets::m_pGameSceneNode);
        if (!sceneNode) return true; // Consider invalid nodes as dormant
        
        // Read dormant flag from scene node
        bool result = false;
        __try {
            result = *reinterpret_cast<bool*>(sceneNode + pattern_offsets::entity_offsets::m_bDormant);
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            result = true; // Consider inaccessible as dormant
        }
        return result;
    }
};

class CCSPlayerController : public C_BaseEntity {
public:
    CHandle<void*> GetPlayerPawn() {
        return GetValue<CHandle<void*>>(pattern_offsets::entity_offsets::m_hPlayerPawn);
    }
    
    const char* GetPlayerName() {
        // CUtlString handling - read the string pointer from the CUtlString structure
        uintptr_t utilStringAddr = reinterpret_cast<uintptr_t>(this) + pattern_offsets::entity_offsets::m_sSanitizedPlayerName;
        uintptr_t stringPtr = *reinterpret_cast<uintptr_t*>(utilStringAddr);
        if (stringPtr && stringPtr != 0) {
            return reinterpret_cast<const char*>(stringPtr);
        }
        return "Unknown";
    }
};

class C_CSPlayerPawn : public C_BaseEntity {
};

// CS2 Bone data structure (based on working implementation)
struct BoneJointData {
    Vector3 position;
    char pad[20];  // Padding as seen in working code
};

// Bone matrix structure for skeleton rendering
struct BoneMatrix {
    float matrix[3][4];
    
    Vector3 GetPosition() const {
        return Vector3(matrix[0][3], matrix[1][3], matrix[2][3]);
    }
};

// Entity system structures
class CGameEntitySystem {
public:
    template<typename T>
    T GetEntityByIndex(int index) {
        // CS2 entity lookup using correct offsets
        if (index < 0 || index >= 2048) return nullptr;
        
        // First level: get list entry
        uintptr_t list_entry = *reinterpret_cast<uintptr_t*>(
            reinterpret_cast<uintptr_t>(this) + 8 * (index >> 9) + 16
        );
        
        if (!list_entry) return nullptr;
        
        // Second level: get entity from list
        uintptr_t entity = *reinterpret_cast<uintptr_t*>(
            list_entry + 120 * (index & 0x1FF)
        );
        
        return reinterpret_cast<T>(entity);
    }
};

// ============================================================================
// Entity Utility Functions - To avoid code duplication
// ============================================================================

class EntityUtils {
public:
    // Get world position of any entity efficiently
    static Vector3 GetWorldPosition(C_BaseEntity* entity) {
        if (!entity) return Vector3(0, 0, 0);
        
        uintptr_t entityAddr = reinterpret_cast<uintptr_t>(entity);
        uintptr_t sceneNodeAddr = Memory::Read<uintptr_t>(entityAddr + pattern_offsets::entity_offsets::m_pGameSceneNode);
        if (!sceneNodeAddr) return Vector3(0, 0, 0);
        
        return Memory::Read<Vector3>(sceneNodeAddr + pattern_offsets::entity_offsets::m_vecAbsOrigin);
    }
    
    // Get eye position (for aiming calculations)
    static Vector3 GetEyePosition(C_CSPlayerPawn* player) {
        if (!player) return Vector3(0, 0, 0);
        
        Vector3 origin = GetWorldPosition(player);
        Vector3 viewOffset = player->GetValue<Vector3>(pattern_offsets::entity_offsets::m_vecViewOffset);
        return origin + viewOffset;
    }
    
    // Calculate FOV between two angles efficiently
    static float CalculateFOV(const Vector3& angle1, const Vector3& angle2) {
        Vector3 delta = angle1 - angle2;
        return sqrt(delta.x * delta.x + delta.y * delta.y);
    }
    
    // Get current view angles - Using Pattern Offsets
    static Vector3 GetViewAngles() {
        uintptr_t viewAnglesAddr = VIEW_ANGLES;
        if (!viewAnglesAddr) return {};
        return Memory::Read<Vector3>(viewAnglesAddr);
    }
    
    // Set view angles with safety - Using Pattern Offsets
    static void SetViewAngles(const Vector3& angles) {
        uintptr_t viewAnglesAddr = VIEW_ANGLES;
        if (viewAnglesAddr) {
            Memory::Write<Vector3>(viewAnglesAddr, angles);
        }
    }
    
    // Distance conversion utilities - CS2 accurate conversion
    static float GameUnitsToMeters(float gameUnits) {
        // CS2 uses 1 meter = 39.3701 units (more precise)
        return gameUnits / 39.3701f;
    }
    
    static float MetersToGameUnits(float meters) {
        return meters * 39.3701f;
    }
    
    // Check if entity is dormant
    static bool IsDormant(C_BaseEntity* entity) {
        if (!entity) return true;
        
        uintptr_t entityAddr = reinterpret_cast<uintptr_t>(entity);
        uintptr_t sceneNodeAddr = Memory::Read<uintptr_t>(entityAddr + pattern_offsets::entity_offsets::m_pGameSceneNode);
        if (!sceneNodeAddr) return true;
        
        return Memory::Read<bool>(sceneNodeAddr + pattern_offsets::entity_offsets::m_bDormant);
    }
    
    // Check if entity is spotted
    static bool IsSpotted(C_BaseEntity* entity) {
        if (!entity) return false;
        
        uintptr_t entityAddr = reinterpret_cast<uintptr_t>(entity);
        uintptr_t spottedStateAddr = entityAddr + pattern_offsets::entity_offsets::m_entitySpottedState;
        return Memory::Read<bool>(spottedStateAddr + pattern_offsets::entity_offsets::m_bSpotted);
    }
};