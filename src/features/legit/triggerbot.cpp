#include "triggerbot.h"
#include "../../game/pattern.h"
#include "../../game/entities.h"
#include <iostream>
#include <Windows.h>
#include <cmath>

DWORD Triggerbot::lastFireTime = 0;

void Triggerbot::Initialize() {
    std::cout << "[Triggerbot] Triggerbot module initialized\n";
}

// Helper function to get target bone based on hitbox setting
int Triggerbot::GetTargetBone(int hitbox) {
    switch (hitbox) {
        case HITBOX_HEAD: return pattern_offsets::bone_indices::BONE_HEAD;
        case HITBOX_NECK: return pattern_offsets::bone_indices::BONE_NECK;
        case HITBOX_CHEST: return pattern_offsets::bone_indices::BONE_CHEST;
        case HITBOX_STOMACH: return pattern_offsets::bone_indices::BONE_SPINE_1;
        case HITBOX_NEAREST: return pattern_offsets::bone_indices::BONE_HEAD; // Default to head for nearest
        default: return pattern_offsets::bone_indices::BONE_HEAD;
    }
}

// Helper function to get bone position using the same reliable method as ESP skeleton
Vector3 Triggerbot::GetBonePosition(C_CSPlayerPawn* player, int boneIndex) {
    if (!player || boneIndex < 0 || boneIndex >= 128) return Vector3(0, 0, 0);
    
    __try {
        uintptr_t playerAddr = reinterpret_cast<uintptr_t>(player);
        
        // Get CGameSceneNode using the correct offset (same as ESP)
        uintptr_t gameScene = Memory::Read<uintptr_t>(playerAddr + pattern_offsets::entity_offsets::m_pGameSceneNode);
        if (!gameScene) {
            return Vector3(0, 0, 0);
        }
        
        // Use the working method: CGameSceneNode + CSkeletonInstance::m_modelState + 0x80
        uintptr_t boneArray = Memory::Read<uintptr_t>(gameScene + pattern_offsets::entity_offsets::m_modelState + 0x80);
        
        if (!boneArray) {
            return Vector3(0, 0, 0);
        }
        
        // Read the specific bone using the working BoneJointData structure
        BoneJointData boneData;
        if (!Memory::ReadRaw(boneArray + (boneIndex * sizeof(BoneJointData)), &boneData, sizeof(BoneJointData))) {
            return Vector3(0, 0, 0);
        }
        
        // Validate bone position using CS2 world coordinate ranges
        if (!std::isnan(boneData.position.x) && !std::isnan(boneData.position.y) && !std::isnan(boneData.position.z) &&
            abs(boneData.position.x) >= 10.0f && abs(boneData.position.y) >= 10.0f && abs(boneData.position.z) >= 10.0f &&
            abs(boneData.position.x) < 100000.0f && abs(boneData.position.y) < 100000.0f && abs(boneData.position.z) < 100000.0f) {
            return boneData.position;
        }
        
        return Vector3(0, 0, 0);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return Vector3(0, 0, 0);
    }
}

void Triggerbot::Update() {
    auto& settings = Legit::GetSettings();
    
    if (!settings.triggerbotEnabled) {
        return;
    }
    
    DWORD currentTime = GetTickCount();
    
    // Check if triggerbot key is pressed (independent from aimbot)
    bool keyPressed = Legit::IsKeyPressed(settings.triggerbotKey);
    
    // Only fire when key is pressed, delay is met, AND crosshair is on target
    if (keyPressed && (currentTime - lastFireTime >= (DWORD)settings.triggerbotDelay)) {
        // Get local player
        C_CSPlayerPawn* localPlayer = Memory::GetLocalPlayerPawn();
        if (!localPlayer) {
            return;
        }
        
        // Get local player eye position
        Vector3 localPos = EntityUtils::GetEyePosition(localPlayer);
        
        // Only fire if enemy is EXACTLY on crosshair
        if (CheckIfEnemyOnCrosshair(localPlayer, localPos)) {
            FireWeapon();
            lastFireTime = currentTime;
        }
    }
}

bool Triggerbot::CheckIfEnemyOnCrosshair(C_CSPlayerPawn* localPlayer, const Vector3& localPos) {
    auto& settings = Legit::GetSettings();
    
    // Get current view angles
    Vector3 currentViewAngles = EntityUtils::GetViewAngles();
    
    // Get entity system
    CGameEntitySystem* entitySystem = Memory::GetEntitySystem();
    if (!entitySystem) {
        return false;
    }
    
    __try {
        // Check all players
        for (int i = 1; i <= 64; i++) {
            auto controller = entitySystem->GetEntityByIndex<CCSPlayerController*>(i);
            if (!controller) continue;
            
            auto playerPawnHandle = controller->GetPlayerPawn();
            if (!playerPawnHandle.IsValid()) continue;
            
            auto player = entitySystem->GetEntityByIndex<C_CSPlayerPawn*>(playerPawnHandle.GetIndex());
            if (!player || player == localPlayer) continue;
            
            // Get player info
            int health = player->GetHealth();
            int team = player->GetTeamNum();
            int localTeam = localPlayer->GetTeamNum();
            
            // Check if alive
            if (health <= 0) continue;
            
            // Check team only if team check is enabled
            if (settings.triggerbotTeamCheck && team == localTeam) continue;
            
            // Get target bone position based on triggerbot hitbox setting
            int targetBone = GetTargetBone(settings.triggerbotHitbox);
            Vector3 targetPos = GetBonePosition(player, targetBone);
            
            // Skip if position is invalid
            if (targetPos.x == 0 && targetPos.y == 0 && targetPos.z == 0) continue;
            
            // Calculate angle to target bone
            Vector3 aimAngles = Math::CalcAngle(localPos, targetPos);
            
            // Calculate FOV using utility function
            float fov = Math::CalculateFOV(currentViewAngles, aimAngles);
            
            // Very precise FOV for exact bone targeting (1.0 degree for precision)
            if (fov <= 0.5f) {
                return true;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        // Silent error handling
    }
    
    return false;
}

void Triggerbot::FireWeapon() {
    // Fire single bullet
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    Sleep(10);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}