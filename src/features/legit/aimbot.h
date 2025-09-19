#pragma once
#include "legit.h"
#include "../../core/math.h"
#include "../../game/entities.h"
#include "../../../dependencies/imgui/imgui.h"

class Aimbot {
private:
    static C_CSPlayerPawn* FindBestTarget(C_CSPlayerPawn* localPlayer, const Vector3& localPos);
    static void AimAtTarget(C_CSPlayerPawn* target, const Vector3& localPos);
    static void DrawFOVCircle();
    static Vector3 GetBonePosition(C_CSPlayerPawn* player, int boneIndex);
    static int GetTargetBone(int hitbox);
    static Vector3 GetTargetPosition(C_CSPlayerPawn* player);
    static float GetTargetScore(C_CSPlayerPawn* player, const Vector3& localPos, const Vector3& currentViewAngles);

public:
    static void Initialize();
    static void Update();
    static void Render();
};