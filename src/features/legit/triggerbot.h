#pragma once
#include "legit.h"
#include "../../core/math.h"
#include "../../game/entities.h"

class Triggerbot {
private:
    static DWORD lastFireTime;
    static bool CheckIfEnemyOnCrosshair(C_CSPlayerPawn* localPlayer, const Vector3& localPos);
    static void FireWeapon();
    static int GetTargetBone(int hitbox);
    static Vector3 GetBonePosition(C_CSPlayerPawn* player, int boneIndex);

public:
    static void Initialize();
    static void Update();
};