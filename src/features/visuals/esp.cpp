#include "esp.h"
#include "../../game/pattern.h"  // Use pattern-based offsets
#include <cmath>
#include <vector>
#include <iostream>

void ESP::Initialize() {
    // ESP module initialization complete
}

void ESP::Render() {
    auto& settings = Visuals::GetSettings();
    
    auto entitySystem = Memory::GetEntitySystem();
    auto localPlayerPawn = Memory::GetLocalPlayerPawn();
    
    if (!entitySystem || !localPlayerPawn) {
        return;
    }
    
    // Process all players for ESP rendering
    __try {
        int processedPlayers = 0;
        for (int i = 1; i <= 64; ++i) {
            auto controller = entitySystem->GetEntityByIndex<CCSPlayerController*>(i);
            if (!controller || !Visuals::IsValidEntity(controller)) continue;
            
            auto playerPawnHandle = controller->GetPlayerPawn();
            if (!playerPawnHandle.IsValid()) continue;
            
            auto playerPawn = entitySystem->GetEntityByIndex<C_CSPlayerPawn*>(playerPawnHandle.GetIndex());
            if (!Visuals::IsValidEntity(playerPawn) || playerPawn == localPlayerPawn) continue;
            
            // Get position using utility function
            Vector3 playerPos = EntityUtils::GetWorldPosition(playerPawn);
            
            // Skip if position is invalid
            if (playerPos.x == 0 && playerPos.y == 0 && playerPos.z == 0) continue;
            
            processedPlayers++;
            
            ProcessPlayer(playerPawn, controller, localPlayerPawn, playerPos);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        // Silent error handling
    }
}

void ESP::ProcessPlayer(C_CSPlayerPawn* player, CCSPlayerController* controller, 
                       C_CSPlayerPawn* localPlayer, const Vector3& playerPos) {
    auto& settings = Visuals::GetSettings();
    auto& viewMatrix = Visuals::GetViewMatrix();
    int screenWidth = Visuals::GetScreenWidth();
    int screenHeight = Visuals::GetScreenHeight();
    
    if (player->IsDormant()) return;
    
    // Check if player is alive
    int health = player->GetHealth();
    if (health <= 0) return;
    
    // Get local player position
    // Get local player position using utility
    Vector3 localPos = EntityUtils::GetWorldPosition(localPlayer);
    
    // Distance check
    float distance = Math::Distance3D(playerPos, localPos);
    if (distance > settings.maxDistance) return;
    
    // Simple WorldToScreen test
    Vector2 screenPos;
    if (!Math::WorldToScreen(playerPos, screenPos, viewMatrix, screenWidth, screenHeight)) {
        return;
    }
    
    // Get proper bounding box
    Vector2 topLeft, bottomRight;
    if (!Math::GetBoundingBox(playerPos, viewMatrix, screenWidth, screenHeight, topLeft, bottomRight)) {
        return;
    }
    
    // Team ESP check - Skip teammates unless teamESP is enabled
    bool isEnemy = Visuals::IsEnemy(localPlayer, player);
    if (!isEnemy && !settings.teamESP) {
        return; // Skip teammates if teamESP is disabled
    }
    
    // Choose appropriate colors based on team
    float* currentBoxColor = isEnemy ? settings.enemyBoxColor : settings.teamBoxColor;
    float* currentNameColor = isEnemy ? settings.enemyNameColor : settings.teamNameColor; 
    float* currentDistanceColor = isEnemy ? settings.enemyDistanceColor : settings.teamDistanceColor;
    float* currentSnaplineColor = isEnemy ? settings.enemySnaplineColor : settings.teamSnaplineColor;
    float* currentSkeletonColor = isEnemy ? settings.enemySkeletonColor : settings.teamSkeletonColor;
    
    // Convert individual colors to ImU32
    ImU32 boxColor = IM_COL32(
        (int)(currentBoxColor[0] * 255),
        (int)(currentBoxColor[1] * 255),
        (int)(currentBoxColor[2] * 255),
        (int)(currentBoxColor[3] * 255)
    );
    
    ImU32 nameColor = IM_COL32(
        (int)(currentNameColor[0] * 255),
        (int)(currentNameColor[1] * 255),
        (int)(currentNameColor[2] * 255),
        (int)(currentNameColor[3] * 255)
    );
    
    ImU32 distanceColor = IM_COL32(
        (int)(currentDistanceColor[0] * 255),
        (int)(currentDistanceColor[1] * 255),
        (int)(currentDistanceColor[2] * 255),
        (int)(currentDistanceColor[3] * 255)
    );
    
    ImU32 snaplineColor = IM_COL32(
        (int)(currentSnaplineColor[0] * 255),
        (int)(currentSnaplineColor[1] * 255),
        (int)(currentSnaplineColor[2] * 255),
        (int)(currentSnaplineColor[3] * 255)
    );
    
    ImU32 skeletonColor = IM_COL32(
        (int)(currentSkeletonColor[0] * 255),
        (int)(currentSkeletonColor[1] * 255),
        (int)(currentSkeletonColor[2] * 255),
        (int)(currentSkeletonColor[3] * 255)
    );
    
    // Draw box ESP
    if (settings.boxESP) {
        DrawBox(topLeft, bottomRight, boxColor, (float)settings.lineThickness);
    }
    
    // Draw corner box ESP
    if (settings.cornerBoxESP) {
        DrawCornerBox(topLeft, bottomRight, boxColor, (float)settings.lineThickness);
    }
    
    // Draw health ESP
    if (settings.healthESP) {
        int health = player->GetHealth();
        if (health > 0 && health <= 100) {
            Vector2 healthBarPos = { topLeft.x - 8, topLeft.y };
            DrawHealthBar(healthBarPos, health, 100);
        }
    }
    
    // Draw name ESP
    if (settings.nameESP) {
        const char* playerName = "Unknown";
        __try {
            if (controller) {
                const char* name = controller->GetPlayerName();
                if (name && strlen(name) > 0 && strlen(name) < 64) {
                    playerName = name;
                }
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            playerName = "Unknown";
        }
        
        Vector2 namePos = { screenPos.x, topLeft.y - 15 };
        DrawText(namePos, playerName, nameColor, true);
    }
    
    // Draw distance ESP
    if (settings.distanceESP) {
        char distText[32];
        snprintf(distText, sizeof(distText), "%.0fm", EntityUtils::GameUnitsToMeters(distance));
        Vector2 distPos = { screenPos.x, bottomRight.y + 5 };
        DrawText(distPos, distText, distanceColor, true);
    }
    
    // Draw snapline ESP
    if (settings.snaplineESP) {
        DrawSnapline(screenPos, snaplineColor, (float)settings.lineThickness);
    }
    
    // Draw skeleton ESP
    if (settings.skeletonESP) {
        DrawSkeleton(player, skeletonColor, (float)settings.lineThickness, topLeft, bottomRight);
    }
}

Vector3 ESP::GetBonePosition(C_CSPlayerPawn* player, int boneIndex) {
    if (!player || boneIndex < 0 || boneIndex >= 128) return Vector3(0, 0, 0);
    
    __try {
        uintptr_t playerAddr = reinterpret_cast<uintptr_t>(player);
        
        // Get CGameSceneNode using the correct offset
        uintptr_t gameScene = Memory::Read<uintptr_t>(playerAddr + pattern_offsets::entity_offsets::m_pGameSceneNode);
        if (!gameScene) {
            return Vector3(0, 0, 0);
        }
        
        // Use the working method: CGameSceneNode + CSkeletonInstance::m_modelState + 0x80
        uintptr_t boneArray = Memory::Read<uintptr_t>(gameScene + pattern_offsets::entity_offsets::m_modelState + 0x80);
        
        if (!boneArray) {
            return Vector3(0, 0, 0);
        }
        
        // Read the entire bone array using the working structure
        const int maxBones = 128;
        BoneJointData boneData[maxBones];
        
        if (!Memory::ReadRaw(boneArray, boneData, sizeof(BoneJointData) * maxBones)) {
            // Fallback: Try reading just the specific bone
            BoneJointData singleBone;
            if (!Memory::ReadRaw(boneArray + (boneIndex * sizeof(BoneJointData)), &singleBone, sizeof(BoneJointData))) {
                return Vector3(0, 0, 0);
            }
            
            // Validate single bone (CS2 world coordinates are typically in this range)
            if (!std::isnan(singleBone.position.x) && !std::isnan(singleBone.position.y) && !std::isnan(singleBone.position.z) &&
                abs(singleBone.position.x) >= 10.0f && abs(singleBone.position.y) >= 10.0f && abs(singleBone.position.z) >= 10.0f &&
                abs(singleBone.position.x) < 100000.0f && abs(singleBone.position.y) < 100000.0f && abs(singleBone.position.z) < 100000.0f) {
                return singleBone.position;
            }
            
            return Vector3(0, 0, 0);
        }
        
        // Successfully read the array, get the specific bone
        if (boneIndex >= 0 && boneIndex < maxBones) {
            const BoneJointData& bone = boneData[boneIndex];
            
            // Validate bone position using CS2 world coordinate ranges
            if (!std::isnan(bone.position.x) && !std::isnan(bone.position.y) && !std::isnan(bone.position.z) &&
                abs(bone.position.x) >= 10.0f && abs(bone.position.y) >= 10.0f && abs(bone.position.z) >= 10.0f &&
                abs(bone.position.x) < 100000.0f && abs(bone.position.y) < 100000.0f && abs(bone.position.z) < 100000.0f) {
                return bone.position;
            }
        }
        
        return Vector3(0, 0, 0);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return Vector3(0, 0, 0);
    }
}

void ESP::DrawBoneLine(int bone1, int bone2, C_CSPlayerPawn* player, ImU32 color, float thickness) {
    if (!player) return;
    
    auto& viewMatrix = Visuals::GetViewMatrix();
    int screenWidth = Visuals::GetScreenWidth();
    int screenHeight = Visuals::GetScreenHeight();
    
    __try {
        // Get bone positions using our simplified GetBonePosition function
        Vector3 bone1Pos = GetBonePosition(player, bone1);
        Vector3 bone2Pos = GetBonePosition(player, bone2);
        
        // Check if positions are valid
        if ((bone1Pos.x == 0 && bone1Pos.y == 0 && bone1Pos.z == 0) ||
            (bone2Pos.x == 0 && bone2Pos.y == 0 && bone2Pos.z == 0)) {
            return;
        }
        
        // Convert to screen coordinates
        Vector2 screen1, screen2;
        if (!Math::WorldToScreen(bone1Pos, screen1, viewMatrix, screenWidth, screenHeight) ||
            !Math::WorldToScreen(bone2Pos, screen2, viewMatrix, screenWidth, screenHeight)) {
            return;
        }
        
        // Draw line
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        drawList->AddLine(ImVec2(screen1.x, screen1.y), ImVec2(screen2.x, screen2.y), color, thickness);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        // Silently handle memory access errors
        return;
    }
}

void ESP::DrawSkeleton(C_CSPlayerPawn* player, ImU32 color, float thickness, const Vector2& topLeft, const Vector2& bottomRight) {
    if (!player) return;
    
    auto& viewMatrix = Visuals::GetViewMatrix();
    int screenWidth = Visuals::GetScreenWidth();
    int screenHeight = Visuals::GetScreenHeight();
    
    __try {
        // Get bone positions for basic skeleton
        Vector3 bonePositions[30]; // Basic skeleton bones
        bool validBones[30] = {false};
        int validBoneCount = 0;
        
        // Read bone positions
        for (int i = 0; i < 30; i++) {
            bonePositions[i] = GetBonePosition(player, i);
            if (!(bonePositions[i].x == 0 && bonePositions[i].y == 0 && bonePositions[i].z == 0)) {
                validBones[i] = true;
                validBoneCount++;
            }
        }
        
        
        // Only draw if we have enough valid bones
        if (validBoneCount > 8) {
            ImDrawList* drawList = ImGui::GetForegroundDrawList();
            
            // Define basic bone connections only (NO HEAD)
            struct BoneConnection {
                int bone1;
                int bone2;
            };
            
            BoneConnection connections[] = {
                // Main spine (start from neck instead of head)
                {5, 4},   // neck to chest 
                {4, 2},   // chest to mid spine
                {2, 0},   // mid spine to pelvis
                
                // Left arm
                {4, 8},   // chest to left shoulder
                {8, 9},   // left shoulder to left upper arm
                {9, 10},  // left upper arm to left elbow
                
                // Right arm
                {4, 13},  // chest to right shoulder
                {13, 14}, // right shoulder to right upper arm
                {14, 15}, // right upper arm to right elbow
          
                // Left leg (corrected bone IDs for CS2)
                {0, 22},  // pelvis to left hip
                {22, 23}, // left hip to left knee
                {23, 24}, // left knee to left foot
                
                // Right leg
                {0, 25},  // pelvis to right hip
                {25, 26}, // right hip to right knee
                {26, 27}  // right knee to right foot
            };
            
            // Draw bone connections with validation
            for (const auto& conn : connections) {
                if (validBones[conn.bone1] && validBones[conn.bone2]) {
                    Vector2 screen1, screen2;
                    if (Math::WorldToScreen(bonePositions[conn.bone1], screen1, viewMatrix, screenWidth, screenHeight) &&
                        Math::WorldToScreen(bonePositions[conn.bone2], screen2, viewMatrix, screenWidth, screenHeight)) {
                        
                        // Additional validation: check if line is reasonable (not too long)
                        float distance = sqrt((screen2.x - screen1.x) * (screen2.x - screen1.x) + 
                                            (screen2.y - screen1.y) * (screen2.y - screen1.y));
                        
                        // Only draw if distance is reasonable (prevent glitchy long lines)
                        if (distance > 1.0f && distance < 200.0f) {
                            drawList->AddLine(ImVec2(screen1.x, screen1.y), ImVec2(screen2.x, screen2.y), color, thickness);
                        }
                    }
                }
            }
            
            // NO HEAD CIRCLE - removed completely
        }
        
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return;
    }
}

void ESP::DrawBox(const Vector2& topLeft, const Vector2& bottomRight, ImU32 color, float thickness) {
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    
    drawList->AddRect(
        ImVec2(topLeft.x, topLeft.y),
        ImVec2(bottomRight.x, bottomRight.y),
        color,
        0.0f,
        0,
        thickness
    );
}

void ESP::DrawCornerBox(const Vector2& topLeft, const Vector2& bottomRight, ImU32 color, float thickness) {
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    
    float width = bottomRight.x - topLeft.x;
    float height = bottomRight.y - topLeft.y;
    float cornerLength = (width < height ? width : height) * 0.2f; // 20% of the smaller dimension
    
    // Top-left corner
    drawList->AddLine(ImVec2(topLeft.x, topLeft.y), ImVec2(topLeft.x + cornerLength, topLeft.y), color, thickness);
    drawList->AddLine(ImVec2(topLeft.x, topLeft.y), ImVec2(topLeft.x, topLeft.y + cornerLength), color, thickness);
    
    // Top-right corner
    drawList->AddLine(ImVec2(bottomRight.x, topLeft.y), ImVec2(bottomRight.x - cornerLength, topLeft.y), color, thickness);
    drawList->AddLine(ImVec2(bottomRight.x, topLeft.y), ImVec2(bottomRight.x, topLeft.y + cornerLength), color, thickness);
    
    // Bottom-left corner
    drawList->AddLine(ImVec2(topLeft.x, bottomRight.y), ImVec2(topLeft.x + cornerLength, bottomRight.y), color, thickness);
    drawList->AddLine(ImVec2(topLeft.x, bottomRight.y), ImVec2(topLeft.x, bottomRight.y - cornerLength), color, thickness);
    
    // Bottom-right corner
    drawList->AddLine(ImVec2(bottomRight.x, bottomRight.y), ImVec2(bottomRight.x - cornerLength, bottomRight.y), color, thickness);
    drawList->AddLine(ImVec2(bottomRight.x, bottomRight.y), ImVec2(bottomRight.x, bottomRight.y - cornerLength), color, thickness);
}

void ESP::DrawSnapline(const Vector2& screenPos, ImU32 color, float thickness) {
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    int screenWidth = Visuals::GetScreenWidth();
    int screenHeight = Visuals::GetScreenHeight();
    
    // Snapline from bottom of screen to player position
    Vector2 bottomCenter = { (float)screenWidth / 2.0f, (float)screenHeight };
    
    drawList->AddLine(
        ImVec2(bottomCenter.x, bottomCenter.y),
        ImVec2(screenPos.x, screenPos.y),
        color,
        thickness
    );
}

void ESP::DrawText(const Vector2& position, const char* text, ImU32 color, bool centered) {
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    ImVec2 textSize = ImGui::CalcTextSize(text);
    
    ImVec2 textPos;
    if (centered) {
        textPos = ImVec2(position.x - textSize.x / 2, position.y);
    } else {
        textPos = ImVec2(position.x, position.y);
    }
    
    // Direct text without background for cleaner look
    drawList->AddText(textPos, color, text);
}

void ESP::DrawHealthBar(const Vector2& position, int health, int maxHealth) {
    float barHeight = 50.0f;
    float barWidth = 4.0f;
    
    float healthPercent = (float)health / (float)maxHealth;
    float currentBarHeight = barHeight * healthPercent;
    
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    
    // Background
    drawList->AddRectFilled(
        ImVec2(position.x, position.y),
        ImVec2(position.x + barWidth, position.y + barHeight),
        IM_COL32(0, 0, 0, 200)
    );
    
    // Health bar color (red to green based on health)
    ImU32 healthColor;
    if (healthPercent > 0.6f) {
        healthColor = IM_COL32(0, 255, 0, 255); // Green
    } else if (healthPercent > 0.3f) {
        healthColor = IM_COL32(255, 255, 0, 255); // Yellow
    } else {
        healthColor = IM_COL32(255, 0, 0, 255); // Red
    }
    
    // Health fill
    drawList->AddRectFilled(
        ImVec2(position.x, position.y + barHeight - currentBarHeight),
        ImVec2(position.x + barWidth, position.y + barHeight),
        healthColor
    );
    
    // Health text
    if (health < maxHealth) {
        char healthText[8];
        snprintf(healthText, sizeof(healthText), "%d", health);
        Vector2 textPos = { position.x - 15, position.y + barHeight - currentBarHeight - 5 };
        DrawText(textPos, healthText, IM_COL32(255, 255, 255, 255), false);
    }
}