#pragma once
#include "visuals.h"
#include "../../core/math.h"
#include "../../../dependencies/imgui/imgui.h"

class ESP {
private:
    // Drawing helper functions
    static void DrawBox(const Vector2& topLeft, const Vector2& bottomRight, ImU32 color, float thickness);
    static void DrawCornerBox(const Vector2& topLeft, const Vector2& bottomRight, ImU32 color, float thickness);
    static void DrawSnapline(const Vector2& screenPos, ImU32 color, float thickness);
    static void DrawText(const Vector2& position, const char* text, ImU32 color, bool centered = false);
    static void DrawHealthBar(const Vector2& position, int health, int maxHealth);
    static void DrawSkeleton(C_CSPlayerPawn* player, ImU32 color, float thickness, const Vector2& topLeft, const Vector2& bottomRight);
    static void DrawBoneLine(int bone1, int bone2, C_CSPlayerPawn* player, ImU32 color, float thickness);
    
    // Bone utilities
    static Vector3 GetBonePosition(C_CSPlayerPawn* player, int boneIndex);
    
    // Processing
    static void ProcessPlayer(C_CSPlayerPawn* player, CCSPlayerController* controller,
                            C_CSPlayerPawn* localPlayer, const Vector3& playerPos);

public:
    static void Initialize();
    static void Render();
};