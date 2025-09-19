#include "aimbot.h"
#include "../../game/pattern.h"  // Use pattern-based offsets
#include "../../game/entities.h"
#include <iostream>
#include <cmath>
#include <Windows.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Aimbot::Initialize() {
    std::cout << "[Aimbot] Aimbot module initialized\n";
}

// Helper function to get target bone based on hitbox setting
int Aimbot::GetTargetBone(int hitbox) {
    switch (hitbox) {
        case HITBOX_HEAD: return pattern_offsets::bone_indices::BONE_HEAD;
        case HITBOX_NECK: return pattern_offsets::bone_indices::BONE_NECK; // Neck bone
        case HITBOX_CHEST: return pattern_offsets::bone_indices::BONE_CHEST; // Chest bone
        case HITBOX_STOMACH: return pattern_offsets::bone_indices::BONE_SPINE_1; // Stomach bone
        case HITBOX_NEAREST: return pattern_offsets::bone_indices::BONE_HEAD; // Default to head for nearest
        default: return pattern_offsets::bone_indices::BONE_HEAD;
    }
}

void Aimbot::Update() {
    auto& settings = Legit::GetSettings();
    
    if (!settings.aimbotEnabled) {
        return;
    }
    
    // Get local player
    C_CSPlayerPawn* localPlayer = Memory::GetLocalPlayerPawn();
    if (!localPlayer) {
        return;
    }
    
    // Get local player eye position
    Vector3 localPos = EntityUtils::GetEyePosition(localPlayer);
    
    // Check if aimbot key is pressed
    bool keyPressed = Legit::IsKeyPressed(settings.aimbotKey);
    if (!keyPressed) {
        return;
    }
    
    // Find best target
    C_CSPlayerPawn* bestTarget = FindBestTarget(localPlayer, localPos);
    
    if (bestTarget) {
        AimAtTarget(bestTarget, localPos);
    }
}

void Aimbot::Render() {
    auto& settings = Legit::GetSettings();
    
    // Only render FOV circle if aimbot is enabled and draw FOV is enabled
    if (!settings.aimbotEnabled || !settings.aimbotDrawFOV) {
        return;
    }
    
    DrawFOVCircle();
}

// Helper function to calculate target priority score (lower = better)
float Aimbot::GetTargetScore(C_CSPlayerPawn* player, const Vector3& localPos, const Vector3& currentViewAngles) {
    auto& settings = Legit::GetSettings();
    
    int targetBone = GetTargetBone(settings.aimbotHitbox);
    Vector3 targetPos = GetBonePosition(player, targetBone);
    if (targetPos.x == 0 && targetPos.y == 0 && targetPos.z == 0) {
        return 9999.0f; // Invalid position, very low priority
    }
    
    switch (settings.aimbotPriority) {
        case PRIORITY_CROSSHAIR: {
            // Priority based on FOV to crosshair (closer to crosshair = better)
            Vector3 aimAngles = Math::CalcAngle(localPos, targetPos);
            return Math::CalculateFOV(currentViewAngles, aimAngles);
        }
        case PRIORITY_HEALTH: {
            // Priority based on health (lower health = better target)
            int health = player->GetHealth();
            return (float)health; // Lower health = lower score = higher priority
        }
        case PRIORITY_DISTANCE: {
            // Priority based on distance (closer = better)
            return Math::Distance3D(localPos, targetPos);
        }
        default:
            Vector3 aimAngles = Math::CalcAngle(localPos, targetPos);
            return Math::CalculateFOV(currentViewAngles, aimAngles);
    }
}

// Helper function to get bone position using the same reliable method as ESP skeleton
Vector3 Aimbot::GetBonePosition(C_CSPlayerPawn* player, int boneIndex) {
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

C_CSPlayerPawn* Aimbot::FindBestTarget(C_CSPlayerPawn* localPlayer, const Vector3& localPos) {
    auto& settings = Legit::GetSettings();
    
    float bestScore = 999999.0f;
    C_CSPlayerPawn* bestTarget = nullptr;
    
    // Get entity system
    CGameEntitySystem* entitySystem = Memory::GetEntitySystem();
    if (!entitySystem) {
        return nullptr;
    }

    __try {
        // Iterate through all entities
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
            
            // Check if enemy
            if (settings.aimbotTeamCheck && team == localPlayer->GetTeamNum()) {
                continue;
            }
            
            // Check if alive
            if (health <= 0) {
                continue;
            }
            
            // Get target bone position based on hitbox setting
            int targetBone = GetTargetBone(settings.aimbotHitbox);
            Vector3 targetPos = GetBonePosition(player, targetBone);
            
            // Skip if position is invalid
            if (targetPos.x == 0 && targetPos.y == 0 && targetPos.z == 0) continue;
            
            // Additional validation: Check distance (avoid targets too far away)
            float distance = Math::Distance3D(localPos, targetPos);
            if (distance > 5000.0f) continue;  // Skip targets beyond reasonable range
            
            // Get current view angles for FOV calculation
            Vector3 currentViewAngles = EntityUtils::GetViewAngles();
            Vector3 aimAngles = Math::CalcAngle(localPos, targetPos);
            float fov = Math::CalculateFOV(currentViewAngles, aimAngles);
            
            // Only consider targets within the FOV setting
            if (fov > (float)settings.aimbotFOV) continue;
            
            // Calculate score based on priority setting
            float score = 0.0f;
            switch (settings.aimbotPriority) {
                case PRIORITY_CROSSHAIR:
                    score = fov; // Lower FOV = better
                    break;
                case PRIORITY_HEALTH:
                    score = (float)health; // Lower health = better
                    break;
                case PRIORITY_DISTANCE:
                    score = distance; // Closer distance = better
                    break;
                default:
                    score = fov;
                    break;
            }
            
            // Select target with best (lowest) score
            if (score < bestScore) {
                bestScore = score;
                bestTarget = player;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        // Silent error handling
    }
    
    return bestTarget;
}

void Aimbot::AimAtTarget(C_CSPlayerPawn* target, const Vector3& localPos) {
    auto& settings = Legit::GetSettings();
    
    // Get target bone position based on hitbox setting
    int targetBone = GetTargetBone(settings.aimbotHitbox);
    Vector3 targetPos = GetBonePosition(target, targetBone);
    
    // If bone position failed, don't aim
    if (targetPos.x == 0 && targetPos.y == 0 && targetPos.z == 0) {
        return;
    }
    
    // Additional validation: ensure target is within reasonable range
    float distance = Math::Distance3D(localPos, targetPos);
    if (distance > 5000.0f || distance < 10.0f) {
        return;  // Skip targets too far or too close
    }
    
    // Get current view angles
    Vector3 currentViewAngles = EntityUtils::GetViewAngles();
    
    // Calculate angle to target (precise head position)
    Vector3 targetAngles = Math::CalcAngle(localPos, targetPos);
    
    // Apply smoothing using utility function
    float smoothFactor = 1.0f / (float)settings.aimbotSmooth;
    Vector3 newViewAngles = Math::LerpAngles(currentViewAngles, targetAngles, smoothFactor);
    
    // Clamp and normalize angles to prevent invalid values
    newViewAngles = Math::ClampAngles(newViewAngles);
    
    // Write new view angles to memory
    EntityUtils::SetViewAngles(newViewAngles);
}

void Aimbot::DrawFOVCircle() {
    auto& settings = Legit::GetSettings();
    
    // Get ImGui draw list for overlay rendering
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    if (!drawList) {
        return;
    }
    
    // Get screen dimensions from ImGui
    ImGuiIO& io = ImGui::GetIO();
    float screenWidth = io.DisplaySize.x;
    float screenHeight = io.DisplaySize.y;
    
    // Calculate center of screen (crosshair position)
    float centerX = screenWidth / 2.0f;
    float centerY = screenHeight / 2.0f;
    
    // Calculate FOV circle radius based on screen resolution and FOV setting
    float fovRadius = (float)settings.aimbotFOV * (screenHeight / 90.0f);
    
    // Use fixed white color for FOV circle
    ImU32 color = IM_COL32(255, 255, 255, 255);
    
    // Draw the FOV circle
    drawList->AddCircle(
        ImVec2(centerX, centerY),
        fovRadius,
        color,
        64,  // Number of segments
        2.0f // Thickness
    );
    
    // Optionally draw crosshair dot in the center
    drawList->AddCircleFilled(
        ImVec2(centerX, centerY),
        2.0f,
        IM_COL32(255, 255, 255, 100), 
        12 // Segments for small circle
    );
}