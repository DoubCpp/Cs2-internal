#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <tlhelp32.h>
#include <psapi.h>
#include <cstddef>

class PatternScanner;
class DynamicOffsets;

class PatternScanner {
private:
    struct ModuleInfo {
        uintptr_t base;
        size_t size;
        std::string name;
    };

    std::unordered_map<std::string, ModuleInfo> modules;
    bool initialized;

public:
    PatternScanner();
    ~PatternScanner();

    bool Initialize();
    uintptr_t FindPattern(const std::string& module_name, const std::string& pattern);
    uintptr_t FindPattern(uintptr_t start, size_t size, const std::string& pattern);
    
    std::vector<uint8_t> ReadMemory(uintptr_t address, size_t size);
    
    std::vector<int> PatternToBytes(const std::string& pattern);
    
    uintptr_t ScanPattern(uintptr_t start, size_t size, const std::vector<int>& pattern);

private:
    bool GetModuleInfo(const std::string& module_name);
};

class DynamicOffsets {
private:
    static PatternScanner scanner;
    static bool initialized;

public:
    static std::unordered_map<std::string, uintptr_t> offset_cache;
    static bool Initialize();
    static uintptr_t GetOffset(const std::string& name);
    static void ClearCache();
    
    struct Patterns {
        // Essential patterns only - used in the project
        static constexpr const char* dwLocalPlayerController = "48 8B 05 ? ? ? ? 41 89 BE";
        static constexpr const char* dwLocalPlayerPawn = "4C 39 B6 ? ? ? ? 74 ? 44 88 BE";
        static constexpr const char* dwEntityList = "48 89 35 ? ? ? ? 48 85 F6";
        static constexpr const char* dwViewMatrix = "48 8D 0D ? ? ? ? 48 C1 E0 06";
        static constexpr const char* dwViewAngles = "F2 42 0F 10 84 28 ? ? ? ?";
        static constexpr const char* dwCSGOInput = "48 89 05 ? ? ? ? 0F 57 C0 0F 11 05";
        static constexpr const char* dwPrediction = "48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 40 53 56 41 54";
    };

private:
    static void InitializePatterns();
    static void HandleSpecialPatterns();
};

// ============================================================================
// Pattern-Based Offset Manager - High-level interface
// ============================================================================

namespace pattern_offsets {
    
    class PatternOffsets {
    private:
        static std::unordered_map<std::string, uintptr_t> cached_offsets;
        static bool initialized;
        
    public:
        // Initialize pattern scanning system
        static bool Initialize();
        
        // Get a global offset by name (e.g., "dwLocalPlayerController")
        static uintptr_t GetGlobalOffset(const std::string& name);
        
        // Clear cache and force re-scan
        static void Refresh();
        
        // Check if system is initialized
        static bool IsInitialized() { return initialized; }
        
        // Validate that all critical offsets are found
        static bool ValidateCriticalOffsets();
        
        // Print status of all found offsets (debugging)
        static void PrintOffsetStatus();
        
        // Get all cached offsets for debugging
        static const std::unordered_map<std::string, uintptr_t>& GetAllOffsets() { return cached_offsets; }
    };
    
    // ========================================================================
    // Entity Field Offsets - These remain static as they're structure offsets
    // Note: Ces offsets sont relatifs aux structures et changent rarement
    // ========================================================================
    
    namespace entity_offsets {
        // C_BaseEntity - Core entity properties
        constexpr std::ptrdiff_t m_vecOrigin = 0x88;
        constexpr std::ptrdiff_t m_pGameSceneNode = 0x330;
        constexpr std::ptrdiff_t m_pRenderComponent = 0x338;
        constexpr std::ptrdiff_t m_pCollision = 0x340;
        constexpr std::ptrdiff_t m_iMaxHealth = 0x348;
        constexpr std::ptrdiff_t m_iHealth = 0x34C;
        constexpr std::ptrdiff_t m_lifeState = 0x350;
        constexpr std::ptrdiff_t m_iTeamNum = 0x3EB;
        constexpr std::ptrdiff_t m_spawnflags = 0x3EC;
        constexpr std::ptrdiff_t m_fFlags = 0x3F8;
        constexpr std::ptrdiff_t m_hOwnerEntity = 0x438;
        
        // C_CSPlayerPawnBase/C_CSPlayerPawn
        constexpr std::ptrdiff_t m_vecViewOffset = 0xD98;
        constexpr std::ptrdiff_t m_entitySpottedState = 0x1400;
        constexpr std::ptrdiff_t m_bSpotted = 0x8;           // offset within EntitySpottedState_t
        constexpr std::ptrdiff_t m_bSpottedByMask = 0xC;     // offset within EntitySpottedState_t
        constexpr std::ptrdiff_t m_iShotsFired = 0x273C;
        constexpr std::ptrdiff_t m_aimPunchAngle = 0x16F4;
        constexpr std::ptrdiff_t m_aimPunchCache = 0x1528;
        constexpr std::ptrdiff_t m_pClippingWeapon = 0x3DF0;
        constexpr std::ptrdiff_t m_flFlashDuration = 0x1620;
        constexpr std::ptrdiff_t m_flFlashMaxAlpha = 0x161C;
        
        // C_CSWeaponBase
        constexpr std::ptrdiff_t m_iItemDefinitionIndex = 0x1BA;
        
        // CGameSceneNode
        constexpr std::ptrdiff_t m_nodeToWorld = 0x10;
        constexpr std::ptrdiff_t m_pOwner = 0x30;
        constexpr std::ptrdiff_t m_hParent = 0x78;
        constexpr std::ptrdiff_t m_vecAbsOrigin = 0xD0;
        constexpr std::ptrdiff_t m_angAbsRotation = 0xDC;
        constexpr std::ptrdiff_t m_flAbsScale = 0xE8;
        constexpr std::ptrdiff_t m_bDormant = 0x10B;
        
        // CSkeletonInstance
        constexpr std::ptrdiff_t m_modelState = 0x190;
        
        // Rendering
        constexpr std::ptrdiff_t m_Glow = 0xAD0;
        constexpr std::ptrdiff_t m_clrRender = 0xAE8;
        
        // CCSPlayerController
        constexpr std::ptrdiff_t m_hPlayerPawn = 0x8FC;
        constexpr std::ptrdiff_t m_hObserverPawn = 0x900;
        constexpr std::ptrdiff_t m_sSanitizedPlayerName = 0x850;
        constexpr std::ptrdiff_t m_iPing = 0x740;
        constexpr std::ptrdiff_t m_bPawnIsAlive = 0x904;
        constexpr std::ptrdiff_t m_iPawnHealth = 0x908;
        constexpr std::ptrdiff_t m_bPawnHasDefuser = 0x910;
        constexpr std::ptrdiff_t m_bPawnHasHelmet = 0x911;
        
        // C_PlantedC4
        constexpr std::ptrdiff_t m_nBombSite = 0xFB0;
        constexpr std::ptrdiff_t m_bBombTicking = 0xF98;
        constexpr std::ptrdiff_t m_bBombDefused = 0xFB4;
        constexpr std::ptrdiff_t m_flC4Blow = 0xF9C;
        constexpr std::ptrdiff_t m_flTimerLength = 0xFA0;
        constexpr std::ptrdiff_t m_flDefuseLength = 0xFB8;
        constexpr std::ptrdiff_t m_flDefuseCountDown = 0xFBC;
        constexpr std::ptrdiff_t m_bBeingDefused = 0xFC0;
        constexpr std::ptrdiff_t m_hBombDefuser = 0xFC4;
        
        // Model and bone matrix
        constexpr std::ptrdiff_t m_CBodyComponent = 0x328;
        constexpr std::ptrdiff_t dwBoneMatrix = 0x80;
        
        // Sensitivity (nested offset)
        constexpr std::ptrdiff_t dwSensitivity_sensitivity = 0x48;
    }
    
    // ========================================================================
    // Bone Indices - Ces valeurs sont constantes pour le modèle de joueur CS2
    // ========================================================================
    
    namespace bone_indices {
        // Core spine
        constexpr int BONE_PELVIS = 0;
        constexpr int BONE_SPINE_0 = 1;
        constexpr int BONE_SPINE_1 = 2;
        constexpr int BONE_SPINE_2 = 3;
        constexpr int BONE_CHEST = 4;
        constexpr int BONE_NECK = 5;
        constexpr int BONE_HEAD = 6;
        constexpr int BONE_HEAD_TOP = 7;
        
        // Left arm chain
        constexpr int BONE_LEFT_CLAVICLE = 8;
        constexpr int BONE_LEFT_SHOULDER = 9;
        constexpr int BONE_LEFT_UPPER_ARM = 10;
        constexpr int BONE_LEFT_ELBOW = 11;
        constexpr int BONE_LEFT_FOREARM = 12;
        constexpr int BONE_LEFT_WRIST = 13;
        constexpr int BONE_LEFT_HAND = 14;
        constexpr int BONE_LEFT_FINGER_0 = 15;
        constexpr int BONE_LEFT_FINGER_1 = 16;
        constexpr int BONE_LEFT_FINGER_2 = 17;
        
        // Right arm chain
        constexpr int BONE_RIGHT_CLAVICLE = 18;
        constexpr int BONE_RIGHT_SHOULDER = 19;
        constexpr int BONE_RIGHT_UPPER_ARM = 20;
        constexpr int BONE_RIGHT_ELBOW = 21;
        constexpr int BONE_RIGHT_FOREARM = 22;
        constexpr int BONE_RIGHT_WRIST = 23;
        constexpr int BONE_RIGHT_HAND = 24;
        constexpr int BONE_RIGHT_FINGER_0 = 25;
        constexpr int BONE_RIGHT_FINGER_1 = 26;
        constexpr int BONE_RIGHT_FINGER_2 = 27;
        
        // Left leg chain
        constexpr int BONE_LEFT_HIP = 28;
        constexpr int BONE_LEFT_THIGH = 29;
        constexpr int BONE_LEFT_KNEE = 30;
        constexpr int BONE_LEFT_CALF = 31;
        constexpr int BONE_LEFT_ANKLE = 32;
        constexpr int BONE_LEFT_FOOT = 33;
        constexpr int BONE_LEFT_TOE = 34;
        
        // Right leg chain
        constexpr int BONE_RIGHT_HIP = 35;
        constexpr int BONE_RIGHT_THIGH = 36;
        constexpr int BONE_RIGHT_KNEE = 37;
        constexpr int BONE_RIGHT_CALF = 38;
        constexpr int BONE_RIGHT_ANKLE = 39;
        constexpr int BONE_RIGHT_FOOT = 40;
        constexpr int BONE_RIGHT_TOE = 41;
        
        // Additional detailed bones
        constexpr int BONE_JAW = 42;
        constexpr int BONE_LEFT_EYE = 43;
        constexpr int BONE_RIGHT_EYE = 44;
    }
    
    // ========================================================================
    // Convenience Macros for Easy Usage
    // ========================================================================
    
    // Get a global address (automatically calls PatternOffsets::GetGlobalOffset)
    #define GET_GLOBAL_OFFSET(name) pattern_offsets::PatternOffsets::GetGlobalOffset(name)
    
    // Quick access to commonly used global offsets
    #define LOCAL_PLAYER_CONTROLLER GET_GLOBAL_OFFSET("dwLocalPlayerController")
    #define LOCAL_PLAYER_PAWN GET_GLOBAL_OFFSET("dwLocalPlayerPawn")
    #define ENTITY_LIST GET_GLOBAL_OFFSET("dwEntityList")
    #define VIEW_MATRIX GET_GLOBAL_OFFSET("dwViewMatrix")
    #define VIEW_ANGLES GET_GLOBAL_OFFSET("dwViewAngles")
    #define CSGO_INPUT GET_GLOBAL_OFFSET("dwCSGOInput")
    #define GAME_RULES GET_GLOBAL_OFFSET("dwGameRules")
    #define GLOBAL_VARS GET_GLOBAL_OFFSET("dwGlobalVars")
    #define GLOW_MANAGER GET_GLOBAL_OFFSET("dwGlowManager")
    #define PLANTED_C4 GET_GLOBAL_OFFSET("dwPlantedC4")
    #define WEAPON_C4 GET_GLOBAL_OFFSET("dwWeaponC4")
    #define SENSITIVITY GET_GLOBAL_OFFSET("dwSensitivity")
    
    // ========================================================================
    // Migration Helper - Compatibility Layer
    // ========================================================================
    
    // Cette classe permet de migrer facilement depuis l'ancien système d'offsets
    class OffsetCompatibility {
    public:
        // Initialize the pattern system (call this once at startup)
        static bool Initialize() {
            return PatternOffsets::Initialize();
        }
        
        // Get global offsets using the old naming convention
        static uintptr_t dwLocalPlayerController() { return GET_GLOBAL_OFFSET("dwLocalPlayerController"); }
        static uintptr_t dwLocalPlayerPawn() { return GET_GLOBAL_OFFSET("dwLocalPlayerPawn"); }
        static uintptr_t dwEntityList() { return GET_GLOBAL_OFFSET("dwEntityList"); }
        static uintptr_t dwGameEntitySystem() { return GET_GLOBAL_OFFSET("dwGameEntitySystem"); }
        static uintptr_t dwViewMatrix() { return GET_GLOBAL_OFFSET("dwViewMatrix"); }
        static uintptr_t dwViewAngles() { return GET_GLOBAL_OFFSET("dwViewAngles"); }
        static uintptr_t dwViewRender() { return GET_GLOBAL_OFFSET("dwViewRender"); }
        static uintptr_t dwCSGOInput() { return GET_GLOBAL_OFFSET("dwCSGOInput"); }
        static uintptr_t dwGameRules() { return GET_GLOBAL_OFFSET("dwGameRules"); }
        static uintptr_t dwGlobalVars() { return GET_GLOBAL_OFFSET("dwGlobalVars"); }
        static uintptr_t dwGlowManager() { return GET_GLOBAL_OFFSET("dwGlowManager"); }
        static uintptr_t dwPrediction() { return GET_GLOBAL_OFFSET("dwPrediction"); }
        static uintptr_t dwSensitivity() { return GET_GLOBAL_OFFSET("dwSensitivity"); }
        static uintptr_t dwPlantedC4() { return GET_GLOBAL_OFFSET("dwPlantedC4"); }
        static uintptr_t dwWeaponC4() { return GET_GLOBAL_OFFSET("dwWeaponC4"); }
        
        // Entity offsets remain the same (forward to entity_offsets namespace)
        static constexpr std::ptrdiff_t m_vecOrigin = entity_offsets::m_vecOrigin;
        static constexpr std::ptrdiff_t m_pGameSceneNode = entity_offsets::m_pGameSceneNode;
        static constexpr std::ptrdiff_t m_iHealth = entity_offsets::m_iHealth;
        static constexpr std::ptrdiff_t m_iTeamNum = entity_offsets::m_iTeamNum;
        static constexpr std::ptrdiff_t m_fFlags = entity_offsets::m_fFlags;
        static constexpr std::ptrdiff_t m_vecViewOffset = entity_offsets::m_vecViewOffset;
        static constexpr std::ptrdiff_t m_entitySpottedState = entity_offsets::m_entitySpottedState;
        static constexpr std::ptrdiff_t m_bSpotted = entity_offsets::m_bSpotted;
        static constexpr std::ptrdiff_t m_bSpottedByMask = entity_offsets::m_bSpottedByMask;
        static constexpr std::ptrdiff_t m_iShotsFired = entity_offsets::m_iShotsFired;
        static constexpr std::ptrdiff_t m_aimPunchAngle = entity_offsets::m_aimPunchAngle;
        static constexpr std::ptrdiff_t m_pClippingWeapon = entity_offsets::m_pClippingWeapon;
        static constexpr std::ptrdiff_t m_vecAbsOrigin = entity_offsets::m_vecAbsOrigin;
        static constexpr std::ptrdiff_t m_bDormant = entity_offsets::m_bDormant;
        static constexpr std::ptrdiff_t m_modelState = entity_offsets::m_modelState;
        static constexpr std::ptrdiff_t dwBoneMatrix = entity_offsets::dwBoneMatrix;
        
        // Bone indices
        static constexpr int BONE_HEAD = bone_indices::BONE_HEAD;
        static constexpr int BONE_NECK = bone_indices::BONE_NECK;
        static constexpr int BONE_CHEST = bone_indices::BONE_CHEST;
        static constexpr int BONE_SPINE_1 = bone_indices::BONE_SPINE_1;
        static constexpr int BONE_SPINE_2 = bone_indices::BONE_SPINE_2;
    };

} // namespace pattern_offsets

// ============================================================================
// Usage Examples and Migration Guide
// ============================================================================

/*
EXAMPLE USAGE:

// 1. Initialize once at startup
pattern_offsets::PatternOffsets::Initialize();

// 2. Use macros for common offsets
uintptr_t localPlayer = *(uintptr_t*)(LOCAL_PLAYER_CONTROLLER);
uintptr_t entityList = *(uintptr_t*)(ENTITY_LIST);

// 3. Use entity offsets (unchanged from before)
int health = *(int*)(entity + pattern_offsets::entity_offsets::m_iHealth);
int team = *(int*)(entity + pattern_offsets::entity_offsets::m_iTeamNum);

// 4. Or use compatibility layer for easy migration
pattern_offsets::OffsetCompatibility::Initialize();
uintptr_t localPlayer = *(uintptr_t*)(pattern_offsets::OffsetCompatibility::dwLocalPlayerController());
*/